#include <gtest/gtest.h>

#include "generalized_optional.hpp"

constexpr static inline int fourty_two = 42;

template <class T> struct t {
  const T v = {};
};

struct t2 {
  static int foo() { return fourty_two; }
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

TEST(Access, ExceptionOnCheckedDeref) {
  using o = dpsg::generalized_optional<
      t2, dpsg::policy<dpsg::access::throw_exception,
                       dpsg::control::dependent_bool, dpsg::storage::aligned>>;
  o i;
  ASSERT_THROW(*i, dpsg::bad_optional_access);       // NOLINT
  ASSERT_THROW(*o{}, dpsg::bad_optional_access);     // NOLINT
  ASSERT_THROW(i->foo(), dpsg::bad_optional_access); // NOLINT
                                                     // NOLINTNEXTLINE
  ASSERT_THROW(
      o {}->foo(), dpsg::bad_optional_access);
  t<o> i2;
  ASSERT_THROW(*i2.v, dpsg::bad_optional_access);           // NOLINT
  ASSERT_THROW(*t<o>{}.v, dpsg::bad_optional_access);       // NOLINT
  ASSERT_THROW(i2.v->foo(), dpsg::bad_optional_access);     // NOLINT
  ASSERT_THROW(t<o>{}.v->foo(), dpsg::bad_optional_access); // NOLINT
}

TEST(Access, NoExceptionOnUncheckedDeref) {
  using o = dpsg::generalized_optional<
      t2, dpsg::policy<dpsg::access::unchecked, dpsg::control::dependent_bool,
                       dpsg::storage::aligned>>;
  o i;
  ASSERT_NO_THROW(*i);          // NOLINT
  ASSERT_NO_THROW(*o{});        // NOLINT
  ASSERT_NO_THROW(i->foo());    // NOLINT
  ASSERT_NO_THROW(o {}->foo()); // NOLINT
  t<o> i2;
  ASSERT_NO_THROW(*i2.v);           // NOLINT
  ASSERT_NO_THROW(*t<o>{}.v);       // NOLINT
  ASSERT_NO_THROW(i2.v->foo());     // NOLINT
  ASSERT_NO_THROW(t<o>{}.v->foo()); // NOLINT
}