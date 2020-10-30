#include <gtest/gtest.h>

#include "generalized_optional.hpp"

template <class T> using tsd = dpsg::optional_tombstone<T>;
template <class T, T V> using ts = dpsg::optional_tombstone<T, V>;

TEST(Tombstone, Ctor) {
  tsd<int> i;
  ASSERT_FALSE(i.has_value());
}