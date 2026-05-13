#include "geojson_pathfinder/priority_queue.hpp"

#include <gtest/gtest.h>

#include <stdexcept>

using namespace gjp;

TEST(IndexedMinHeap, PushPopOrdered) {
    IndexedMinHeap h(10);
    h.push(0, 5.0);
    h.push(1, 2.0);
    h.push(2, 8.0);
    h.push(3, 1.0);

    auto [a, ka] = h.pop_min(); EXPECT_EQ(a, 3u); EXPECT_DOUBLE_EQ(ka, 1.0);
    auto [b, kb] = h.pop_min(); EXPECT_EQ(b, 1u); EXPECT_DOUBLE_EQ(kb, 2.0);
    auto [c, kc] = h.pop_min(); EXPECT_EQ(c, 0u); EXPECT_DOUBLE_EQ(kc, 5.0);
    auto [d, kd] = h.pop_min(); EXPECT_EQ(d, 2u); EXPECT_DOUBLE_EQ(kd, 8.0);
    EXPECT_TRUE(h.empty());
}

TEST(IndexedMinHeap, DecreaseKey) {
    IndexedMinHeap h(10);
    h.push(0, 5.0);
    h.push(1, 3.0);
    h.push(2, 7.0);
    h.decrease_key(2, 1.0);  // now 2 should be smallest

    auto [id, key] = h.pop_min();
    EXPECT_EQ(id, 2u);
    EXPECT_DOUBLE_EQ(key, 1.0);
}

TEST(IndexedMinHeap, ContainsTracksMembership) {
    IndexedMinHeap h(5);
    EXPECT_FALSE(h.contains(0));
    h.push(0, 1.0);
    EXPECT_TRUE(h.contains(0));
    h.pop_min();
    EXPECT_FALSE(h.contains(0));
}

TEST(IndexedMinHeap, PopEmptyThrows) {
    IndexedMinHeap h(5);
    EXPECT_THROW(h.pop_min(), std::logic_error);
}

TEST(IndexedMinHeap, DuplicatePushThrows) {
    IndexedMinHeap h(5);
    h.push(0, 1.0);
    EXPECT_THROW(h.push(0, 2.0), std::logic_error);
}

TEST(IndexedMinHeap, DecreaseKeyWithLargerValueThrows) {
    IndexedMinHeap h(5);
    h.push(0, 1.0);
    EXPECT_THROW(h.decrease_key(0, 2.0), std::logic_error);
}
