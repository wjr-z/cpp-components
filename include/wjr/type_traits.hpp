#ifndef WJR_TYPE_TRAITS_HPP__
#define WJR_TYPE_TRAITS_HPP__

#include <functional>
#include <limits>
#include <optional>
#include <type_traits>

#include <wjr/preprocessor.hpp>

namespace wjr {

template <typename... Args>
struct multi_conditional;

template <typename... Args>
using multi_conditional_t = typename multi_conditional<Args...>::type;

template <bool f, typename T, typename... Args>
struct multi_conditional_impl {
    using type = T;
};

template <typename T, typename... Args>
struct multi_conditional_impl<false, T, Args...> {
    using type = multi_conditional_t<Args...>;
};

template <typename F, typename T, typename U>
struct multi_conditional<F, T, U> {
    using type = std::conditional_t<F::value, T, U>;
};

template <typename F, typename T, typename U, typename V, typename... Args>
struct multi_conditional<F, T, U, V, Args...> {
    using type = typename multi_conditional_impl<F::value, T, U, V, Args...>::type;
};

template <typename T>
using remove_ref_t = std::remove_reference_t<T>;

template <typename T>
using remove_cref_t = std::remove_const_t<remove_ref_t<T>>;

template <typename T>
using remove_cvref_t = std::remove_cv_t<remove_ref_t<T>>;

template <typename iter, typename = void>
struct is_contiguous_iterator_impl : std::is_pointer<iter> {};

template <typename iter>
struct is_contiguous_iterator_impl<iter, typename iter::is_contiguous_iterator>
    : std::true_type {};

#if defined(WJR_CPP_20)
template <typename iter>
struct is_contiguous_iterator
    : std::bool_constant<std::contiguous_iterator<iter> ||
                         is_contiguous_iterator_impl<iter>::value> {};
#else
template <typename iter>
struct is_contiguous_iterator : is_contiguous_iterator_impl<iter> {};
#endif

template <typename iter>
struct is_contiguous_iterator<std::reverse_iterator<iter>>
    : is_contiguous_iterator<iter> {};

template <size_t n>
struct __uint_selector {};

template <>
struct __uint_selector<8> {
    using type = std::uint8_t;
};

template <>
struct __uint_selector<16> {
    using type = std::uint16_t;
};

template <>
struct __uint_selector<32> {
    using type = std::uint32_t;
};

template <>
struct __uint_selector<64> {
    using type = std::uint64_t;
};

template <size_t n>
struct __int_selector {
    using type = std::make_signed_t<typename __uint_selector<n>::type>;
};

#if WJR_HAS_FEATURE(INT128)
template <>
struct __uint_selector<128> {
    using type = __uint128_t;
};

template <>
struct __int_selector<128> {
    using type = __int128_t;
};
#endif

template <size_t n>
using uint_t = typename __uint_selector<n>::type;

template <size_t n>
using int_t = typename __int_selector<n>::type;

using int8_t = int_t<8>;
using int16_t = int_t<16>;
using int32_t = int_t<32>;
using int64_t = int_t<64>;
#if WJR_HAS_FEATURE(INT128)
using int128_t = int_t<128>;
#endif 

using uint8_t = uint_t<8>;
using uint16_t = uint_t<16>;
using uint32_t = uint_t<32>;
using uint64_t = uint_t<64>;
#if WJR_HAS_FEATURE(INT128)
using uint128_t = uint_t<128>;
#endif 

using intptr_t = int_t<sizeof(void *) * 8>;
using uintptr_t = uint_t<sizeof(void *) * 8>;

using std_int8_t = std::int8_t;
using std_int16_t = std::int16_t;
using std_int32_t = std::int32_t;
using std_int64_t = std::int64_t;

using std_uint8_t = std::uint8_t;
using std_uint16_t = std::uint16_t;
using std_uint32_t = std::uint32_t;
using std_uint64_t = std::uint64_t;

using std_intptr_t = std::intptr_t;
using std_uintptr_t = std::uintptr_t;

template <size_t n, bool __s>
using usint_t = std::conditional_t<__s, int_t<n>, uint_t<n>>;

template <typename T>
struct is_unsigned_integral : std::conjunction<std::is_integral<T>, std::is_unsigned<T>> {
};

template <typename T>
struct is_signed_integral : std::conjunction<std::is_integral<T>, std::is_signed<T>> {};

class __is_little_endian_helper {
    constexpr static std::uint32_t u4 = 1;
    constexpr static std::uint8_t u1 = static_cast<const std::uint8_t &>(u4);

public:
    constexpr static bool value = u1 != 0;
};

// constexpr endian
enum class endian {
    little = 0,
    big = 1,
    native = __is_little_endian_helper::value ? little : big
};

// type identity
template <typename T>
struct type_identity {
    using type = T;
};

template <typename T>
using type_identity_t = typename type_identity<T>::type;

template <typename T>
struct is_constant : std::false_type {};

template <typename T, T V>
struct is_constant<std::integral_constant<T, V>> : std::true_type {};

template <typename T>
inline constexpr bool is_constant_v = is_constant<T>::value;

template <typename T>
WJR_INTRINSIC_CONSTEXPR std::enable_if_t<!is_constant_v<T>, const T &>
get_constant(const T &p) {
    return p;
}

template <typename T>
WJR_INTRINSIC_CONSTEXPR std::enable_if_t<is_constant_v<T>, typename T::value_type>
get_constant(const T &p) {
    return static_cast<typename T::value_type>(p);
}

WJR_INTRINSIC_CONSTEXPR bool is_constant_evaluated() noexcept {
    return WJR_IS_CONSTANT_EVALUATED();
}

template <typename T>
WJR_INTRINSIC_CONSTEXPR bool is_constant_p(const T &p) {
    if constexpr (is_constant_v<T>) {
        return true;
    } else {
        return WJR_BUILTIN_CONSTANT_P(p);
    }
}

template <typename T>
using null_optional_t = std::optional<T>;

template <typename T>
inline constexpr null_optional_t<T> null_optional = null_optional_t<T>(std::nullopt);

template <typename T>
struct may_null {
    using type = T;
    static constexpr bool value = false;
};

template <typename T>
struct may_null<null_optional_t<T>> {
    using type = T;
    static constexpr bool value = true;
};

template <typename T>
using may_null_t = typename may_null<T>::type;

template <typename T>
inline constexpr bool may_null_v = may_null<T>::value;

} // namespace wjr

#endif // ! WJR_TYPE_TRAITS_HPP__