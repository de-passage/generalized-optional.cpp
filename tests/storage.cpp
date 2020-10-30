#include "generalized_optional.hpp"

#include <gtest/gtest.h>

#include <string>
#include <vector>


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