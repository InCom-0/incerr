#pragma once

#include <source_location>
#include <string_view>
#include <system_error>
#include <utility>


namespace incom {
namespace error {
namespace detail {

template <typename T>
class incerr_cat : public std::error_category {
private:
    incerr_cat() = default;

    template <typename TT>
    constexpr auto __typeToString() {
        auto EmbeddingSignature = std::string{std::source_location::current().function_name()};
        auto firstPos           = EmbeddingSignature.rfind("::") + 2;
        return EmbeddingSignature.substr(firstPos, EmbeddingSignature.size() - firstPos - 1);
    }

    virtual const char *name() const noexcept override { return __typeToString<T>(); }
    virtual std::string message(int ev) const override { return (incerr_msg_dispatch(T{ev})); }

public:
    // Meyers' Singleton technique to guarantee only 1 instance is ever created
    static const std::error_category &getSingleton() {
        static const incerr_cat<T> instance;
        return instance;
    }
};
// IF THIS GETS INSTANTIATED ONE GETS A COMPILE TIME ERROR ON PURPOSE
// DEFAULT ie. unhandled message dispatch for a particular enum
template <typename E>
requires std::is_scoped_enum_v<E>
inline const std::string incerr_msg_dispatch(E &&e) {
    static_assert(false,
                  "Unhandled message dispatch for some error type (ie scoped enum type used for errors). Please "
                  "provide a free function with the signature 'inline const std::string err_msg_dispatch(E &&e)' where "
                  "'E' is the scoped enum type in the same namespace as the scoped enum definition");
    std::unreachable();
}
} // namespace detail

class incerr_code : public std::error_code {
public:
    const std::string localMsg;

    template <typename E>
    requires std::is_scoped_enum_v<E>
    static inline const incerr_code make_incerr_code(E e) {
        return incerr_code(std::to_underlying(e), error::detail::incerr_cat<E>::getSingleton());
    }

    template <typename E>
    requires std::is_scoped_enum_v<E>
    static inline const incerr_code make_incerr_code(E e, std::string_view sv) {
        return incerr_code(std::to_underlying(e), error::detail::incerr_cat<E>::getSingleton(), sv);
    }

private:
    incerr_code(int ec, const std::error_category &cat) noexcept : std::error_code(ec, cat), localMsg() {}
    incerr_code(int ec, const std::error_category &cat, std::string_view localMsg) noexcept
        : std::error_code(ec, cat), localMsg(localMsg) {}
};

// template <typename E>
// requires std::is_scoped_enum_v<E>
// inline const std::error_code make_error_code(E e) {
//     return std::error_code(std::to_underlying(e), error::detail::incerr_cat<E>::getSingleton());
// }
// template <typename E>
// requires std::is_scoped_enum_v<E>
// inline const std::error_code make_error_code(E e, std::string_view sv) {
//     return std::error_code(std::to_underlying(e), error::detail::incerr_cat<E>::getSingleton());
// }


} // namespace error
} // namespace incom

#define INCERR_REGISTER(TYPE)                                                                                          \
    template <>                                                                                                        \
    struct std::is_error_code_enum<TYPE> : public true_type {}

namespace incerr = incom::error;