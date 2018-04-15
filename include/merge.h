#ifndef MULTI_MERGE_MERGE_H
#define MULTI_MERGE_MERGE_H

#include <memory>
#include <vector>
#include <numeric>
#include <algorithm>
#include <iterator>
#include <forward_list>
#include <list>
#include <cmath>

#ifdef _OPENMP
#include "omp.h"
#endif

template <template<class, class> typename Container, typename T>
using Container_t = Container<T, std::allocator<T>>;


/**
 * @brief Efficient stable k-way merge.
 *
 *
 * This function merges k sorted STL-like containers (InternalContainer) into
 * one sorted InternalContainer.
 *
 * The merge is stable:
 * -# Ordering of equivalent elements within the same InternalContainer is
 * preserved.
 * -# Ordering of equivalent elements from different InternalContainers follows
 * the InternalContainers ordering.
 *
 * The function is parallelised using OpenMP when the library is available.
 *
 * Overall complexity is:
 *
 *
 * @tparam T type of the objects to sort.
 * @tparam InternalContainer STL-like container used to store the objects of
 * type T. This is the type of the k lists to merge as well as the type of the
 * returned sorted list.
 * @tparam ExternalContainer STL-like container that can be iterated on with
 * iterators verifying at least the ForwardIterator requirements.
 * @tparam Comp Comparator used to merge the lists. Each list in @p arrays
 * should be sorted according to this comparator.
 * @param arrays k sorted lists to merge.
 * @param comp See template parameter @p Comp.
 * @return a sorted list.
 * @warning For the moment, InternalContainer should be std::vector.
 */
template <
		typename T,
		template<class, class> typename InternalContainer,
		template<class, class> typename ExternalContainer,
		typename Comp = std::less<T>
>
Container_t<InternalContainer, T> merge_arrays(Container_t<ExternalContainer, Container_t<InternalContainer, T>> const & arrays,
                                               Comp comp = Comp()) {

	/* Note about complexity:
	 * Complexity of each major point ("0.", "1.", ...) is analysed in details.
	 * The names used are:
	 *   1. "k": the number of lists to merge. `k == arrays.size()`.
	 *   2. "N": the total number of elements. N is the sum of the size of the
	 *           "k" lists in arrays.
	 *   3. "M": the size of the longest list in arrays.
	 *   4. "Comp": the number of operations needed to compare to objects of
	 *              type T with the provided comparator.
	 *   4. "TDefInit": the number of operations needed to default-initialise
	 *                  an object of type T.
	 *   5. "p": the number of processor(s) used by OpenMP.
	 * For parallelised parts "complexity" means the "sequential complexity",
	 * also called "work". The "depth" is also given.
	 */

	// 1. Useful types definition
	// Complexity: O(0) (compile-time).
	using ExternalContainer_t = Container_t<ExternalContainer, Container_t<InternalContainer, T>>;
	using InternalContainer_t = Container_t<InternalContainer, T>;

	// 1.1. Check that the ExternalContainer can be iterated on with at least a ForwardIterator.
	using ExternalContainerIterator = typename ExternalContainer<T, std::allocator<T>>::iterator;
	using ExternalContainerIteratorCategory = typename std::iterator_traits<ExternalContainerIterator>::iterator_category;
	static_assert(std::is_same<ExternalContainerIteratorCategory, std::forward_iterator_tag>::value ||
	              std::is_same<ExternalContainerIteratorCategory, std::bidirectional_iterator_tag>::value ||
	              std::is_same<ExternalContainerIteratorCategory, std::random_access_iterator_tag>::value,
	              "The ExternalContainer type should be ForwardIterable");
	// 1.2. For the moment, the InternalContainer can only be a std::vector.
	// The reason is the use of std::vector::resize 12 lines below.
	static_assert(std::is_same< Container_t<InternalContainer, T>, std::vector<T> >::value,
	              "InternalContainer should be std::vector.");

	// 2. Create the resulting structure and reserve enough space for it.
	// Complexity: O(k + N*TDefInit)  ==  O(N*TDefInit)

	// 2.1. Computing the total number of elements in order to save some re-allocations.
	// Complexity: O(k)
	auto add_size = [] (std::size_t const & previous_size, std::vector<T> const & elem) { return previous_size + elem.size(); };
	std::size_t const number_of_elements{ std::accumulate(arrays.begin(), arrays.end(), static_cast<std::size_t>(0), add_size) };
	// 2.2. Resize the result to be sure that all the values can fit in it.
	// Resize is needed because std::merge will need the memory block afterwards (and reserve do not
	// initialise the memory block).
	// Complexity: O(N*TDefInit)
	InternalContainer_t result(number_of_elements);

	// 3. Declare some useful constants for the following steps.
	// Complexity: O(1) if ExternalContainer supports an O(1) size method.
	//             O(k) if ExternalContainer is std::forward_list.
	std::size_t const number_of_lists_to_merge{ [&arrays]() {
		if constexpr(std::is_same<ExternalContainer_t, std::forward_list<InternalContainer_t>>::value) {
			// The external container is std::forward_list. This type does not have a size() method,
			// so we need to call std::distance.
			return static_cast<std::size_t>(std::distance(arrays.begin(), arrays.end()));
		} else {
			return arrays.size();
		}
	}() }; // Initialisation with a directly-called lambda function.
	std::size_t const block_number_after_first_round{number_of_lists_to_merge/2 + number_of_lists_to_merge%2};
	std::size_t const block_separator_number_after_first_round{block_number_after_first_round - 1};


	// 4. Handle simple cases:
	if(number_of_lists_to_merge == 0) {
		return result;
	} else if(number_of_lists_to_merge == 1) {
		std::copy(arrays.front().begin(), arrays.front().end(), result.begin());
		return result;
	} else if(number_of_lists_to_merge == 2) {
		auto const first = arrays.begin();
		auto const second = std::next(first);
		std::merge(first->begin(), first->end(), second->begin(), second->end(), result.begin());
		return result;
	}

	// 5. First step of the merge: concurrently merge adjacent containers into the memory space
	// allocated for the result.
	// We also keep track of the iterators that separate 2 sorted lists in separators.
	// separators stores iterators pointing to the beginning of a sorted sequence.
	// Complexity:
	//   - Work: O(M*Comp*k + k + M)
	//   - Depth: O(M*Comp*k/p + k + M)

	// 5.1. Create a vector of pointers on the beginning of sorted-blocks.
	// Complexity: O(k)
	// The last pointer is the end of the vector.
	std::forward_list<typename decltype(result)::iterator> separators(2 + block_separator_number_after_first_round, result.begin());
	// Iterator on the last valid element of separators.
	auto last_inserted = separators.begin();

	// 5.2. Merge concurrently adjacent containers
	// Complexity:
	//   - Work: O(M*Comp*k) (at most (2*M-1) comparisons per merge, k/2 merges)
	//   - Depth: O(M*Comp*k/p)
	{   // Open a block here because we have other variables called left/right
		// after this block and we don't want to mix them.
		auto left = arrays.begin();
		auto right = std::next(left);
#pragma omp parallel
		{
#pragma omp single nowait
			{
				std::size_t next_free_position{0};
				while(left != arrays.end() && right != arrays.end()) {
					right = std::next(left);
					++last_inserted;
#pragma omp task firstprivate(last_inserted, left, right, next_free_position)
					{
						*last_inserted = std::merge(left->begin() , left->end(),         /*input  1*/
						                            right->begin(), right->end(),        /*input  2*/
						                            result.begin() + next_free_position, /*output 1*/
						                            comp);                               /*comparator*/
					}
					next_free_position += left->size() + right->size();
					left = std::next(right);
				};
			}
		}
		// 5.3. Take care of the last array if there is a odd number of arrays.
		// Complexity: O(M)
		if(number_of_lists_to_merge%2) {
			auto const previously_inserted = last_inserted;
			++last_inserted;
			*last_inserted = std::copy(left->begin(),
			                           left->end(),
			                           *previously_inserted);
		}
	}
	// 6. Now result contains all the values, we just need to merge all the lists not merged in the previous step.
	// In the separators list, we also have the begin() and end() iterators, that is why our stop condition is
	// that the separators list should contain only 2 elements: the begin() and the end()
	// Complexity:
	//   - Work: O(k*M*log2(k))
	//   - Depth: O(k*M*log2(k)/p)
#pragma omp parallel
	{
#pragma omp single nowait
		{
			std::size_t const merging_pass_number{static_cast<std::size_t>(std::ceil(std::log2(block_separator_number_after_first_round)))};
			// O(log2(k)) iterations
			for(std::size_t i{0}; i < merging_pass_number; ++i) {
				auto left_iterator   = separators.begin();
				auto middle_iterator = std::next(left_iterator);
				decltype(middle_iterator) right_iterator;
				// We will iterate over each separators, and each time merge two blocks.
				// Block size upper-bound: 2^(1+i)*M
				// Upper-bound of the number of elements given to inplace_merge:
				//   n = 2^(2+i)*M
				// Number of merge: k/(2^(2+i))
				while(middle_iterator != separators.end() &&
				      (right_iterator = std::next(middle_iterator)) != separators.end()) {
					auto left = *left_iterator;
					auto middle = *middle_iterator;
					auto right = *right_iterator;
#pragma omp task firstprivate(left, middle, right) shared(result)
					{
						// Do the merge in-place!
						// O(n) if there is enough memory available, else O(n*log(n))
						std::inplace_merge(left, middle, right, comp);
					}
					// Remove the separator in the middle that now points at the middle of a sorted part.
					separators.erase_after(left_iterator);
					// And we update the iterator to merge the 2 next non-sorted blocks.
					left_iterator   = std::next(left_iterator);
					middle_iterator = std::next(left_iterator);
				}
#pragma omp taskwait
			}
		}
	}

	// Return the result
	return result;
}

#endif //MULTI_MERGE_MERGE_H
