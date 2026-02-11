no readme yet, checkout [Example](https://github.com/Maslowian/argless/blob/master/example/src/main.cpp) for usage

[![image](https://github.com/Maslowian/argless/actions/workflows/build.yml/badge.svg?branch=master)](https://github.com/Maslowian/argless/actions/workflows/build.yml)
[![image](https://github.com/Maslowian/argless/actions/workflows/tests.yml/badge.svg?branch=master)](https://github.com/Maslowian/argless/actions/workflows/tests.yml)

supported types:
- bool
- int, unsigned int, short, unsigned short, signed char, unsigned char, long, unsigned long, long long, unsigned long long
- float, double, long double
- char, wchar_t, char8_t, char16_t, char32_t, char[4], wchar_t[4 / sizeof(wchar_t)], char8_t[4], char16_t[2], char32_t[1]
- const char_t*, std::basic_string_view, std::basic_string
- enum, enum class
- std::array
- std::vector, std::list, std::deque, std::stack, std::queue,
- std::optional
- std::variant
- std::tuple
- std::filesystem::path

special types:
- argless::accumulate
- argless::required
- argless::default_value

extra types:
- argless::rename
- argless::force
- argless::transform
- argless::validate
- argless::option

TODO:
- [ ] pretty | and , names
- [ ] unicode
- [ ] helper
- [ ] pages
- [ ] groups
