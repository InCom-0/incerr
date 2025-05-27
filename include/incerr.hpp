#pragma once

#include <memory>
#include <string>
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

template <typename ENUM_T, int VAL>
struct enum_hasNoZeroValue {
    static const bool value = (not magic_enum::enum_cast<ENUM_T>(VAL).has_value());
};

// The scoped enums used as template arguments in incerr library MUST NOT use the constant '0' as one of the
// explicitly named contants (so called 'enumerators')
// The fix would typically be to define the first enumerator as having constant value '1'
// This small limitation ensures better compatibility with the standard library
template <typename ENUM_T>
concept enum_hasNoZeroValue_v = enum_hasNoZeroValue<ENUM_T, 0>::value;

template <typename E>
requires std::is_scoped_enum_v<E> && enum_hasNoZeroValue_v<E>
class incerr_cat : public std::error_category {
private:
    incerr_cat() = default;

    virtual const char *name() const noexcept override {
        static const std::string s{__internal_name_dispatch()};
        return s.c_str();
    }
    virtual std::string message(int ev) const override { return __internal_msg_dispatch(ev); }

    // template <typename EE = E>
    // requires enum_hasNameDispatch<EE>
    // std::string __internal_name_dispatch() const {
    //     static constexpr const EE instance{};
    //     return std::string{incerr_name_dispatch(instance)};
    // }

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
    static const incerr_cat<E> &getSingleton() {
        static const incerr_cat<E> instance;
        return instance;
    }
};
} // namespace detail

class incerr_code : public std::error_code {
private:
    std::unique_ptr<std::string> customMessage;

public:
    // MAIN INTERFACE METHODS
    template <typename E>
    requires std::is_scoped_enum_v<E> && detail::enum_hasNoZeroValue_v<E> && std::is_error_code_enum<E>::value &&
             detail::enum_isRegistered<E>
    static incerr_code make(E e) {
        return incerr_code(std::to_underlying(e), error::detail::incerr_cat<E>::getSingleton());
    }

    // One can provide 'custom message' which is stored in the static storage of the class
    // This way the message does not get passed around and the 'incerr_code' instances are kept tiny
    template <typename E, typename S>
    requires std::is_scoped_enum_v<E> && detail::enum_hasNoZeroValue_v<E> && std::is_error_code_enum<E>::value &&
             detail::enum_isRegistered<E> && std::is_convertible_v<S, std::string_view>
    static incerr_code make(E e, S const &&customMessage) {
        return incerr_code(std::to_underlying(e), error::detail::incerr_cat<E>::getSingleton(),
                           std::forward<decltype(customMessage)>(customMessage));
    }

    template <typename E>
    requires std::is_scoped_enum_v<E> && detail::enum_hasNoZeroValue_v<E> && std::is_error_code_enum<E>::value &&
             detail::enum_isRegistered<E>
    static inline const std::error_code make_std_ec(E e) {
        return std::error_code(std::to_underlying(e), error::detail::incerr_cat<E>::getSingleton());
    }

    const std::string_view get_customMessage() const { return std::string_view{*customMessage}; }

    // CONSTRUCTION
    incerr_code()                  = delete;
    incerr_code(incerr_code &&src) = default; // move constructor

    // copy constructor
    incerr_code(const incerr_code &src) : customMessage(std::make_unique<std::string>(*src.customMessage)) {};
    // copy assignment
    incerr_code &operator=(const incerr_code &src) { return *this = incerr_code(src); }

    // move assignment
    incerr_code &operator=(incerr_code &&src) noexcept {
        std::swap(customMessage, src.customMessage);
        return *this;
    }

    // DESTRUCTION
    ~incerr_code() = default;

private:
    template <typename E>
    requires std::is_scoped_enum_v<E> && detail::enum_hasNoZeroValue_v<E> && std::is_error_code_enum<E>::value &&
             detail::enum_isRegistered<E>
    incerr_code(E __e) {
        *this = make(__e);
    }

    incerr_code(int ec, const std::error_category &cat) noexcept
        : std::error_code(ec, cat), customMessage{std::make_unique<std::string>("")} {}

    template <typename SV>
    requires std::is_convertible_v<SV, std::string_view>
    incerr_code(int ec, const std::error_category &cat, SV const &&sv) noexcept
        : std::error_code(ec, cat), customMessage{std::make_unique<std::string>(sv)} {}
};
} // namespace error
} // namespace incom

// The user MUST 'register' the enum types used in the library. Constraints violation on incerr_code static methods will
// ensue during compilation otherwise.
// Either do this by using this macro (which can be 'undefed' once not needed ...
// typically at the end of the file with the enums definitions). Or just do the thing the macro does manually
#define INCERR_REGISTER(TYPE_FULLY_QUALIFIED, NAMESPACE_FULLY_QUALIFIED)                                               \
    template <>                                                                                                        \
    struct std::is_error_code_enum<TYPE_FULLY_QUALIFIED> : public true_type {};                                        \
                                                                                                                       \
    namespace NAMESPACE_FULLY_QUALIFIED {                                                                              \
    inline std::error_code make_error_code(TYPE_FULLY_QUALIFIED e) {                                                   \
        return std::error_code(static_cast<int>(e),                                                                    \
                               incom::error::detail::incerr_cat<TYPE_FULLY_QUALIFIED>::getSingleton());                \
    }                                                                                                                  \
                                                                                                                       \
    inline std::error_condition make_error_condition(TYPE_FULLY_QUALIFIED e) {                                         \
        return std::error_condition(static_cast<int>(e),                                                               \
                                    incom::error::detail::incerr_cat<TYPE_FULLY_QUALIFIED>::getSingleton());           \
    }                                                                                                                  \
    }

#ifndef INCOM_INCERR_NAMESPACE_ALIAS
#define INCOM_INCERR_NAMESPACE_ALIAS

namespace incerr = incom::error;

#endif