#include "generalized_optional.hpp"
#include <gtest/gtest.h>

#include <initializer_list>
#include <string>
#include <vector>

using namespace std;

constexpr static inline const char *hello_world = "Hello World!";

TEST(Optional, Ctor) {
  dpsg::optional<int> i;
  dpsg::optional<int> i2{dpsg::nullopt};
  ASSERT_FALSE(i.has_value());
  ASSERT_FALSE(i2.has_value());
  dpsg::optional<int> i3{42};
  ASSERT_TRUE(i3.has_value());
  ASSERT_EQ(*i3, 42);
  dpsg::optional<string> i4{hello_world};
  ASSERT_TRUE(i4.has_value());
  ASSERT_EQ(*i4, hello_world);
  string hw = hello_world;
  dpsg::optional<string> s1{hw};
  ASSERT_TRUE(s1.has_value());
  ASSERT_EQ(*s1, hello_world);
}

TEST(Optional, CopyCtor) {
  dpsg::optional<string> s;
  dpsg::optional<string> s2{s};
  ASSERT_FALSE(s2.has_value());
  dpsg::optional<string> s3{hello_world};
  dpsg::optional<string> s4{s3};
  ASSERT_TRUE(s4.has_value());
  ASSERT_EQ(*s4, hello_world);
}

TEST(Optional, MoveCtor) {
  dpsg::optional<string> s;
  dpsg::optional<string> s2{std::move(s)};
  ASSERT_FALSE(s2.has_value());
}