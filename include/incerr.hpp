#pragma once

#include <system_error>
#include <utility>

namespace incom {
namespace error {
template <typename T>
class err_category_impl : public std::error_category {
    // friend const std::error_code make_error_code<T>(T e);
    // friend const std::error_code make_error_condition(Unexp_plotSpecs e);

private:
    err_category_impl() = default;

    virtual const char *name() const noexcept override { return "plotSpecs Error"; }
    virtual std::string message(int ev) const override { return (incerr_msg_dispatch(T{ev})); }

public:
    static const std::error_category &getSingleton() {
        static const err_category_impl<T> instance;
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

template <typename E>
requires std::is_scoped_enum_v<E>
inline const std::error_code make_error_code(E e) {
    return std::error_code(std::to_underlying(e), error::err_category_impl<E>::getSingleton());
}

} // namespace error
} // namespace incom

#define INCERR_REGISTER(TYPE)                                                                                          \
    template <>                                                                                                        \
    struct std::is_error_code_enum<TYPE> : public true_type {}

namespace incerr = incom::error;