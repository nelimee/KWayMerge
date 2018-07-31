#ifndef INCLUDE_DETAIL_FIRST_ROUND_MERGE_H_
#define INCLUDE_DETAIL_FIRST_ROUND_MERGE_H_

#include <forward_list>
#include <algorithm>
#include <functional>

#include "include/detail/get_container_size.h"

#ifdef _OPENMP
#include "omp.h"
#endif

namespace detail {

template <typename Container> using Iterator_t = typename Container::iterator;

/**
 * Perform the first-round of k-way merge.
 *
 * The first round will merge the containers contained in @a container
 * in the container @a output. Containers 2*i and 2*i+1 are merged.
 *
 * Complexity:
 *   - Work:  O(M*Comp*k   + k + M)
 *   - Depth: O(M*Comp*k/p + k + M)
 * Where:
 *   1. "k": the number of lists to merge. `k == arrays.size()`.
 *   2. "M": the size of the longest list in arrays.
 *   3. "Comp": the number of operations needed to compare to objects of
 *              type T with the provided comparator.
 *   4. "p": the number of processor(s) used.
 *
 * @tparam ContainerOfContainers type of @a container.
 * @tparam OutputContainer type of the output container.
 * @tparam Comp comparator used to sort each container of @a container.
 * @param container a sequence of containers to merge.
 * @param output a pre-allocated output container that will be filled
 * with the merged arrays.
 * @param comp comparator used to sort each container of @a container.
 * @return a list of iterators over @a output elements. Each iterator (except
 * the last one which is @a output.end()) points to the beginning of a sorted
 * sequence.
 */
template <
        typename ContainerOfContainers,
        typename OutputContainer,
        typename Comp = std::less<typename ContainerOfContainers::value_type::value_type>
>
std::forward_list<Iterator_t<OutputContainer>>
first_round_merge(ContainerOfContainers const & container, OutputContainer & output, Comp comp = Comp()) {

    // 0. Useful constants definition.
    std::size_t const number_of_lists_to_merge{get_container_size(container)};
    std::size_t const block_number_after_first_round{number_of_lists_to_merge / 2 + number_of_lists_to_merge % 2};
    std::size_t const block_separator_number_after_first_round{block_number_after_first_round - 1};

    // 1. Create a list of pointers on the beginning of sorted-blocks.
    // This list is then returned by the function to keep track of the non-sorted
    // blocks that will need additional merges.
    // The last pointer contained in the list is the end of the vector.
    // Complexity: O(k)
    std::forward_list<Iterator_t<OutputContainer>> separators(2 + block_separator_number_after_first_round,
                                                              output.begin());
    // Iterator on the last valid element of separators.
    auto last_inserted = separators.begin();

    // 2. Merge concurrently adjacent containers
    // Complexity:
    //   - Work:  O(M*Comp*k) (at most (2*M-1) comparisons per merge, k/2 merges)
    //   - Depth: O(M*Comp*k/p)
    auto left = container.begin();
    decltype(left) right;
#pragma omp parallel
    {
#pragma omp single nowait
        {
            // We need to keep track of the beginning of the free section
            // in order to be able to keep the data contiguous.
            std::size_t next_free_position{0};
            // While there are at least 2 non-processed regions that should be
            // merged.
            while (left != container.end() && right != container.end()) {
                // First update the second iterator.
                right = std::next(left);
                // Increase the iterator over the separators. We want this
                // iterator to points to the end of the memory section we will
                // fill during the upcoming merge operation.
                ++last_inserted;
                // Then we declare a task that will be executed by one of the
                // workers. Tasks are independent and can be computed in any
                // order because each task copy the data it needs to complete
                // and the memory sections used by each task do not overlap.
#pragma omp task firstprivate(last_inserted, left, right, next_free_position)
                {
                    // Merge a memory portion and update our separator structure
                    // with an iterator pointing to the end of the memory section
                    // used.
                    *last_inserted = std::merge(left->begin(), left->end(),          /*input  1*/
                                                right->begin(), right->end(),        /*input  2*/
                                                output.begin() + next_free_position, /*output 1*/
                                                comp);                               /*comparator*/
                }
                // Update the next free position in the output vector.
                next_free_position += left->size() + right->size();
                left = std::next(right);
            }
        }
    }

    // 5.3. Take care of the last array if there is a odd number of arrays.
    // Complexity: O(M)
    if (number_of_lists_to_merge % 2) {
        auto const previously_inserted = last_inserted;
        ++last_inserted;
        *last_inserted = std::copy(left->begin(), left->end(), *previously_inserted);
    }

    return separators;
}

}  // namespace detail

#endif  // INCLUDE_DETAIL_FIRST_ROUND_MERGE_H_
