#ifndef KWAYMERGE_ITERATIVE_MERGE_H
#define KWAYMERGE_ITERATIVE_MERGE_H

#include <cmath>
#include <iterator>
#include <algorithm>
#include <forward_list>
#include <functional>

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

template<
		typename OutputContainer,
		typename Comp = std::less<typename OutputContainer::value_type>
>
void iterative_merge(OutputContainer &output,
                     std::forward_list<typename OutputContainer::iterator> &separators,
                     std::size_t separators_size,
                     Comp comp = Comp()) {
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
			std::size_t const merging_pass_number{static_cast<std::size_t>(std::ceil(std::log2(separators_size)))};
			// O(log2(k)) iterations
			for (std::size_t i{0}; i < merging_pass_number; ++i) {
				auto left_iterator = separators.begin();
				auto middle_iterator = std::next(left_iterator);
				decltype(middle_iterator) right_iterator;
				// We will iterate over each separators, and each time merge two blocks.
				// Block size upper-bound: 2^(1+i)*M
				// Upper-bound of the number of elements given to inplace_merge:
				//   n = 2^(2+i)*M
				// Number of merge: k/(2^(2+i))
				while (middle_iterator != separators.end() &&
				       (right_iterator = std::next(middle_iterator)) != separators.end()) {
					auto left = *left_iterator;
					auto middle = *middle_iterator;
					auto right = *right_iterator;
#pragma omp task firstprivate(left, middle, right) shared(output)
					{
						// Do the merge in-place!
						// O(n) if there is enough memory available, else O(n*log(n))
						std::inplace_merge(left, middle, right, comp);
					}
					// Remove the separator in the middle that now points at the middle of a sorted part.
					separators.erase_after(left_iterator);
					// And we update the iterator to merge the 2 next non-sorted blocks.
					left_iterator = std::next(left_iterator);
					middle_iterator = std::next(left_iterator);
				}
#pragma omp taskwait
			}
		}
	}
}


} //detail
#endif //KWAYMERGE_ITERATIVE_MERGE_H
