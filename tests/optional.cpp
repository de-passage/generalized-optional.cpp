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

template <class T>
struct show_impl : dpsg::detail::generalized_optional_storage<T> {
  using dpsg::detail::generalized_optional_storage<T>::build;
  using dpsg::detail::generalized_optional_storage<T>::destroy;
  using dpsg::detail::generalized_optional_storage<T>::get_ptr;
  using dpsg::detail::generalized_optional_storage<T>::get_ref;
  using dpsg::detail::generalized_optional_storage<
      T>::generalized_optional_storage;
};

TEST(GeneralizedOptionalStorage, ShouldWork) {
  using namespace std;
  show_impl<vector<string>> g;
  initializer_list<string> l{"Hello", " World!",
                             " This is a fairly long character string!!"};
  g.build(l);
  auto it = begin(l);
  for (auto &v : g.get_ref()) {
    ASSERT_EQ(v, *it++);
  }

  g.destroy();
  g.build(std::vector<string>({"1", "2", "3", "4"}));
  int r = 1;
  for (auto &v : g.get_ref()) {
    ASSERT_EQ(to_string(r++), v);
  }
  g.destroy();
}