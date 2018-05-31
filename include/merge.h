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

#include "include/detail/get_container_size.h"
#include "include/detail/first_round_merge.h"
#include "include/detail/iterative_merge.h"

#ifdef _OPENMP
#include "omp.h"
#endif

template <
        template <class,class> typename Container,
        typename T
> using Container_t = Container<T, std::allocator<T>>;


/**
 * @brief Efficient stable k-way merge.
 *
 * This function merges k sorted STL-like containers into one sorted container
 * of the same type.
 *
 * The merge is stable:
 * -# Ordering of equivalent elements within the same container is preserved.
 * -# Ordering of equivalent elements from different containers follows the
 * containers ordering.
 *
 * The function is parallelised using OpenMP when the library is available.
 *
 * Overall complexity is:
 *
 *
 * @tparam ContainerOfContainers structure containing the sorted container to
 * merge.
 * @tparam Comp Comparator used to merge the lists. Each list in @p arrays
 * should be sorted according to this comparator.
 * @param arrays k sorted lists to merge.
 * @param comp See template parameter @p Comp.
 * @return a sorted container.
 * @warning For the moment, InternalContainer should be std::vector.
 */
template <
        typename ContainerOfContainers,
        typename Comp = std::less<typename ContainerOfContainers::value_type::value_type>
>
typename ContainerOfContainers::value_type merge_arrays(ContainerOfContainers const & arrays, Comp comp = Comp()) {

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
    using ExternalContainer_t = ContainerOfContainers;
    using InternalContainer_t = typename ContainerOfContainers::value_type;
    using StoredDataType_t    = typename InternalContainer_t::value_type;

    // 1.1. Check that the ExternalContainer can be iterated on with at least a ForwardIterator.
    using ExternalContainerIterator = typename ExternalContainer_t::iterator;
    using ExternalContainerIteratorCategory = typename std::iterator_traits<ExternalContainerIterator>::iterator_category;
    static_assert(std::is_same<ExternalContainerIteratorCategory, std::forward_iterator_tag>::value ||
                  std::is_same<ExternalContainerIteratorCategory, std::bidirectional_iterator_tag>::value ||
                  std::is_same<ExternalContainerIteratorCategory, std::random_access_iterator_tag>::value,
                  "The type used as external container should be ForwardIterable");
    // 1.2. For the moment, the InternalContainer can only be a std::vector.
    static_assert(std::is_same<InternalContainer_t, std::vector<StoredDataType_t>>::value,
                  "The type used as internal container should be std::vector.");

    // 2. Create the resulting structure and reserve enough space for it.
    // Complexity: - Sequential: O(k + N*TDefInit)  ==  O(N*TDefInit)
    //             - Parallel:   O(k + N*TDefInit)  ==  O(N*TDefInit)

    // 2.1. Computing the total number of elements in order to save some re-allocations.
    // Complexity: O(k)
    auto add_size = [](std::size_t const & previous_size, std::vector<StoredDataType_t> const & elem) {
        return previous_size + elem.size();
    };
    std::size_t const number_of_elements{
            std::accumulate(arrays.begin(), arrays.end(), static_cast<std::size_t>(0), add_size)};
    // 2.2. Create the structure.
    // Complexity: - Sequential: O(N*TDefInit)
    //             - Parallel:   O(N*TDefInit)

    // Here we need to reserve enough memory because std::merge will write on it.
    // The interface of std::vector does not allow us to use memory without
    // initialising it so a simple "reserve" is not enough. A resize should be
    // employed, which is equivalent to creating the vector with enough
    // default-initialised elements.
    InternalContainer_t result(number_of_elements);

    // 3. Declare some useful constants for the following steps.
    // Complexity: O(1) if ExternalContainer supports an O(1) size method.
    //             O(k) if ExternalContainer is std::forward_list.
    std::size_t const number_of_lists_to_merge{detail::get_container_size(arrays)};
    std::size_t const block_number_after_first_round{number_of_lists_to_merge / 2 + number_of_lists_to_merge % 2};
    std::size_t const block_separator_number_after_first_round{block_number_after_first_round - 1};

    // 4. Handle simple cases:
    if (number_of_lists_to_merge == 0) {
        return result;
    } else if (number_of_lists_to_merge == 1) {
        auto const front = *(arrays.begin());
        std::copy(front.begin(), front.end(), result.begin());
        return result;
    } else if (number_of_lists_to_merge == 2) {
        auto const first = arrays.begin();
        auto const second = std::next(first);
        std::merge(first->begin(), first->end(), second->begin(), second->end(), result.begin());
        return result;
    }

    // 5. First step of the merge: concurrently merge adjacent containers into the memory space
    // allocated for the result.
    auto separators = detail::first_round_merge(arrays, result, comp);

    // 6. Now result contains all the values, we just need to merge all the lists not merged in the previous step.
    detail::iterative_merge(result, separators, block_separator_number_after_first_round, comp);

    // Return the result
    return result;
}

#endif //MULTI_MERGE_MERGE_H
