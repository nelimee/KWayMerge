#ifndef KWAYMERGE_FIRST_ROUND_MERGE_H
#define KWAYMERGE_FIRST_ROUND_MERGE_H

#include <forward_list>
#include <algorithm>
#include <functional>

#include "get_container_size.h"

#ifdef _OPENMP

#include "omp.h"

#endif

namespace detail {

/* Note about complexity:
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

template<typename Container>
using Iterator_t = typename Container::iterator;

/**
 *
 * @tparam ContainerOfContainers
 * @tparam OutputContainer
 * @tparam Comp
 * @param container
 * @param output
 * @param comp
 * @return
 */
template<
		typename ContainerOfContainers,
		typename OutputContainer,
		typename Comp = std::less<typename ContainerOfContainers::value_type::value_type>
>
std::forward_list<Iterator_t<OutputContainer>> first_round_merge(ContainerOfContainers const &container,
                                                                 OutputContainer &output,
                                                                 Comp comp = Comp()) {
	// Concurrently merge adjacent containers into the memory space allocated for the result.
	// We also keep track of the iterators that separate 2 sorted lists in separators.
	// separators stores iterators pointing to the beginning of a sorted sequence in output.
	// Complexity:
	//   - Work: O(M*Comp*k + k + M)
	//   - Depth: O(M*Comp*k/p + k + M)

	std::size_t const number_of_lists_to_merge{get_container_size(container)};
	std::size_t const block_number_after_first_round{number_of_lists_to_merge / 2 + number_of_lists_to_merge % 2};
	std::size_t const block_separator_number_after_first_round{block_number_after_first_round - 1};

	// 1. Create a vector of pointers on the beginning of sorted-blocks.
	// Complexity: O(k)
	// The last pointer is the end of the vector.
	std::forward_list<Iterator_t<OutputContainer>> separators(2 + block_separator_number_after_first_round,
	                                                          output.begin());
	// Iterator on the last valid element of separators.
	auto last_inserted = separators.begin();

	// 2. Merge concurrently adjacent containers
	// Complexity:
	//   - Work: O(M*Comp*k) (at most (2*M-1) comparisons per merge, k/2 merges)
	//   - Depth: O(M*Comp*k/p)
	{   // Open a block here because we have other variables called left/right
		// after this block and we don't want to mix them.
		auto left = container.begin();
		auto right = std::next(left);
#pragma omp parallel
		{
#pragma omp single nowait
			{
				std::size_t next_free_position{0};
				// TODO: Change this to a for-loop?
				while (left != container.end() && right != container.end()) {
					right = std::next(left);
					++last_inserted;
#pragma omp task firstprivate(last_inserted, left, right, next_free_position)
					{
						*last_inserted = std::merge(left->begin(), left->end(),         /*input  1*/
						                            right->begin(), right->end(),        /*input  2*/
						                            output.begin() + next_free_position, /*output 1*/
						                            comp);                               /*comparator*/
					}
					next_free_position += left->size() + right->size();
					left = std::next(right);
				};
			}
		}
		// 5.3. Take care of the last array if there is a odd number of arrays.
		// Complexity: O(M)
		if (number_of_lists_to_merge % 2) {
			auto const previously_inserted = last_inserted;
			++last_inserted;
			*last_inserted = std::copy(left->begin(),
			                           left->end(),
			                           *previously_inserted);
		}
	}

	return separators;
}

}
#endif //KWAYMERGE_FIRST_ROUND_MERGE_H
