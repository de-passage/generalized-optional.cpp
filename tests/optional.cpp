#include "generalized_optional.hpp"
#include <gtest/gtest.h>

#include <initializer_list>
#include <string>
#include <utility>
#include <vector>

using namespace std;

constexpr static inline const char *hello_world = "Hello World!";
constexpr static inline const char *something_else = "something else";
constexpr static inline const char *one_last_thing = "one last thing";
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
  ASSERT_FALSE(m->moved_from);
  om m2{std::move(m)};
  ASSERT_TRUE(m.has_value()); // NOLINT intentional
  ASSERT_TRUE(m2.has_value());
  ASSERT_TRUE(m->moved_from); // NOLINT intentional
  ASSERT_TRUE(m2->moved_into);
}

TEST(Optional, CopyAssign) {
  dpsg::optional<string> s;
  dpsg::optional<string> s2;
  s = s2;
  ASSERT_FALSE(s.has_value());
  ASSERT_FALSE(s2.has_value());

  string hw{hello_world};
  s = hw;
  s2 = s;
  ASSERT_TRUE(s.has_value());
  ASSERT_TRUE(s2.has_value());
  ASSERT_EQ(*s, *s2);

  using om = dpsg::optional<move_recorder>;
  om m{dpsg::nullopt};
  move_recorder mr1;
  ASSERT_FALSE(mr1.moved_from);
  ASSERT_FALSE(m.has_value());
  m = mr1;
  ASSERT_FALSE(mr1.moved_from);
  ASSERT_TRUE(m.has_value());
  ASSERT_FALSE(m->moved_into);
  om m2{mr1};
  ASSERT_FALSE(mr1.moved_from);
  ASSERT_TRUE(m2.has_value());
  ASSERT_FALSE(m2->moved_into);
  m = m2;
  ASSERT_FALSE(m2->moved_from);
  ASSERT_FALSE(m->moved_into);
}

TEST(Optional, MoveAssign) {
  dpsg::optional<string> s;
  dpsg::optional<string> s2;
  s = std::move(s2);
  ASSERT_FALSE(s.has_value());
  ASSERT_FALSE(s2.has_value()); // NOLINT intentional

  s = hello_world;
  s2 = std::move(s);
  ASSERT_TRUE(s.has_value()); // NOLINT intentional
  ASSERT_TRUE(s2.has_value());
  ASSERT_EQ(*s2, hello_world);

  using om = dpsg::optional<move_recorder>;
  om m1{dpsg::nullopt};
  move_recorder mr1;
  m1 = mr1;
  om m2{mr1};
  ASSERT_TRUE(m2.has_value());
  ASSERT_FALSE(m2->moved_into);
  m1 = std::move(m2);
  ASSERT_TRUE(m2.has_value()); // NOLINT intentional
  ASSERT_TRUE(m1.has_value());
  ASSERT_TRUE(m2->moved_from); // NOLINT intentional
  ASSERT_TRUE(m1->moved_into);
}

struct noncopyable {
  noncopyable() = default;
  noncopyable(noncopyable &&) = default;
  noncopyable(const noncopyable &) = delete;
  noncopyable &operator=(noncopyable &&) = default;
  noncopyable &operator=(const noncopyable &) = delete;
  ~noncopyable() = default;
};

TEST(Optional, Swap) {
  dpsg::optional<string> s;
  dpsg::optional<string> s2;
  s.swap(s2);
  ASSERT_FALSE(s.has_value());
  ASSERT_FALSE(s2.has_value());

  const string str{something_else};
  s = str;
  s2 = hello_world;
  s.swap(s2);
  ASSERT_TRUE(s.has_value());
  ASSERT_TRUE(s2.has_value());
  ASSERT_EQ(*s, hello_world);
  ASSERT_EQ(*s2, str);

  s.reset();
  s.swap(s2);
  ASSERT_TRUE(s.has_value());
  ASSERT_FALSE(s2.has_value());
  ASSERT_EQ(*s, str);

  s.swap(s2);
  ASSERT_TRUE(s2.has_value());
  ASSERT_FALSE(s.has_value());
  ASSERT_EQ(*s2, str);

  // checking compilation
  dpsg::optional<noncopyable> ncp1{dpsg::in_place};
  dpsg::optional<noncopyable> ncp2{dpsg::in_place};
  ncp1.swap(ncp2);
  swap(ncp1, ncp2);
}

TEST(Optional, ValueOr) {
  dpsg::optional<int> i1;
  dpsg::optional<int> i2{fourtytwo};
  ASSERT_EQ(i1.value_or(0), 0);
  ASSERT_EQ(i2.value_or(0), fourtytwo);
}

TEST(Optional, Emplace) {
  using ovs = dpsg::optional<vector<string>>;
  ovs o1;
  ovs o2{{something_else, hello_world, one_last_thing}};

  o1.emplace({string{hello_world}, string{something_else}});
  ASSERT_TRUE(o1.has_value());
  ASSERT_EQ((*o1)[0], hello_world);
  ASSERT_EQ((*o1)[1], something_else);

  o2.emplace(static_cast<std::size_t>(0));
  ASSERT_TRUE(o2.has_value());
  ASSERT_EQ(o2->size(), 0);
}