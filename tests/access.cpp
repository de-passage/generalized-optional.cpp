#include <gtest/gtest.h>

#include "generalized_optional.hpp"

template <class T> struct t {
  const T v = {};
};

TEST(Access, ExceptionOnValue) {
  using o = dpsg::optional<int>;
  o i;
  ASSERT_THROW(i.value(), dpsg::bad_optional_access);   // NOLINT
  ASSERT_THROW(o{}.value(), dpsg::bad_optional_access); // NOLINT
  t<o> i2;
  ASSERT_THROW(i2.v.value(), dpsg::bad_optional_access);     // NOLINT
  ASSERT_THROW(t<o>{}.v.value(), dpsg::bad_optional_access); // NOLINT
}

TEST(Access, NoExceptionOnUncheckedValue) {
  using o = dpsg::generalized_optional<
      int, dpsg::policy<dpsg::access::unchecked, dpsg::control::dependent_bool,
                        dpsg::storage::aligned>>;
  o i;
  ASSERT_NO_THROW(i.value());   // NOLINT
  ASSERT_NO_THROW(o{}.value()); // NOLINT
  t<o> i2;
  ASSERT_NO_THROW(i2.v.value());     // NOLINT
  ASSERT_NO_THROW(t<o>{}.v.value()); // NOLINT
}