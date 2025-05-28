incerr is a tiny lightweight library that extends and wraps (parts of) the functionality from [<system_error>](https://en.cppreference.com/w/cpp/error.html#System_error) in (in the author's opinion) saner way for the year 2025 so that one can use [std::error_code](https://en.cppreference.com/w/cpp/error/error_code.html) type or this library's [incerr_code](<https://github.com/InCom-0/incerr/blob/main/include/incerr.hpp#L87>) type that extends it inside [std::expected](https://en.cppreference.com/w/cpp/utility/expected/) as the 'unexpected' type (or elsewhere) in a more straightforward manner.

In short the library provides a very ergonomic way to customize/extend std::error_code.

## Reasons for the library's existence ##

While (almost) all of the things the library does can be done just with <system_error>, the practice of doing so is perhaps less ergonomic and straightforward than one would ideally like for the (arguably) rather basic programming concepts such as error codes and error messages. This library makes it as simple as possible.

* Only requires 1 additional line of code per error enum class type for setting things up
* 
* The interface of the library as well as the internals are properly contrained using C++20 concepts making it harder to misuse
* Forbids the user from using unscoped enums for errors (and from doing some other undesirable things)
* Provides sensible default for getting 'message' (enum value enumerator name) through compile time reflection through [magic_enum] (<https://github.com/Neargye/magic_enum>) header-only library
* User can optionally override the default 'message' by defining 'std::string_view incerr_msg_dispatch (ENUM_T)' in a way so that it gets picked up by [ADL](https://en.cppreference.com/w/cpp/language/adl). The library will then automatically use it instead of the default implementation
* Optionally provides a way to supply additional custom 'message' at the point where incerr_code is emitted which can be used to provide additional information 'on top of' the regular 'message' that's based solely on the enumerator.
* If opted out from the above emmits vanilla std::error_code
