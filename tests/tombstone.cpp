#include <gtest/gtest.h>

#include "generalized_optional.hpp"

template <class T> using tsd = dpsg::optional_tombstone<T>;
template <class T, T V> using ts = dpsg::optional_tombstone<T, V>;
constexpr static inline auto fourty_two = 42;

TEST(Tombstone, Ctor) {
  tsd<int> i;
  ASSERT_FALSE(i.has_value());
  tsd<int> i2{fourty_two};
  ASSERT_TRUE(i2.has_value());
  ASSERT_EQ(*i2, fourty_two);
  tsd<int> i3{0};
  ASSERT_TRUE(i2.has_value());
  tsd<unsigned> u1{0U};
  ASSERT_TRUE(u1.has_value());
}

TEST(Tombstone, CopyCtor) {
  tsd<int> i1;
  tsd<int> i2{i1};
  ASSERT_FALSE(i2.has_value());
  tsd<int> i3{fourty_two};
  tsd<int> i4{i3};
  ASSERT_TRUE(i4.has_value());
  ASSERT_EQ(*i4, fourty_two);
}