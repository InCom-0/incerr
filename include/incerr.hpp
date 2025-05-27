#pragma once

#include <string_view>
#include <system_error>
#include <type_traits>
#include <utility>

#include <magic_enum/magic_enum.hpp>


namespace incom {
namespace error {
namespace detail {
template <typename T>
concept enum_isRegistered = requires(T t) {
    { make_error_code(t) } -> std::same_as<std::error_code>;
    { make_error_condition(t) } -> std::same_as<std::error_condition>;
};
template <typename T>
concept enum_hasMsgDispatch = requires(T t) {
    { incerr_msg_dispatch(std::move(t)) } -> std::same_as<std::string_view>;
};
template <typename T>
concept enum_hasNameDispatch = requires(T t) {
    { incerr_name_dispatch(std::move(t)) } -> std::same_as<std::string_view>;
};

template <typename E>
requires std::is_scoped_enum_v<E>
class incerr_cat : public std::error_category {
private:
    incerr_cat() = default;

    virtual const char *name() const noexcept override {
        static const std::string s{__internal_name_dispatch()};
        return s.c_str();
    }
    virtual std::string message(int ev) const override { return __internal_msg_dispatch(ev); }

    template <typename EE = E>
    requires enum_hasNameDispatch<EE>
    std::string __internal_name_dispatch() const {
        static constexpr const EE instance{};
        return incerr_name_dispatch(instance);
    }

    template <typename EE = E>
    std::string __internal_name_dispatch() const {
        return std::string{magic_enum::enum_type_name<EE>()};
    }

    template <typename EE = E>
    requires enum_hasMsgDispatch<EE>
    std::string __internal_msg_dispatch(const int ev) const {
        return std::string{incerr_msg_dispatch(EE{ev})};
    }

    template <typename EE = E>
    std::string __internal_msg_dispatch(const int ev) const {
        return std::string{magic_enum::enum_name<EE>(magic_enum::enum_cast<EE>(ev).value())};
    }

public:
    // Meyers' Singleton technique to guarantee only 1 instance is ever created
    static const std::error_category &getSingleton() {
        static const incerr_cat<E> instance;
        return instance;
    }
};

} // namespace detail

class incerr_code : public std::error_code {
public:
    const std::string localMsg;

    template <typename E>
    requires std::is_scoped_enum_v<E> && std::is_error_code_enum<E>::value && detail::enum_isRegistered<E>
    static inline const incerr_code make(E e) {
        return incerr_code(std::to_underlying(e), error::detail::incerr_cat<E>::getSingleton());
    }

    template <typename E, typename S>
    requires std::is_scoped_enum_v<E> && std::is_error_code_enum<E>::value && detail::enum_isRegistered<E> &&
             std::is_convertible_v<S, std::string_view>
    static inline const incerr_code make(E e, S const sv) {
        return incerr_code(std::to_underlying(e), error::detail::incerr_cat<E>::getSingleton(), sv);
    }

    template <typename E>
    requires std::is_scoped_enum_v<E> && std::is_error_code_enum<E>::value && detail::enum_isRegistered<E>
    static inline const std::error_code make_std_ec(E e) {
        return std::error_code(std::to_underlying(e), error::detail::incerr_cat<E>::getSingleton());
    }

private:
    incerr_code() = delete;
    template <typename E>
    requires std::is_scoped_enum_v<E> && std::is_error_code_enum<E>::value && detail::enum_isRegistered<E>
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