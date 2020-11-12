#include "generalized_optional.hpp"

#include <gtest/gtest.h>

#include <string>
#include <vector>

template <class T>
struct show_impl : dpsg::storage::aligned<T>::template type<
                       dpsg::detail::base<show_impl<T>>> {
  using base = typename dpsg::storage::aligned<T>::template type<
      dpsg::detail::base<show_impl<T>>>;
  using base::base;
  using base::build;
  using base::destroy;
  using base::get_ptr;
  using base::get_ref;
};

TEST(GeneralizedOptionalStorage, ShouldWork) {
  using namespace std;
  show_impl<vector<string>> g;
  initializer_list<string> l{"Hello", " World!",
                             " This is a fairly long character string!!"};
  g.build(l);
  const auto *it = begin(l);
  for (auto &v : g.get_ref()) {
    ASSERT_EQ(v, *it++); // NOLINT
  }

  g.destroy();
  g.build(std::vector<string>({"1", "2", "3", "4"}));
  int r = 1;
  for (auto &v : g.get_ref()) {
    ASSERT_EQ(to_string(r++), v);
  }
  g.destroy();
}