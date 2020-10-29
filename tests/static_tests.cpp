#include "generalized_optional.hpp"
#include <string>
#include <type_traits>
#include <vector>

using namespace std;

template <class T> struct test_size_and_alignment {
  struct sized {
    bool b;
    T t;
  };
  static_assert(alignof(T) == alignof(dpsg::optional<T>));
  static_assert(sizeof(sized) == sizeof(dpsg::optional<T>));
  template <class> using type = void;
};

template <class... Args> struct test_all;
template <> struct test_all<> { using type = void; };
template <class T, class... Args> struct test_all<T, Args...> {
  using type = typename test_size_and_alignment<T>::template type<
      typename test_all<Args...>::type>;
};

static_assert(std::is_same_v<typename test_all<char, int, double, string,
                                               vector<string>>::type,
                             void>);