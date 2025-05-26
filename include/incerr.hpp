#pragma once

#include <source_location>
#include <string_view>
#include <system_error>
#include <type_traits>
#include <utility>


namespace incom {
namespace error {
namespace detail {
template <typename T>
concept enumIsRegistered = requires(T t) {
    { make_error_code(t) } -> std::same_as<std::error_code>;
    { make_error_condition(t) } -> std::same_as<std::error_condition>;
};

template <typename T>
class incerr_cat : public std::error_category {
private:
    incerr_cat() = default;

    template <typename TT>
    constexpr auto __typeToString() const {
        auto EmbeddingSignature = std::string{std::source_location::current().function_name()};
        auto firstPos           = EmbeddingSignature.rfind("::") + 2;
        return EmbeddingSignature.substr(firstPos, EmbeddingSignature.size() - firstPos - 1);
    }

    virtual const char *name() const noexcept override {
        static const std::string s = __typeToString<T>();
        return s.c_str();
    }
    virtual std::string message(int ev) const override { return std::string(incerr_msg_dispatch(T{ev})); }

public:
    // Meyers' Singleton technique to guarantee only 1 instance is ever created
    static const std::error_category &getSingleton() {
        static const incerr_cat<T> instance;
        return instance;
    }
};
} // namespace detail

class incerr_code : public std::error_code {
public:
    const std::string localMsg;

    template <typename E>
    requires std::is_scoped_enum_v<E> && std::is_error_code_enum<E>::value && detail::enumIsRegistered<E>
    static inline const incerr_code make(E e) {
        return incerr_code(std::to_underlying(e), error::detail::incerr_cat<E>::getSingleton());
    }

    template <typename E, typename S>
    requires std::is_scoped_enum_v<E> && std::is_error_code_enum<E>::value && detail::enumIsRegistered<E> &&
             std::is_convertible_v<S, std::string_view>
    static inline const incerr_code make(E e, S const sv) {
        return incerr_code(std::to_underlying(e), error::detail::incerr_cat<E>::getSingleton(), sv);
    }

    template <typename E>
    requires std::is_scoped_enum_v<E> && std::is_error_code_enum<E>::value && detail::enumIsRegistered<E>
    static inline const std::error_code make_std_ec(E e) {
        return std::error_code(std::to_underlying(e), error::detail::incerr_cat<E>::getSingleton());
    }

private:
    incerr_code() = delete;
    template <typename E>
    requires std::is_scoped_enum_v<E> && std::is_error_code_enum<E>::value && detail::enumIsRegistered<E>
    incerr_code(E __e) {
        *this = make(__e);
    }

    incerr_code(int ec, const std::error_category &cat) noexcept : std::error_code(ec, cat), localMsg() {}
    incerr_code(int ec, const std::error_category &cat, std::string_view const localMsg) noexcept
        : std::error_code(ec, cat), localMsg(localMsg) {}
};
} // namespace error
} // namespace incom


namespace incom::error {}

// The user MUST 'register' the enum types used in the library. Constraints violation on incerr_code static methods will
// ensue during compilation otherwise.
// Either do this by using this macro (which can be 'undefed' once not needed ...
// typically at the end of the file with the enums definitions). Or just do the thing the macro does manually
#define INCERR_REGISTER(TYPE, NAMESPACE_FULL)                                                                          \
    template <>                                                                                                        \
    struct std::is_error_code_enum<TYPE> : public true_type {};                                                        \
                                                                                                                       \
    namespace NAMESPACE_FULL {                                                                                         \
    inline std::error_code make_error_code(TYPE e) {                                                                   \
        return std::error_code(static_cast<int>(e), incom::error::detail::incerr_cat<TYPE>::getSingleton());           \
    }                                                                                                                  \
                                                                                                                       \
    inline std::error_condition make_error_condition(TYPE e) {                                                         \
        return std::error_condition(static_cast<int>(e), incom::error::detail::incerr_cat<TYPE>::getSingleton());      \
    }                                                                                                                  \
    }

namespace incerr = incom::error;