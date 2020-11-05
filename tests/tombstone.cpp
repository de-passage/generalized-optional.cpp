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
  ASSERT_EQ(*u1, 0);
  tsd<int> i4{dpsg::in_place, fourty_two};
  ASSERT_TRUE(i4.has_value());
  ASSERT_EQ(*i4, fourty_two);
}

TEST(Tombstone, CopyCtor) {
  tsd<int> i1;
  tsd<int> i2{i1}; // NOLINT intentional
  ASSERT_FALSE(i2.has_value());
  tsd<int> i3{fourty_two};
  tsd<int> i4{i3};
  ASSERT_TRUE(i4.has_value());
  ASSERT_EQ(*i4, fourty_two);
}

TEST(Tombstone, MoveCtor) {
  // Mostly tests compilation, moving & copying are the same in this case
  tsd<int> i1;
  tsd<int> i2{std::move(i1)};
  ASSERT_FALSE(i2.has_value());
  tsd<int> i3{fourty_two};
  tsd<int> i4{std::move(i3)};
  ASSERT_TRUE(i4.has_value());
  ASSERT_EQ(i4.value(), fourty_two);
}

TEST(Tombstone, CopyAssignment) {
  tsd<int> i1{fourty_two};
  tsd<int> i2;
  i1 = i2;
  ASSERT_FALSE(i1.has_value());
  i1 = fourty_two;
  ASSERT_TRUE(i1.has_value());
  ASSERT_EQ(*i1, fourty_two);
  i2 = i1;
  ASSERT_TRUE(i2.has_value());
  ASSERT_EQ(*i2, fourty_two);
}

TEST(Tombstone, MoveAssignment) {
  tsd<int> i1{fourty_two};
  tsd<int> i2;
  i1 = std::move(i2);
  ASSERT_FALSE(i1.has_value());
  i1 = std::move(fourty_two); // NOLINT intentional, tests compilation &
                              // behaviour of wrapper
  ASSERT_TRUE(i1.has_value());
  ASSERT_EQ(*i1, fourty_two);
  i2 = std::move(i1);
  ASSERT_TRUE(i2.has_value());
  ASSERT_EQ(*i2, fourty_two);
}