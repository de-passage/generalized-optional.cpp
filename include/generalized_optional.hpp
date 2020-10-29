#ifndef GUARD_GENERALIZED_OPTIONAL_HEADER
#define GUARD_GENERALIZED_OPTIONAL_HEADER

#include <exception>
#include <initializer_list>
#include <type_traits>
#include <utility>

namespace dpsg {

namespace detail {
template <class T>
using remove_cvref_t = std::remove_cv_t<std::remove_reference_t<T>>;

template <class T> class generalized_optional_storage {
  static_assert(!std::is_reference_v<T>,
                "generalized_optional cannot contain a reference type. Store a "
                "reference_wrapper or equivalent.");

protected:
  std::aligned_storage_t<sizeof(T), alignof(T)> _storage;

  constexpr generalized_optional_storage() = default;

  constexpr T *get_ptr() noexcept { return static_cast<T *>(&_storage); }
  constexpr T *get_ptr() const noexcept {
    return const_cast<generalized_optional_storage *>(this)->get_ptr();
  }
  constexpr T &&get_ref() &&noexcept {
    return reinterpret_cast<T &&>(_storage);
  }
  constexpr T &&get_ref() const &&noexcept {
    return reinterpret_cast<T &&>(_storage);
  }
  constexpr T &get_ref() &noexcept { return reinterpret_cast<T &>(_storage); }
  constexpr const T &get_ref() const &noexcept {
    return reinterpret_cast<const T &>(_storage);
  }
  template <class... Args> constexpr void build(Args &&... args) {
    ::new (&_storage) T{std::forward<Args>(args)...};
  }
  constexpr void destroy() noexcept { get_ref().~T(); }
};
} // namespace detail

// Policies

namespace detail {
template <class B> struct base {
  constexpr B *self() noexcept { return static_cast<B *>(this); }
  constexpr const B *self() const noexcept {
    return static_cast<const B *>(this);
  }
};
} // namespace detail

template <class T, T V = T{}> struct tombstone {
  template <class B> struct type : detail::base<B> {
    constexpr bool has_value() const noexcept { return self()->get_ref() != V; }

  protected:
    constexpr type() noexcept { self()->build(V); }
    constexpr void value_set() const noexcept {}
    constexpr void value_unset() const noexcept {};
  };
};
struct dependent_bool {
  template <class B> struct type : detail::base<B> {
  protected:
    bool _has_value = false;

    constexpr type() noexcept = default;
    constexpr void value_set() noexcept { _has_value = true; }
    constexpr void value_unset() noexcept { _has_value = false; }

  public:
    constexpr bool has_value() const noexcept { return _has_value; }
  };
};

// Utilities
struct bad_optional_access : std::exception {
  const char *what() const override { return "bad optional access"; }
};

struct in_place_t {
} constexpr static inline in_place;
struct nullopt_t {
} constexpr static inline nullopt;

template <class T, class Policy>
class generalized_optional
    : detail::generalized_optional_storage<T>,
      public Policy::template type<generalized_optional<T, Policy>> {
public:
  using value_type = T;

private:
  using policy =
      typename Policy::template type<generalized_optional<value_type, Policy>>;
  using storage = detail::generalized_optional_storage<value_type>;
  using storage::build;
  using storage::destroy;
  constexpr void _clean() noexcept(std::is_nothrow_destructible_v<value_type>) {
    if (has_value()) {
      destroy();
      policy::value_unset();
    }
  }
  constexpr void
  _copy(const T &t) noexcept(std::is_nothrow_copy_constructible_v<value_type>) {
    build(t);
    policy::value_set();
  }
  constexpr void
  _move(T &&t) noexcept(std::is_nothrow_move_constructible_v<value_type>) {
    build(std::move(t));
    policy::value_set();
  }

public:
  using policy::has_value;
  constexpr generalized_optional() noexcept = default;
  constexpr generalized_optional(nullopt_t) noexcept {}
  constexpr generalized_optional(const generalized_optional &other) noexcept(
      std::is_nothrow_copy_constructible_v<T>) {
    if (other.has_value()) {
      _copy(other.get_ref());
    }
  }
  constexpr generalized_optional(generalized_optional &&other) noexcept(
      std::is_nothrow_move_constructible_v<T>) {
    if (other.has_value()) {
      _move(std::move(other));
    }
  }
  template <class U, class P,
            std::enable_if_t<std::is_constructible_v<T, U>> = 0>
  generalized_optional(const generalized_optional<U, P> &other) noexcept(
      std::is_nothrow_constructible_v<T, U>) {
    if (other.has_value()) {
      _copy(other.get_ref());
    }
  }
  template <class U, class P>
  generalized_optional(generalized_optional<U, P> &&other) noexcept(
      std::is_nothrow_constructible_v<T, U>) {
    if (other.has_value()) {
      _move(std::move(other).get_ref());
    }
  }
  template <class... Args>
  constexpr explicit generalized_optional(in_place_t, Args &&... args) {
    storage::build(std::forward<Args>(args)...);
    policy::value_set();
  }
  template <class U, class... Args>
  constexpr explicit generalized_optional(in_place_t,
                                          std::initializer_list<U> ilist,
                                          Args &&... args);

private:
  template <class Ty>
  using allow_direct_conversion = std::bool_constant<std::conjunction_v<
      std::negation<
          std::is_same<detail::remove_cvref_t<Ty>, generalized_optional>>,
      std::negation<std::is_same<detail::remove_cvref_t<Ty>, in_place_t>>,
      std::is_constructible<value_type, Ty>>>;

public:
  template <
      class U = value_type,
      std::enable_if_t<std::conjunction_v<allow_direct_conversion<U>,
                                          std::is_convertible<U, value_type>>,
                       int> = 0>
  constexpr generalized_optional(U &&value) {
    storage::build(std::forward<U>(value));
    policy::value_set();
  }
  template <
      class U = value_type,
      std::enable_if_t<
          std::conjunction_v<allow_direct_conversion<U>,
                             std::negation<std::is_convertible<U, value_type>>>,
          int> = 0>
  constexpr explicit generalized_optional(U &&value) {
    storage::build(std::forward<U>(value));
    policy::value_set();
  }

  ~generalized_optional() { _clean(); }

  generalized_optional &operator=(nullopt_t) noexcept { _clean(); }

  constexpr generalized_optional &operator=(const generalized_optional &other) {
    if (other.has_value()) {
      if (has_value()) {
        get_ref() = other.get_ref();
      } else {
        _copy(other.get_ref());
      }
    } else {
      _clean();
    }
  }

  constexpr generalized_optional &
  operator=(generalized_optional &&other) noexcept(
      std::is_nothrow_move_assignable<T>::value
          &&std::is_nothrow_move_constructible<T>::value) {
    if (other.has_value()) {
      if (has_value()) {
        get_ref() = std::move(other).get_ref();
      } else {
        _move(std::move(other).get_ref());
      }
    } else {
      _clean();
    }
  }

  template <class U = value_type>
  constexpr generalized_optional &operator=(U &&value) {
    get_ref() = std::forward<U>(value);
  }

  template <class U, class P>
  constexpr generalized_optional &
  operator=(const generalized_optional<U, P> &other) {
    if (other.has_value()) {
      if (has_value()) {
        get_ref() = other.get_ref();
      } else {
        _copy(other.get_ref());
      }
    } else {
      _clean();
    }
  }

  template <class U, class P>
  constexpr generalized_optional &
  operator=(generalized_optional<U, P> &&other) {
    if (other.has_value()) {
      if (has_value()) {
        get_ref() = std::move(other).get_ref();
      } else {
        _move(std::move(other).get_ref());
      }
    } else {
      _clean();
    }
  }

  constexpr const T *operator->() const { return storage::get_ptr(); }
  constexpr T *operator->() { return storage::get_ptr(); }
  constexpr const T &operator*() const & { return storage::get_ref(); }
  constexpr T &operator*() & { return storage::get_ref(); }
  constexpr const T &&operator*() const && { return storage::get_ref(); }
  constexpr T &&operator*() && { return storage::get_ref(); }
  constexpr explicit operator bool() const noexcept {
    return policy::has_value();
  }

  constexpr T &value() & { return storage::get_ref(); }
  constexpr const T &value() const & { return storage::get_ref(); }
  constexpr T &&value() && { return storage::get_ref(); }
  constexpr const T &&value() const && { return storage::get_ref(); }

  template <class U> constexpr T value_or(U &&default_value) const & {
    if (has_value())
      return storage::get_ref();
    else
      return static_cast<T>(std::forward<U>(default_value));
  }
  template <class U> constexpr T value_or(U &&default_value) && {
    if (has_value())
      return storage::get_ref();
    else
      return static_cast<T>(std::forward<U>(default_value));
  }

  void swap(generalized_optional &other) noexcept(
      std::is_nothrow_move_constructible_v<T>
          &&std::is_nothrow_swappable_v<T>) {
    using namespace std;
    if (has_value()) {
      if (other.has_value()) {
        swap(get_ref(), other.get_ref());
      } else {
        other._move(std::move(get_ref()));
        _clean();
      }
    } else if (other.has_value()) {
      _move(std::move(other.get_ref()));
      other._clean();
    }
  }

  void reset() noexcept { _clean(); }
  template <class... Args> T &emplace(Args &&... args) {
    if (has_value()) {
      storage::destroy();
    }
    storage::build(std::forward<Args>(args)...);
  }

  template <class U, class... Args>
  T &emplace(std::initializer_list<U> ilist, Args &&... args) {
    if (has_value()) {
      storage::destroy();
    }
    storage::build(std::move(ilist), std::forward<Args>(args)...);
  }
};

template <class T> using optional = generalized_optional<T, dependent_bool>;

} // namespace dpsg

#endif // GUARD_GENERALIZED_OPTIONAL_HEADER