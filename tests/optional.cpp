#include "generalized_optional.hpp"
#include <gtest/gtest.h>

#include <initializer_list>
#include <string>
#include <vector>

using namespace std;

constexpr static inline const char *hello_world = "Hello World!";
constexpr static inline int fourtytwo = 42;

TEST(Optional, Ctor) {
  dpsg::optional<int> i;
  dpsg::optional<int> i2{dpsg::nullopt};
  ASSERT_FALSE(i.has_value());
  ASSERT_FALSE(i2.has_value());
  dpsg::optional<int> i3{fourtytwo};
  ASSERT_TRUE(i3.has_value());
  ASSERT_EQ(*i3, 42);
  dpsg::optional<string> i4{hello_world};
  ASSERT_TRUE(i4.has_value());
  ASSERT_EQ(*i4, hello_world);
  string hw = hello_world;
  dpsg::optional<string> s1{hw};
  ASSERT_TRUE(s1.has_value());
  ASSERT_EQ(*s1, hello_world);
  dpsg::optional<string> s2{dpsg::in_place};
  ASSERT_TRUE(s2.has_value());
  ASSERT_EQ(*s2, "");
  dpsg::optional<int> i5 = {};
  ASSERT_FALSE(i5.has_value());
}

TEST(Optional, CopyCtor) {
  dpsg::optional<string> s;
  dpsg::optional<string> s2{s}; // NOLINT testing copy ctor
  ASSERT_FALSE(s2.has_value());
  dpsg::optional<string> s3{hello_world};
  dpsg::optional<string> s4{s3};
  ASSERT_TRUE(s4.has_value());
  ASSERT_EQ(*s4, hello_world);
}

struct move_recorder {
  constexpr move_recorder() noexcept = default;
  constexpr move_recorder(const move_recorder &) noexcept = default;
  constexpr move_recorder(move_recorder &&other) noexcept : moved_into(true) {
    other.moved_from = true;
  }
  move_recorder &operator=(const move_recorder &) noexcept = default;
  move_recorder &operator=(move_recorder &&other) noexcept {
    other.moved_from = true;
    moved_into = true;
    return *this;
  }
  ~move_recorder() = default;
  bool moved_from = false;
  bool moved_into = false;
};

TEST(Optional, MoveCtor) {
  dpsg::optional<string> s;
  dpsg::optional<string> s2{std::move(s)};
  ASSERT_FALSE(s2.has_value());
  using om = dpsg::optional<move_recorder>;
  om m{dpsg::in_place};
  om m2{std::move(m)};
  ASSERT_TRUE(m.has_value()); // NOLINT intentional
  ASSERT_TRUE(m2.has_value());
  ASSERT_TRUE(m->moved_from); // NOLINT intentional
  ASSERT_TRUE(m2->moved_into);
}