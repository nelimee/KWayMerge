#ifndef INCLUDE_DETAIL_ITERATIVE_MERGE_H_
#define INCLUDE_DETAIL_ITERATIVE_MERGE_H_

#include <cmath>
#include <iterator>
#include <algorithm>
#include <forward_list>
#include <functional>

#ifdef _OPENMP
#include "omp.h"
#endif

namespace detail {

template <typename Container> using Iterator_t = typename Container::iterator;

/**
 * @brief Iteratively merge contiguous sorted sequences represented by @a separators.
 *
 * The method uses std::merge_inplace to merge the separators "in place".
 * The merging is done by approximately log(separators.size()) rounds where
 * each round "remove" the separators in the odd positions (indexing starting
 * from 0) by the appropriate merging.
 *
 * Complexity:
 *   - Work:  O(k*M*log2(k))
 *   - Depth: O(k*M*log2(k)/p)
 * Where:
 *   1. "k": the number of lists to merge. `k == arrays.size()`.
 *   2. "M": the size of the longest list in arrays.
 *   3. "p": the number of processor(s) used.
 *
 * @tparam InputIterator type of the iterator used as separator.
 * @tparam Comp type of the comparator used to merge.
 * @param separators list of iterators over the contiguous sequence to merge.
 * @param separators_size size of @a separators.
 * @param comp instance of Comp used to merge.
 */
template <
        typename InputIterator,
        typename Comp = std::less<typename std::iterator_traits<InputIterator>::value_type>
>
void iterative_merge(std::forward_list<InputIterator> & separators,
                     std::size_t separators_size,
                     Comp comp = Comp()) {
    // Directly start the parallel region to merge concurrently.
#pragma omp parallel
    {
#pragma omp single nowait
        {
            // Pre-compute the number of merging pass (or "round") we will need
            // to do in order to have a fully merged container.
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

}  // namespace detail

#endif  // INCLUDE_DETAIL_ITERATIVE_MERGE_H_
