#ifndef GUARD_GENERALIZED_OPTIONAL_HEADER
#define GUARD_GENERALIZED_OPTIONAL_HEADER

#include <exception>
#include <initializer_list>
#include <limits>
#include <optional>
#include <type_traits>
#include <utility>

namespace dpsg {

struct in_place_t {
} constexpr static inline in_place;

namespace detail {
template <class T>
using remove_cvref_t = std::remove_cv_t<std::remove_reference_t<T>>;
} // namespace detail

namespace storage {
struct aligned {
  template <class B> class type : public B {
  private:
    using T = typename B::type;
    static_assert(
        !std::is_reference_v<T>,
        "generalized_optional cannot contain a reference type. Store a "
        "reference_wrapper or equivalent.");

  protected:
    std::aligned_storage_t<sizeof(T), alignof(T)> _storage;

    constexpr type() = default;

    constexpr explicit type([[maybe_unused]] std::nullopt_t marker) {}

    template <class... Args>
    constexpr explicit type(
        [[maybe_unused]] in_place_t marker,
        Args &&... args) noexcept(std::is_nothrow_constructible_v<T, Args...>) {
      build(std::forward<Args>(args)...);
    }

    constexpr T *get_ptr() noexcept {
      return reinterpret_cast<T *>(&_storage); // NOLINT
    }
    constexpr const T *get_ptr() const noexcept {
      return reinterpret_cast<const T *>(&_storage); // NOLINT
    }
    constexpr T &&get_ref() &&noexcept {
      return reinterpret_cast<T &&>(_storage); // NOLINT
    }
    constexpr const T &&get_ref() const &&noexcept {
      return reinterpret_cast<const T &&>(_storage); // NOLINT
    }
    constexpr T &get_ref() &noexcept {
      return reinterpret_cast<T &>(_storage); // NOLINT
    }
    constexpr const T &get_ref() const &noexcept {
      return reinterpret_cast<const T &>(_storage); // NOLINT
    }
    template <class... Args> constexpr void build(Args &&... args) {
      ::new (&_storage) T{std::forward<Args>(args)...};
    }
    constexpr void destroy() noexcept { get_ref().~T(); }
  };
};

} // namespace storage

// Policies

namespace detail {
template <class T> struct extract_value_type_t;

template <template <class, class...> class G, class T, class... P>
struct extract_value_type_t<G<T, P...>> {
  using type = T;
};

template <class T>
using extract_value_type = typename extract_value_type_t<T>::type;

template <class... B> struct base;
template <class T> struct base<T> {
protected:
  using type = extract_value_type<T>;
  constexpr T *self() noexcept { return static_cast<T *>(this); }
  constexpr const T *self() const noexcept {
    return static_cast<const T *>(this);
  }
};

template <class T, class A, class... Bs>
struct base<T, A, Bs...> : A::template type<base<T, Bs...>> {
private:
  using my_base = typename A::template type<base<T, Bs...>>;

protected:
  using type = extract_value_type<T>;

public:
  constexpr base() = default;
  template <class... Args>
  constexpr explicit base(Args &&... args) noexcept(
      noexcept(std::is_nothrow_constructible_v<my_base, Args...>))
      : my_base(std::forward<Args>(args)...) {}
};

} // namespace detail

namespace control {
template <class T, T V = T{}> struct tombstone {
  template <class B> struct type : B {
    static_assert(std::is_same_v<T, typename B::type>,
                  "Type & tombstone mismatch");
    constexpr type() noexcept : B(in_place, V) {}

    template <class... Args>
    constexpr explicit type(bool initial_value, Args &&... args)
        : B(std::forward<Args>(args)...) {
      assert(initial_value || B::get_ref() == V);
    }

    [[nodiscard]] constexpr bool has_value() const noexcept {
      return B::self()->get_ref() != V;
    }

  protected:
    constexpr void reset() noexcept { B::get_ref() = V; }
  };
};
struct dependent_bool {
  template <class B> struct type : B {
  protected:
    bool _has_value = false;

    constexpr type() noexcept = default;
    template <class... Args>
    constexpr explicit type(bool initial_value, Args &&... args)
        : B(std::forward<Args>(args)...), _has_value(initial_value) {}

    void reset() noexcept {
      B::destroy();
      _has_value = false;
    }

    template <class... Args>
    void build(Args &&... args) noexcept(
        noexcept(B::build(std::forward<Args>(args)...))) {
      B::build(std::forward<Args>(args)...);
      _has_value = true;
    }

  public:
    [[nodiscard]] constexpr bool has_value() const noexcept {
      return _has_value;
    }
  };
};

} // namespace control

// Utilities
struct bad_optional_access : std::exception {
  [[nodiscard]] const char *what() const override {
    return "bad optional access";
  }
};

template <class... Args> struct combine {
  template <class B> struct type;

  template <template <class...> class B, class T, class... Rs>
  struct type<B<T, Rs...>> : B<T, Args..., Rs...> {
    template <class... Args2>
    constexpr explicit type(Args2 &&... args) noexcept(
        noexcept(B<T, Args..., Rs...>(std::forward<Args2>(args)...)))
        : B<T, Args..., Rs...>(std::forward<Args2>(args)...) {}
  };
};

// Access Control

namespace access {
struct unchecked_value {
  template <class B> struct type : B {
  private:
    using T = typename B::type;

  public:
    template <class... Args>
    constexpr explicit type(Args &&... args) noexcept(
        noexcept(B(std::forward<Args>(args)...)))
        : B(std::forward<Args>(args)...) {}
    constexpr T &value() &noexcept { return B::get_ref(); }
    constexpr const T &value() const &noexcept { return B::get_ref(); }
    constexpr T &&value() &&noexcept {
      return static_cast<type &&>(*this).B::get_ref();
    }
    constexpr const T &&value() const &&noexcept {
      return static_cast<const type &&>(*this).B::get_ref();
    }
  };
};

struct unchecked_deref {
  template <class B> struct type : B {
  private:
    using T = typename B::type;

  public:
    template <class... Args>
    constexpr explicit type(Args &&... args) noexcept(
        noexcept(B(std::forward<Args>(args)...)))
        : B(std::forward<Args>(args)...) {}
    constexpr const T *operator->() const noexcept { return B::get_ptr(); }
    constexpr T *operator->() noexcept { return B::get_ptr(); }
    constexpr const T &operator*() const &noexcept { return B::get_ref(); }
    constexpr T &operator*() &noexcept { return B::get_ref(); }
    constexpr const T &&operator*() const &&noexcept {
      return static_cast<const type &&>(*this).B::get_ref();
    }
    constexpr T &&operator*() &&noexcept {
      return static_cast<type &&>(*this).B::get_ref();
    }
  };
};

struct throw_exception_deref {
  template <class B> struct type : B {
  private:
    using T = typename B::type;

  public:
    template <class... Args>
    constexpr explicit type(Args &&... args) noexcept(
        noexcept(B(std::forward<Args>(args)...)))
        : B(std::forward<Args>(args)...) {}

    constexpr const T *operator->() const {
      if (B::has_value()) {
        return B::get_ptr();
      }
      throw bad_optional_access{};
    }

    constexpr T *operator->() {
      if (B::has_value()) {
        return B::get_ptr();
      }
      throw bad_optional_access{};
    }

    constexpr const T &operator*() const & {
      if (B::has_value()) {
        return B::get_ref();
      }
      throw bad_optional_access{};
    }

    constexpr T &operator*() & {
      if (B::has_value()) {
        return B::get_ref();
      }
      throw bad_optional_access{};
    }

    constexpr const T &&operator*() const && {
      if (B::has_value()) {
        return static_cast<const type &&>(*this).B::get_ref();
      }
      throw bad_optional_access{};
    }

    constexpr T &&operator*() && {
      if (B::has_value()) {
        return static_cast<type &&>(*this).B::get_ref();
      }
      throw bad_optional_access{};
    }
  };
};

struct throw_exception_value {
  template <class B> struct type : B {
  private:
    using T = typename B::type;

  public:
    template <class... Args>
    constexpr explicit type(Args &&... args) noexcept(
        noexcept(B(std::forward<Args>(args)...)))
        : B(std::forward<Args>(args)...) {}

    constexpr T &value() & {
      if (B::has_value()) {
        return B::get_ref();
      }
      throw bad_optional_access{};
    }

    constexpr const T &value() const & {
      if (B::has_value()) {
        return B::get_ref();
      }
      throw bad_optional_access{};
    }

    constexpr T &&value() && {
      if (B::has_value()) {
        return static_cast<type &&>(*this).B::get_ref();
      }
      throw bad_optional_access{};
    }

    constexpr const T &&value() const && {
      if (B::has_value()) {
        return static_cast<const type &&>(*this).B::get_ref();
      }
      throw bad_optional_access{};
    }
  };
};

struct functional {
  template <class B> struct type : B {
    template <class... Args>
    constexpr explicit type(Args &&... args) noexcept(
        noexcept(B(std::forward<Args>(args)...)))
        : B(std::forward<Args>(args)...) {}

    template <class U, class F>
    [[nodiscard]] constexpr U with_value(F &&func, U &&default_value) const & {
      if (B::has_value()) {
        return std::forward<F>(func)(B::get_ref());
      }
      return std::forward<U>(default_value);
    }

    template <class U, class F>
    [[nodiscard]] constexpr U with_value(F &&func, U &&default_value) && {
      if (B::has_value()) {
        return std::forward<F>(func)(std::move(B::get_ref()));
      }
      return std::forward<U>(default_value);
    }

    template <class F> constexpr void with_value(F &&func) const & {
      if (B::has_value()) {
        std::forward<F>(func)(B::get_ref());
      }
    }

    template <class F> constexpr void with_value(F &&func) && {
      if (B::has_value()) {
        std::forward<F>(func)(std::move(B::get_ref()));
      }
    }
  };
};

using throw_exception = combine<throw_exception_deref, throw_exception_value>;

using unchecked = combine<unchecked_deref, unchecked_value>;

using standard = combine<unchecked_deref, throw_exception_value>;

using extended = combine<functional, standard>;

using extended_unchecked = combine<functional, unchecked>;

using extended_throw = combine<functional, throw_exception>;
} // namespace access

struct nullopt_t {
} constexpr static inline nullopt;

template <class T, class Policy>
class generalized_optional : public Policy::template type<T> {
public:
  using value_type = T;

private:
  using base = typename Policy::template type<T>;
  using policy = base;
  using storage = base;
  constexpr void _clean() noexcept(std::is_nothrow_destructible_v<value_type>) {
    if (has_value()) {
      storage::reset();
    }
  }
  constexpr void
  _copy(const T &t) noexcept(std::is_nothrow_copy_constructible_v<value_type>) {
    storage::build(t);
  }
  constexpr void
  _move(T &&t) noexcept(std::is_nothrow_move_constructible_v<value_type>) {
    storage::build(std::move(t));
  }

public:
  using policy::has_value;
  constexpr generalized_optional() noexcept = default;

  // NOLINTNEXTLINE
  constexpr generalized_optional([
      [maybe_unused]] nullopt_t empty_ctor) noexcept {}

  constexpr generalized_optional(const generalized_optional &other) noexcept(
      std::is_nothrow_copy_constructible_v<T>) {
    if (other.has_value()) {
      _copy(other.get_ref());
    }
  }
  constexpr generalized_optional(generalized_optional &&other) noexcept(
      std::is_nothrow_move_constructible_v<T>) {
    if (other.has_value()) {
      _move(std::move(other).get_ref());
    }
  }
  template <class U, class P,
            std::enable_if_t<std::is_constructible_v<T, U>> = 0>
  // NOLINTNEXTLINE
  generalized_optional(const generalized_optional<U, P> &other) noexcept(
      std::is_nothrow_constructible_v<T, U>) {
    if (other.has_value()) {
      _copy(other.get_ref());
    }
  }
  template <class U, class P>
  // NOLINTNEXTLINE
  generalized_optional(generalized_optional<U, P> &&other) noexcept(
      std::is_nothrow_constructible_v<T, U>) {
    if (other.has_value()) {
      _move(std::move(other).get_ref());
    }
  }
  template <class... Args>
  constexpr explicit generalized_optional(
      [[maybe_unused]] in_place_t in_place_ctor, Args &&... args)
      : base(true, in_place, std::forward<Args>(args)...) {}

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
  // NOLINTNEXTLINE
  constexpr generalized_optional(U &&value)
      : base(true, in_place, std::forward<U>(value)) {}

  template <
      class U = value_type,
      std::enable_if_t<
          std::conjunction_v<allow_direct_conversion<U>,
                             std::negation<std::is_convertible<U, value_type>>>,
          int> = 0>
  // NOLINTNEXTLINE
  constexpr explicit generalized_optional(U &&value)
      : base(true, in_place, std::forward<U>(value)) {}

  ~generalized_optional() {
    if (base::has_value()) {
      base::destroy();
    }
  }

  generalized_optional &operator=([
      [maybe_unused]] nullopt_t empty_assignment) noexcept {
    _clean();
  }

  constexpr generalized_optional &operator=(const generalized_optional &other) {
    if (std::addressof(other) == this) {
      return *this;
    }
    if (other.has_value()) {
      if (has_value()) {
        storage::get_ref() = other.get_ref();
      } else {
        _copy(other.get_ref());
      }
    } else {
      _clean();
    }
    return *this;
  }

  constexpr generalized_optional &
  operator=(generalized_optional &&other) noexcept(
      std::is_nothrow_move_assignable<T>::value
          &&std::is_nothrow_move_constructible<T>::value) {
    if (other.has_value()) {
      if (has_value()) {
        storage::get_ref() = std::move(other).get_ref();
      } else {
        _move(std::move(other).get_ref());
      }
    } else {
      _clean();
    }
    return *this;
  }

  template <class U = value_type,
            std::enable_if_t<allow_direct_conversion<U>::value, int> = 0>
  constexpr generalized_optional &operator=(U &&value) {
    if (has_value()) {
      storage::get_ref() = std::forward<U>(value);
    } else {
      storage::build(std::forward<U>(value));
    }
    return *this;
  }

  template <class U, class P>
  constexpr generalized_optional &
  operator=(const generalized_optional<U, P> &other) {
    if (other.has_value()) {
      if (has_value()) {
        storage::get_ref() = other.get_ref();
      } else {
        _copy(other.get_ref());
      }
    } else {
      _clean();
    }
    return *this;
  }

  template <class U, class P>
  constexpr generalized_optional &
  operator=(generalized_optional<U, P> &&other) {
    if (other.has_value()) {
      if (has_value()) {
        storage::get_ref() = std::move(other).get_ref();
      } else {
        _move(std::move(other).get_ref());
      }
    } else {
      _clean();
    }
  }

  constexpr explicit operator bool() const noexcept {
    return base::has_value();
  }

  template <class U> constexpr T value_or(U &&default_value) const & {
    if (has_value()) {
      return storage::get_ref();
    }
    return static_cast<T>(std::forward<U>(default_value));
  }
  template <class U> constexpr T value_or(U &&default_value) && {
    if (has_value()) {
      return storage::get_ref();
    }
    return static_cast<T>(std::forward<U>(default_value));
  }

  void swap(generalized_optional &other) noexcept(
      std::is_nothrow_move_constructible_v<T>
          &&std::is_nothrow_swappable_v<T>) {
    using namespace std;
    if (has_value()) {
      if (other.has_value()) {
        value_type tmp = std::move(storage::get_ref());
        storage::get_ref() = std::move(other).get_ref();
        other.get_ref() = std::move(tmp);
      } else {
        other._move(std::move(storage::get_ref()));
        _clean();
      }
    } else if (other.has_value()) {
      _move(std::move(other.get_ref()));
      other._clean();
    }
  }

  friend void
  swap(generalized_optional &lhv, generalized_optional &rhv) noexcept(
      std::is_nothrow_move_constructible_v<T>
          &&std::is_nothrow_swappable_v<T>) {
    lhv.swap(rhv);
  }

  void reset() noexcept { _clean(); }
  template <class... Args> T &emplace(Args &&... args) {
    if (has_value()) {
      storage::destroy();
    }
    storage::build(std::forward<Args>(args)...);
    return storage::get_ref();
  }

  template <class U, class... Args>
  T &emplace(std::initializer_list<U> ilist, Args &&... args) {
    if (has_value()) {
      storage::destroy();
    }
    storage::build(std::move(ilist), std::forward<Args>(args)...);
    return storage::get_ref();
  }
};

namespace detail {
template <class T, class = void> struct deduce_tombstone_value;
template <class T>
struct deduce_tombstone_value<T, std::enable_if_t<std::is_signed_v<T>>> {
  constexpr static inline T value = std::numeric_limits<T>::min();
};
template <class T>
struct deduce_tombstone_value<T, std::enable_if_t<std::is_unsigned_v<T>>> {
  constexpr static inline T value = std::numeric_limits<T>::max();
};
template <class T>
struct deduce_tombstone_value<T, std::enable_if_t<std::is_pointer_v<T>>> {
  constexpr static inline T value = nullptr;
};

} // namespace detail

template <class... Args> struct policy {
  template <class T>
  using type = detail::base<generalized_optional<T, policy<Args...>>, Args...>;
};

template <class T>
using optional = generalized_optional<
    T, policy<access::extended, control::dependent_bool, storage::aligned>>;

template <class T, T Default = detail::deduce_tombstone_value<T>::value>
using optional_tombstone = generalized_optional<
    T,
    policy<access::extended, control::tombstone<T, Default>, storage::aligned>>;

} // namespace dpsg

#endif // GUARD_GENERALIZED_OPTIONAL_HEADER