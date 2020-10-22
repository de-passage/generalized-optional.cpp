#ifndef GUARD_GENERALIZED_OPTIONAL_HEADER
#define GUARD_GENERALIZED_OPTIONAL_HEADER

#include <exception>
#include <type_traits>
#include <utility>

namespace dpsg {

namespace detail {
template <class T> class generalized_optional_storage {
protected:
  std::aligned_storage_t<sizeof(T), alignof(T)> _storage;
};
} // namespace detail

// Policies
template <class T, T V> struct tombstone {};
template <class T> struct dependent_value { T dependent_value; };

// Utilities
struct bad_optional_access : std::exception {
  const char *what() const override { return "bad optional access"; }
};

struct in_place_t {
} constexpr static inline in_place;
struct nullopt_t {
} constexpr static inline nullopt;

template <class T, class Policy> class generalized_optional : Policy {
public:
  using value_type = T;
  constexpr generalized_optional() noexcept;
  constexpr generalized_optional(nullopt_t) noexcept;
  constexpr generalized_optional(const generalized_optional &other);
  constexpr generalized_optional(generalized_optional &&other) noexcept(
      std::is_nothrow_move_constructible_v<T>);
  template <class U, class P>
  generalized_optional(const generalized_optional<U, P> &other);
  template <class U, class P>
  generalized_optional(generalized_optional<U, P> &&other);
  template <class... Args>
  constexpr explicit generalized_optional(in_place_t, Args &&... args);
  template <class U, class... Args>
  constexpr explicit generalized_optional(in_place_t,
                                          std::initializer_list<U> ilist,
                                          Args &&... args);
  template <class U = value_type> constexpr generalized_optional(U &&value);

  ~generalized_optional();

  generalized_optional &operator=(nullopt_t) noexcept;
  constexpr generalized_optional &operator=(const generalized_optional &other);
  constexpr generalized_optional &
  operator=(generalized_optional &&other) noexcept(
      std::is_nothrow_move_assignable<T>::value
          &&std::is_nothrow_move_constructible<T>::value);
  template <class U = value_type> generalized_optional &operator=(U &&value);
  template <class U, class P>
  generalized_optional &operator=(const generalized_optional<U, P> &other);
  template <class U, class P>
  generalized_optional &operator=(generalized_optional<U, P> &&other);

  constexpr const T *operator->() const;
  constexpr T *operator->();
  constexpr const T &operator*() const &;
  constexpr T &operator*() &;
  constexpr const T &&operator*() const &&;
  constexpr T &&operator*() &&;
  constexpr explicit operator bool() const noexcept;
  constexpr bool has_value() const noexcept;

  constexpr T &value() &;
  constexpr const T &value() const &;
  constexpr T &&value() &&;
  constexpr const T &&value() const &&;

  template <class U> constexpr T value_or(U &&default_value) const &;
  template <class U> constexpr T value_or(U &&default_value) &&;

  void swap(generalized_optional &other) noexcept(
      std::is_nothrow_move_constructible_v<T> &&std::is_nothrow_swappable_v<T>);

  void reset() noexcept;
  template <class... Args> T &emplace(Args &&... args);
  template <class U, class... Args>
  T &emplace(std::initializer_list<U> ilist, Args &&... args);

private:
};

template <class T>
using optional = generalized_optional<T, dependent_value<bool>>;

template <class T>
using optional_node = generalized_optional<T, dependent_value<T *>>;

} // namespace dpsg

#endif // GUARD_GENERALIZED_OPTIONAL_HEADER