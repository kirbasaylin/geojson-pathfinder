#include "geojson_pathfinder/priority_queue.hpp"

#include <stdexcept>
#include <utility>

namespace gjp {

IndexedMinHeap::IndexedMinHeap(std::size_t capacity)
    : position_(capacity, kNotInHeap) {
    heap_.reserve(capacity);
}

bool IndexedMinHeap::contains(NodeId id) const {
    return id < position_.size() && position_[id] != kNotInHeap;
}

void IndexedMinHeap::push(NodeId id, double key) {
    if (contains(id)) {
        throw std::logic_error("IndexedMinHeap::push: id already present");
    }
    const std::size_t idx = heap_.size();
    heap_.push_back(Entry{id, key});
    position_[id] = idx;
    sift_up(idx);
}

void IndexedMinHeap::decrease_key(NodeId id, double new_key) {
    const std::size_t idx = position_.at(id);
    if (idx == kNotInHeap) {
        throw std::logic_error("IndexedMinHeap::decrease_key: id not in heap");
    }
    if (new_key > heap_[idx].key) {
        throw std::logic_error("IndexedMinHeap::decrease_key: new key greater than current");
    }
    heap_[idx].key = new_key;
    sift_up(idx);
}

std::pair<NodeId, double> IndexedMinHeap::pop_min() {
    if (heap_.empty()) {
        throw std::logic_error("IndexedMinHeap::pop_min: empty heap");
    }
    const Entry top = heap_.front();
    const Entry last = heap_.back();
    heap_.pop_back();
    position_[top.id] = kNotInHeap;
    if (!heap_.empty()) {
        heap_.front() = last;
        position_[last.id] = 0;
        sift_down(0);
    }
    return {top.id, top.key};
}

double IndexedMinHeap::key_of(NodeId id) const {
    const std::size_t idx = position_.at(id);
    if (idx == kNotInHeap) {
        throw std::logic_error("IndexedMinHeap::key_of: id not in heap");
    }
    return heap_[idx].key;
}

void IndexedMinHeap::sift_up(std::size_t i) {
    while (i > 0) {
        const std::size_t parent = (i - 1) / 2;
        if (heap_[i].key < heap_[parent].key) {
            swap_entries(i, parent);
            i = parent;
        } else {
            break;
        }
    }
}

void IndexedMinHeap::sift_down(std::size_t i) {
    const std::size_t n = heap_.size();
    while (true) {
        const std::size_t l = 2 * i + 1;
        const std::size_t r = 2 * i + 2;
        std::size_t smallest = i;
        if (l < n && heap_[l].key < heap_[smallest].key) smallest = l;
        if (r < n && heap_[r].key < heap_[smallest].key) smallest = r;
        if (smallest == i) break;
        swap_entries(i, smallest);
        i = smallest;
    }
}

void IndexedMinHeap::swap_entries(std::size_t i, std::size_t j) {
    std::swap(heap_[i], heap_[j]);
    position_[heap_[i].id] = i;
    position_[heap_[j].id] = j;
}

}  // namespace gjp
