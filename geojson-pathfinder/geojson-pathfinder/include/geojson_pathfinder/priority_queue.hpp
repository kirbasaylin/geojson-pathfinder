#pragma once

#include "geojson_pathfinder/types.hpp"

#include <cstddef>
#include <vector>

namespace gjp {

// Indexed binary min-heap.
// Supports O(log n) insert, decrease-key, extract-min.
// Position map lets us update a node's key without scanning the heap.
class IndexedMinHeap {
public:
    explicit IndexedMinHeap(std::size_t capacity);

    [[nodiscard]] bool empty() const noexcept { return heap_.empty(); }
    [[nodiscard]] std::size_t size() const noexcept { return heap_.size(); }
    [[nodiscard]] bool contains(NodeId id) const;

    // Insert id with given key. Precondition: !contains(id).
    void push(NodeId id, double key);

    // Decrease the key for id. Precondition: contains(id), new_key < current.
    void decrease_key(NodeId id, double new_key);

    // Remove and return the (id, key) with smallest key.
    std::pair<NodeId, double> pop_min();

    [[nodiscard]] double key_of(NodeId id) const;

private:
    struct Entry {
        NodeId id{kInvalidNode};
        double key{0.0};
    };

    std::vector<Entry> heap_{};
    std::vector<std::size_t> position_{};  // position_[id] = index in heap_, or kNotInHeap
    static constexpr std::size_t kNotInHeap = static_cast<std::size_t>(-1);

    void sift_up(std::size_t i);
    void sift_down(std::size_t i);
    void swap_entries(std::size_t i, std::size_t j);
};

}  // namespace gjp
