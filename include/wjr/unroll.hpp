#ifndef WJR_UNROLL_HPP__
#define WJR_UNROLL_HPP__

#include <wjr/type_traits.hpp>

namespace wjr {

template <size_t unroll, typename Fn, typename Ret, size_t idx>
WJR_INTRINSIC_CONSTEXPR Ret unroll_call_impl(Fn &fn) {
#define WJR_REGISTER_UNROLL_CALL_IMPL(idxs)                                              \
    WJR_PP_TRANSFORM_PUT(idxs, WJR_REGISTER_UNROLL_CALL_IMPL_CALLER)
#define WJR_REGISTER_UNROLL_CALL_IMPL_CALLER(offset)                                     \
    do {                                                                                 \
        auto ret = fn(std::integral_constant<size_t, idx + offset>{});                   \
                                                                                         \
        if (ret.has_value()) {                                                           \
            return ret.value();                                                          \
        }                                                                                \
                                                                                         \
    } while (0);

    if constexpr (idx == unroll) {
        return std::nullopt;
    } else if constexpr (unroll - idx >= 8) {
        WJR_REGISTER_UNROLL_CALL_IMPL((0, 1, 2, 3, 4, 5, 6, 7));
        return unroll_call_impl<unroll, Fn, Ret, idx + 8>(fn);
    } else if constexpr (unroll - idx >= 4) {
        WJR_REGISTER_UNROLL_CALL_IMPL((0, 1, 2, 3));
        return unroll_call_impl<unroll, Fn, Ret, idx + 4>(fn);
    } else {
        WJR_REGISTER_UNROLL_CALL_IMPL_CALLER(0);
        return unroll_call_impl<unroll, Fn, Ret, idx + 1>(fn);
    }

#undef WJR_REGISTER_UNROLL_CALL_IMPL_CALLER
#undef WJR_REGISTER_UNROLL_CALL_IMPL
}

template <size_t unroll, typename Fn,
          typename Ret = std::invoke_result_t<Fn, std::integral_constant<size_t, 0>>>
WJR_INTRINSIC_CONSTEXPR Ret unroll_call(Fn fn) {
    return unroll_call_impl<unroll, Fn, Ret, 0>(fn);
}

} // namespace wjr

#endif // WJR_UNROLL_HPP__