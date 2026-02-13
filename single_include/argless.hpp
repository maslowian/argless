/* argless - https://github.com/maslowian/argless */

/*
MIT License

Copyright (c) 2024-2026 Piotr Ginalski

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#pragma once

#define ARGLESS_VERSION_MAJOR 2
#define ARGLESS_VERSION_MINOR 0
#define ARGLESS_VERSION_PATCH 0

#define _ARGLESS_BEGIN namespace argless {
#define _ARGLESS_END }
#define _ARGLESS ::argless::

#define _ARGLESS_CORE_BEGIN _ARGLESS_BEGIN namespace core {
#define _ARGLESS_CORE_END _ARGLESS_END }
#define _ARGLESS_CORE _ARGLESS core::

#ifndef ARGLESS_ASSERT
#include <cassert>
#define ARGLESS_ASSERT(expr, msg) do { assert((expr) && msg); } while (0)
#endif


#include <type_traits>
#include <optional>
#include <utility>

_ARGLESS_CORE_BEGIN

template <typename _char_t, std::size_t n>
struct str
{
	using char_t = _char_t;

	inline constexpr str() {}

	inline constexpr str(const char_t(&buffer)[n])
	{
		for (std::size_t i = 0; i < n; ++i) m_buffer[i] = buffer[i];
	}

	inline constexpr operator const char_t*() const { return data(); }

	inline constexpr const char_t* data() const { return m_buffer; }
	inline constexpr std::size_t size() const { return n; }

	template <std::size_t other_n>
	inline constexpr str<char_t, n + other_n - 1> operator+(const str<char_t, other_n>& other) const
	{
		str<char_t, n + other_n - 1> result;
		for (std::size_t i = 0; i < n-1; ++i) result.m_buffer[i] = m_buffer[i];
		for (std::size_t i = 0; i < other_n; ++i) result.m_buffer[n-1 + i] = other.m_buffer[i];
		return result;
	}

	char_t m_buffer[n] = { 0 };
};


template <typename to_char_t, typename from_char_t>
constexpr inline std::size_t char_ratio = (sizeof(from_char_t) / sizeof(to_char_t) > 1 ? sizeof(from_char_t) / sizeof(to_char_t) : 1);

template <typename to_char_t, typename from_char_t>
inline constexpr str<to_char_t, char_ratio<to_char_t, from_char_t> + 1> get_charu(const from_char_t*& it)
{
	// TODO: unicode
	str<to_char_t, char_ratio<to_char_t, from_char_t> + 1> str;
	str.m_buffer[0] = static_cast<to_char_t>(*it);
	str.m_buffer[1] = static_cast<to_char_t>('\0');
	++it;
	return str;
}

template <typename char_t>
inline constexpr char_t to_lower(char_t c)
{
    char ch = static_cast<char>(c);
    if (ch >= 'A' && ch <= 'Z')
        return static_cast<char_t>(ch - 'A' + 'a');
    return c;
}

template <typename char_t>
inline constexpr std::size_t slen(const char_t* str)
{
	std::size_t c = 0;
	while (*str) ++c, ++str;
	return c;
}

template <typename l_char_t, typename r_char_t>
inline constexpr bool seq_nocase(const l_char_t* lstr, const r_char_t* rstr)
{
	while (true)
	{
		if (!*lstr || !*rstr)
			return !*lstr && !*rstr;

		if (to_lower(get_charu<char32_t>(lstr).m_buffer[0]) != to_lower(get_charu<char32_t>(rstr).m_buffer[0]))
			return false;
	}
}

template <typename l_char_t, typename r_char_t>
inline constexpr bool seq(const l_char_t* lstr, const r_char_t* rstr)
{
	while (true)
	{
		if (!*lstr || !*rstr)
			return !*lstr && !*rstr;

		if (get_charu<char32_t>(lstr).m_buffer[0] != get_charu<char32_t>(rstr).m_buffer[0])
			return false;
	}
}

template <typename t, typename char_t>
inline constexpr std::optional<t> stot(const char_t* str)
{
	t result = 0;
	bool neg = false;
	bool at_least_one = false;
	bool doted = false;
	t dot_scale = static_cast<t>(.1);

	if constexpr (std::is_signed_v<t>)
	{
		const char_t* str_ = str;
		auto c = get_charu<char32_t>(str_).m_buffer[0];
		if (c == static_cast<char32_t>('-'))
		{
			neg = true;
			str = str_;
		}
	}
	while (*str)
	{
		auto c = get_charu<char32_t>(str).m_buffer[0];

		if constexpr (std::is_floating_point_v<t>)
			if (c == static_cast<char32_t>('.'))
			{
				if (doted)
					return std::nullopt;
				doted = true;
				continue;
			}

		at_least_one = true;
		int d = -1;

		constexpr char_t digits[] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', };

		for (std::size_t i = 0; i < 10; ++i)
			if (c == static_cast<char32_t>(digits[i]))
			{
				d = i;
				break;
			}

		if (d == -1)
			return std::nullopt;

		if constexpr (std::is_floating_point_v<t>)
			if (doted)
			{
				result += dot_scale * d;
				dot_scale /= 10;
				continue;
			}

		auto old_result = result;

		result *= 10;
		result += d;

		if (result < old_result)
			return std::nullopt;
	}

	if (!at_least_one)
		return std::nullopt;

	if (neg)
		return -result;
	else
		return result;
}


template <typename to_char_t, auto from_str>
inline consteval auto str_cast()
{
	if constexpr (std::is_same_v<to_char_t, typename decltype(from_str)::char_t>)
		return from_str;
	return []<auto result>(){
		str<to_char_t, result.second + 1> to_str;
		auto fstr = result.first;
		auto i = 0;
		for (; fstr.m_buffer[i]; ++i)
			to_str.m_buffer[i] = fstr.m_buffer[i];
		to_str.m_buffer[i] = static_cast<to_char_t>('\0');
		return to_str;
	}.template operator()<[](){
		str<to_char_t, (from_str.size() - 1) * char_ratio<to_char_t, typename decltype(from_str)::char_t> + 1> to_str;
		std::size_t c = 0;
		for (auto it = from_str.data(); *it;)
		{
			auto chars = get_charu<to_char_t>(it);
			for (auto cit = chars.data(); *cit; ++cit, ++c)
				to_str.m_buffer[c] = *cit;
		}
		to_str.m_buffer[c] = static_cast<to_char_t>('\0');
		return std::make_pair(to_str, c);
	}()>();
}

template <typename to_char_t, str from_str>
inline consteval auto str_from() 
{
	return str_cast<to_char_t, from_str>();
}

template <str str_>
struct _static_str_impl
{
	static constexpr inline auto value = str_;
};

template <str str_>
constexpr inline auto static_str = _static_str_impl<str_>::value;

_ARGLESS_CORE_END

/* tetter - https://github.com/maslowian/tetter */

/* 
MIT License

Copyright (c) 2025-2026 Piotr Ginalski

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#ifndef _tetter_hpp
#define _tetter_hpp

#define tetter_version_major 1
#define tetter_version_minor 0
#define tetter_version_patch 0

#ifndef _tetter_decltype
#ifdef __cpp_decltype_auto
#define _tetter__cpp_decltype_auto __cpp_decltype_auto
#else
#define _tetter__cpp_decltype_auto __cplusplus
#endif
#if _tetter__cpp_decltype_auto >= 201304L
#define _tetter_decltype(...) decltype(auto) 
#else
#define _tetter_decltype(...) decltype(__VA_ARGS__)
#endif
#endif

#ifndef _tetter_variable_templates
#ifdef __cpp_variable_templates
#define _tetter__cpp_variable_templates __cpp_variable_templates
#else
#define _tetter__cpp_variable_templates __cplusplus
#endif
#if _tetter__cpp_variable_templates >= 201304L
#define _tetter_variable_templates true
#else
#define _tetter_variable_templates false
#endif
#endif

#ifndef _tetter_generic_lambdas
#ifdef __cpp_generic_lambdas
#define _tetter__cpp_generic_lambdas __cpp_generic_lambdas
#else
#define _tetter__cpp_generic_lambdas __cplusplus
#endif
#if _tetter__cpp_generic_lambdas >= 201707L
#define _tetter_generic_lambdas true
#else
#define _tetter_generic_lambdas false
#endif
#endif

#ifndef _tetter_concepts
#ifdef __cpp_concepts
#define _tetter__cpp_concepts __cpp_concepts
#else
#define _tetter__cpp_concepts __cplusplus
#endif
#if _tetter__cpp_concepts >= 201907L
#define _tetter_concepts true
#else
#define _tetter_concepts false
#endif
#endif

#ifndef _tetter_unevaluated_lambda
#if __cplusplus >= 202002L
#define _tetter_unevaluated_lambda true
#else
#define _tetter_unevaluated_lambda false 
#endif
#endif

#ifndef _tetter_static_call_operator
#ifdef __cpp_static_call_operator
#define _tetter__cpp_static_call_operator __cpp_static_call_operator
#else
#define _tetter__cpp_static_call_operator __cplusplus
#endif
#if _tetter__cpp_static_call_operator >= 202207L
#define _tetter_static_call_operator true
#else
#define _tetter_static_call_operator false
#endif
#endif

#ifndef _tetter_force_inline
#if defined(_MSC_VER)
#define _tetter_force_inline __forceinline
#elif defined(__clang__) || defined(__GNUC__)
#define _tetter_force_inline inline __attribute__((always_inline))
#else
#define _tetter_force_inline inline
#endif
#endif

namespace _tetter {

template <bool v, typename tt, typename ft>
struct _conditional;

template <typename tt, typename ft>
struct _conditional<false, tt, ft>
{
	using type = ft;
};

template <typename tt, typename ft>
struct _conditional<true, tt, ft>
{
	using type = tt;
};

template <typename ft, typename st>
struct _is_same
{
	static constexpr bool value = false;
};

template <typename t>
struct _is_same<t, t>
{
	static constexpr bool value = true;
};

using _size_t = decltype(sizeof(0)); 

template <typename t>
t&& _declval() noexcept { static_assert(false); }

template <typename... ts>
struct _tetter
{
	template <template <typename...> typename t>
	using cast = t<ts...>;
};

template <typename lt, typename rt>
struct _join_tetter;

template <typename... lts, typename... rts>
struct _join_tetter<_tetter<lts...>, _tetter<rts...>>
{
	using type = _tetter<lts..., rts...>;
};

template<typename... Ts>
struct _enable_impl { using type = void; };
 
template <typename... ts>
using _enable_t = typename _enable_impl<ts...>::type;

template <typename... ts>
struct _false
{
	static constexpr bool value = false;
};

template <typename t>
struct _clean
{
	using type = t;
};

template <typename t>
struct _clean<const t>
{
	using type = t;
};

template <typename t>
struct _clean<volatile t>
{
	using type = t;
};

template <typename t>
struct _clean<t&>
{
	using type = t;
};

template <typename t>
struct _clean<const t&>
{
	using type = t;
};

template <typename t>
struct _clean<volatile t&>
{
	using type = t;
};

template <typename t>
struct _clean<t&&>
{
	using type = t;
};

template <typename t>
struct _clean<const t&&>
{
	using type = t;
};

template <typename t>
struct _clean<volatile t&&>
{
	using type = t;
};

template <typename... ts>
struct _front_impl;

template <typename t, typename... ts>
struct _front_impl<t, ts...>
{
	using type = t;
};

template <typename... ts>
struct _back_impl;

template <typename t, typename st, typename... ts>
struct _back_impl<t, st, ts...>
{
	using type = typename _back_impl<st, ts...>::type;
};

template <typename t>
struct _back_impl<t>
{
	using type = t;
};

template <_size_t i, typename... ts>
struct _pop_front_impl;

template <_size_t i, typename t, typename... ts>
struct _pop_front_impl<i, t, ts...> 
{
	using type = typename _pop_front_impl<i-1, ts...>::type;
};

template <typename t, typename... ts>
struct _pop_front_impl<0, t, ts...> 
{
	using type = _tetter<t, ts...>;
};

template <_size_t i>
struct _pop_front_impl<i> 
{
	using type = _tetter<>;
};

template <_size_t i, typename tis, typename tos>
struct _pop_back_impl
{
	using type = void;
};

template <_size_t i, typename ti, typename... tis, typename... tos>
struct _pop_back_impl<i, _tetter<ti, tis...>, _tetter<tos...>>
{
	using type = typename _conditional<sizeof...(tos) + 1 == i, _tetter<tos..., ti>, typename _pop_back_impl<i, _tetter<tis...>, _tetter<tos..., ti>>::type>::type;
};

template <_size_t i, typename... ts>
struct _get_impl;

template <_size_t i, typename t, typename... ts>
struct _get_impl<i, t, ts...>
{
	using type = typename _get_impl<i-1, ts...>::type;
};

template <typename t, typename... ts>
struct _get_impl<0, t, ts...>
{
	using type = t;
};

template <_size_t i>
struct _get_impl<i>
{
	static_assert(false, "out of bounds");
};

template <_size_t i, _size_t c, typename tis, typename tos>
struct _slice_impl;

template <_size_t i, _size_t c, typename ti, typename... tis, typename... tos>
struct _slice_impl<i, c, _tetter<ti, tis...>, _tetter<tos...>>
{
	using type = typename _slice_impl<i-1, c, _tetter<tis...>, _tetter<tos...>>::type;
};

template <_size_t c, typename ti, typename... tis, typename... tos>
struct _slice_impl<0, c, _tetter<ti, tis...>, _tetter<tos...>>
{
	using type = typename _slice_impl<0, c-1, _tetter<tis...>, _tetter<tos..., ti>>::type;
};

template <typename ti, typename... tis, typename... tos>
struct _slice_impl<0, 0, _tetter<ti, tis...>, _tetter<tos...>>
{
	using type = _tetter<tos...>;
};

template <_size_t i, _size_t c, typename... tos>
struct _slice_impl<i, c, _tetter<>, _tetter<tos...>>
{
	using type = _tetter<tos...>;
};

template <typename tli, typename tlo>
struct _reverse_impl;

template <typename tli, typename... tlis, typename... tlos>
struct _reverse_impl<_tetter<tli, tlis...>, _tetter<tlos...>>
{
	using type = typename _reverse_impl<_tetter<tlis...>, _tetter<tli, tlos...>>::type;
};

template <typename... tlos>
struct _reverse_impl<_tetter<>, _tetter<tlos...>>
{
	using type = _tetter<tlos...>;
};

template <_size_t i, typename tl, typename tlo>
struct _copy_impl;

template <_size_t i, typename... tls, typename... tlos>
struct _copy_impl<i, _tetter<tls...>, _tetter<tlos...>>
{
	using type = typename _copy_impl<i-1, _tetter<tls...>, _tetter<tlos..., tls...>>::type;
};

template <typename... tls, typename... tlos>
struct _copy_impl<0, _tetter<tls...>, _tetter<tlos...>>
{
	using type = _tetter<tlos...>;
};

template <_size_t i, typename t, typename tlo>
struct _repeat_impl_impl;

template <_size_t i, typename t, typename... tlo>
struct _repeat_impl_impl<i, t, _tetter<tlo...>>
{
	using type = typename _repeat_impl_impl<i-1, t, _tetter<tlo..., t>>::type;
};

template <typename t, typename... tlo>
struct _repeat_impl_impl<0, t, _tetter<tlo...>>
{
	using type = _tetter<tlo...>;
};

template <_size_t i, typename tli, typename tlo>
struct _repeat_impl;

template <_size_t i, typename tli, typename... tlis, typename... tlos>
struct _repeat_impl<i, _tetter<tli, tlis...>, _tetter<tlos...>>
{
	using type = typename _repeat_impl<i, _tetter<tlis...>, typename _repeat_impl_impl<i, tli, _tetter<tlos...>>::type>::type;
};

template <_size_t i, typename... tlos>
struct _repeat_impl<i, _tetter<>, _tetter<tlos...>>
{
	using type = _tetter<tlos...>;
};

template <template <typename, _size_t, typename...> typename vt, typename t, _size_t i, typename ats, typename enable>
struct _wrapper_impl
{
	using type = vt<t, i>;
};

template <template <typename, _size_t, typename...> typename vt, typename t, _size_t i, typename... ts>
struct _wrapper_impl<vt, t, i, _tetter<ts...>, _enable_t<vt<t, i, ts...>>>
{
	using type = vt<t, i, ts...>;
};

template <template <typename, _size_t, typename...> typename vt>
struct _wrapper
{
	template <typename t, _size_t i, typename ats>
	using type = typename _wrapper_impl<vt, t, i, ats, _enable_t<>>::type;
};

template <template <typename, typename> typename vt>
struct _sort_wrapper
{
	template <typename lt, typename rt>
	using type = vt<lt, rt>;
};

template <template <typename> typename vt>
struct _action_wrapper
{
	template <typename t>
	using type = vt<t>;
};

template <template <typename> typename vt>
struct _condition_wrapper
{
	template <typename t>
	using type = vt<t>;
};

template <_size_t i, typename w, typename rts, typename ats, typename ots>
struct _type_impl;

template <_size_t i, typename w, typename rt, typename... rts, typename ats, typename... ots>
struct _type_impl<i, w, _tetter<rt, rts...>, ats, _tetter<ots...>>
{
	using type = typename _type_impl<i+1, w, _tetter<rts...>, ats, _tetter<ots..., typename w::template type<rt, i, ats>::type>>::type;
};

template <_size_t i, typename w, typename ats, typename... ots>
struct _type_impl<i, w, _tetter<>, ats, _tetter<ots...>>
{
	using type = _tetter<ots...>; 
};

template <_size_t i, typename w, typename rts, typename ats>
struct _bool_value_impl;

template <_size_t i, typename w, typename rt, typename srt, typename... rts, typename ats>
struct _bool_value_impl<i, w, _tetter<rt, srt, rts...>, ats>
{
	static constexpr auto all = w::template type<rt, i, ats>::value && _bool_value_impl<i+1, w, _tetter<srt, rts...>, ats>::all;
	static constexpr auto any = w::template type<rt, i, ats>::value || _bool_value_impl<i+1, w, _tetter<srt, rts...>, ats>::any;
};

template <_size_t i, typename w, typename rt, typename ats>
struct _bool_value_impl<i, w, _tetter<rt>, ats>
{
	static constexpr auto all = w::template type<rt, i, ats>::value;
	static constexpr auto any = w::template type<rt, i, ats>::value;
};

template <_size_t i, typename w, typename ats>
struct _bool_value_impl<i, w, _tetter<>, ats>
{
	static constexpr auto all = true;
	static constexpr auto any = false;
};

template <_size_t i, typename w, typename rts, typename ats>
struct _int_value_impl;

template <_size_t i, typename w, typename rt, typename srt, typename... rts, typename ats>
struct _int_value_impl<i, w, _tetter<rt, srt, rts...>, ats>
{
	static constexpr auto sum = w::template type<rt, i, ats>::value + _int_value_impl<i+1, w, _tetter<srt, rts...>, ats>::sum;
	static constexpr auto min = w::template type<rt, i, ats>::value < _int_value_impl<i+1, w, _tetter<srt, rts...>, ats>::min ? w::template type<rt, i, ats>::value : _int_value_impl<i+1, w, _tetter<srt, rts...>, ats>::min;
	static constexpr auto max = w::template type<rt, i, ats>::value > _int_value_impl<i+1, w, _tetter<srt, rts...>, ats>::max ? w::template type<rt, i, ats>::value : _int_value_impl<i+1, w, _tetter<srt, rts...>, ats>::max;
};

template <_size_t i, typename w, typename rt, typename ats>
struct _int_value_impl<i, w, _tetter<rt>, ats>
{
	static constexpr auto sum = w::template type<rt, i, ats>::value;
	static constexpr auto min = w::template type<rt, i, ats>::value;
	static constexpr auto max = w::template type<rt, i, ats>::value;
};

template <_size_t i, typename w, typename ats>
struct _int_value_impl<i, w, _tetter<>, ats>
{
	static constexpr auto sum = 0;
	static constexpr auto min = 0;
	static constexpr auto max = 0;
};

template <_size_t i, typename w, typename rts, typename ats>
struct _find_impl;

template <_size_t i, typename w, typename rt, typename... rts, typename ats>
struct _find_impl<i, w, _tetter<rt, rts...>, ats> 
{
private:
	static constexpr bool found = w::template type<rt, i, ats>::value;
	using next = _find_impl<i+1, w, _tetter<rts...>, ats>;

public:
	static constexpr _size_t index = found ? i : next::index;
	using type = typename _conditional<found, rt, typename next::type>::type;
};

template <_size_t i, typename w, typename ats>
struct _find_impl<i, w, _tetter<>, ats>
{
	static constexpr _size_t index = i;
	using type = void;
};

template <_size_t i, typename w, typename tli, typename tlo, typename ats>
struct _filter_impl;

template <_size_t i, typename w, typename tli, typename... tlis, typename... tlos, typename ats>
struct _filter_impl<i, w, _tetter<tli, tlis...>, _tetter<tlos...>, ats>
{
	using type = typename _filter_impl<i+1, w, _tetter<tlis...>, typename _conditional<w::template type<tli, i, ats>::value, _tetter<tlos..., tli>, _tetter<tlos...>>::type, ats>::type;
};

template <_size_t i, typename w, typename... tlos, typename ats>
struct _filter_impl<i, w, _tetter<>, _tetter<tlos...>, ats>
{
	using type = _tetter<tlos...>;
};

template <typename w, typename t, typename tli, typename tlo>
struct _sort_impl_impl;

template <typename w, typename t, typename tli, typename... tlis, typename... tlos>
struct _sort_impl_impl<w, t, _tetter<tli, tlis...>, _tetter<tlos...>>
{
	using type = typename _conditional<w::template type<t, tli>::value, _tetter<tlos..., t, tli, tlis...>, typename _sort_impl_impl<w, t, _tetter<tlis...>, _tetter<tlos..., tli>>::type>::type;
};

template <typename w, typename t, typename... tlos>
struct _sort_impl_impl<w, t, _tetter<>, _tetter<tlos...>>
{
	using type = _tetter<tlos..., t>;
};
template <typename w, typename tli, typename tlo>
struct _sort_impl;

template <typename w, typename tli, typename... tlis, typename... tlos> 
struct _sort_impl<w, _tetter<tli, tlis...>, _tetter<tlos...>>
{
	using type = typename _sort_impl<w, _tetter<tlis...>, typename _sort_impl_impl<w, tli, _tetter<tlos...>, _tetter<>>::type>::type;
};

template <typename w, typename... tlos> 
struct _sort_impl<w, _tetter<>, _tetter<tlos...>>
{
	using type = _tetter<tlos...>;
};

template <typename w, typename t, _size_t i, typename ats, typename enable, typename... args>
struct _invoke_impl;

template <typename w, typename t, _size_t i, typename ats, typename... args>
struct _invoke_impl<w, t, i, ats, _enable_t<decltype(typename w::template type<t, i, ats>{}(_declval<args>()...))>, args...>
{
	_tetter_force_inline static constexpr _tetter_decltype(typename w::template type<t, i, ats>{}(_declval<args>()...)) call(args&&... _args)
	{
		return typename w::template type<t, i, ats>{}(static_cast<args&&>(_args)...);
	}
};

#if _tetter_static_call_operator
template <typename w, typename t, _size_t i, typename ats, typename enable, typename... args>
struct _invoke_static_impl
{
	_tetter_force_inline static constexpr _tetter_decltype(_invoke_impl<w, t, i, ats, _enable_t<>, args...>::call(_declval<args>()...)) call(args&&... _args)
	{
		return _invoke_impl<w, t, i, ats, _enable_t<>, args...>::call(static_cast<args&&>(_args)...);
	}
};

template <typename w, typename t, _size_t i, typename ats, typename... args>
struct _invoke_static_impl<w, t, i, ats, _enable_t<decltype(w::template type<t, i, ats>::operator()(_declval<args>()...))>, args...>
{
	_tetter_force_inline static constexpr _tetter_decltype(w::template type<t, i, ats>::operator()(_declval<args>()...)) call(args&&... _args)
	{
		return w::template type<t, i, ats>::operator()(static_cast<args&&>(_args)...);
	}
};
#endif

template <typename w, typename t, _size_t i, typename ats, typename enable, typename... args>
struct _invoke
{
#if _tetter_static_call_operator
	_tetter_force_inline static constexpr _tetter_decltype(_invoke_static_impl<w, t, i, ats, _enable_t<>, args...>::call(_declval<args>()...)) call(args&&... _args)
	{
		return _invoke_static_impl<w, t, i, ats, _enable_t<>, args...>::call(static_cast<args&&>(_args)...);
	}
#else
	_tetter_force_inline static constexpr _tetter_decltype(_invoke_impl<w, t, i, ats, _enable_t<>, args...>::call(_declval<args>()...)) call(args&&... _args)
	{
		return _invoke_impl<w, t, i, ats, _enable_t<>, args...>::call(static_cast<args&&>(_args)...);
	}
#endif
};

template <typename w, typename t, _size_t i, typename ats, typename... args>
struct _invoke<w, t, i, ats, _enable_t<decltype(w::template type<t, i, ats>::call(_declval<args>()...))>, args...>
{
	_tetter_force_inline static constexpr _tetter_decltype(w::template type<t, i, ats>::call(_declval<args>()...)) call(args&&... _args)
	{
		return w::template type<t, i, ats>::call(static_cast<args&&>(_args)...);
	}
};

template <_size_t i, typename w, typename rts, typename ats, typename... args>
struct _call_impl;

template <_size_t i, typename w, typename rt, typename srt, typename... rts, typename ats, typename... args>
struct _call_impl<i, w, _tetter<rt, srt, rts...>, ats, args...>
{
	_tetter_force_inline static constexpr void call(args&&... _args)
	{
		_invoke<w, rt, i, ats, _enable_t<>, args&...>::call(static_cast<args&>(_args)...); 
		_call_impl<i+1, w, _tetter<srt, rts...>, ats, args...>::call(static_cast<args&&>(_args)...);
	}
};

template <_size_t i, typename w, typename rt, typename ats, typename... args>
struct _call_impl<i, w, _tetter<rt>, ats, args...>
{
	_tetter_force_inline static constexpr void call(args&&... _args)
	{
		_invoke<w, rt, i, ats, _enable_t<>, args...>::call(static_cast<args&&>(_args)...); 
	}
};

template <_size_t i, typename w, typename ats, typename... args>
struct _call_impl<i, w, _tetter<>, ats, args...>
{
	_tetter_force_inline static constexpr void call(args&&... _args)
	{
		((void) _args, ...);
	}
};

template <_size_t i, typename w, typename rts, typename it, typename ats, typename... args>
struct _call_pipe_impl;

template <_size_t i, typename w, typename rt, typename srt, typename... rts, typename it, typename ats, typename... args>
struct _call_pipe_impl<i, w, _tetter<rt, srt, rts...>, it, ats, args...>
{
	_tetter_force_inline static constexpr _tetter_decltype(_call_pipe_impl<i+1, w, _tetter<srt, rts...>, decltype(_invoke<w, rt, i, ats, _enable_t<>, it, args&...>::call(_declval<it>(), _declval<args&>()...)), ats, args...>::call_pipe(
	                                                                              _declval<decltype(_invoke<w, rt, i, ats, _enable_t<>, it, args&...>::call(_declval<it>(), _declval<args&>()...))>(), _declval<args>()...))
		call_pipe(it&& _it, args&&... _args)
	{
		return _call_pipe_impl<i+1, w, _tetter<srt, rts...>, decltype(_invoke<w, rt, i, ats, _enable_t<>, it, args&...>::call(_declval<it>(), _declval<args&>()...)), ats, args...>::call_pipe(
		                                                              _invoke<w, rt, i, ats, _enable_t<>, it, args&...>::call(static_cast<it&&>(_it), static_cast<args&>(_args)...), static_cast<args&&>(_args)...);
	}
};

template <_size_t i, typename w, typename rt, typename it, typename ats, typename... args>
struct _call_pipe_impl<i, w, _tetter<rt>, it, ats, args...>
{
	_tetter_force_inline static constexpr _tetter_decltype(_invoke<w, rt, i, ats, _enable_t<>, it, args...>::call(_declval<it>(), _declval<args>()...)) call_pipe(it&& _it, args&&... _args)
	{
		return _invoke<w, rt, i, ats, _enable_t<>, it, args...>::call(static_cast<it&&>(_it), static_cast<args&&>(_args)...); 
	}
};

template <_size_t i, typename w, typename it, typename ats, typename... args>
struct _call_pipe_impl<i, w, _tetter<>, it, ats, args...>
{
	_tetter_force_inline static constexpr it&& call_pipe(it&& _it, args&&... _args)
	{
		return static_cast<it&&>(_it);
	}
};

template <_size_t i, typename w, typename rts, typename ats, typename... args>
struct _call_bool_impl;

template <_size_t i, typename w, typename rt, typename srt, typename... rts, typename ats, typename... args>
struct _call_bool_impl<i, w, _tetter<rt, srt, rts...>, ats, args...>
{
	_tetter_force_inline static constexpr _tetter_decltype(_invoke<w, rt, i, ats, _enable_t<>, args&...>::call(_declval<args&>()...) && 
		_call_bool_impl<i+1, w, _tetter<srt, rts...>, ats, args...>::call_all(_declval<args>()...)) call_all(args&&... _args)
	{
		return _invoke<w, rt, i, ats, _enable_t<>, args&...>::call(static_cast<args&>(_args)...) && 
			_call_bool_impl<i+1, w, _tetter<srt, rts...>, ats, args...>::call_all(static_cast<args&&>(_args)...);
	}

	_tetter_force_inline static constexpr _tetter_decltype(_invoke<w, rt, i, ats, _enable_t<>, args&...>::call(_declval<args&>()...) || 
		_call_bool_impl<i+1, w, _tetter<srt, rts...>, ats, args...>::call_any(_declval<args>()...)) call_any(args&&... _args)
	{
		return _invoke<w, rt, i, ats, _enable_t<>, args&...>::call(static_cast<args&>(_args)...) || 
			_call_bool_impl<i+1, w, _tetter<srt, rts...>, ats, args...>::call_any(static_cast<args&&>(_args)...);
	}
};

template <_size_t i, typename w, typename rt, typename ats, typename... args>
struct _call_bool_impl<i, w, _tetter<rt>, ats, args...>
{
	_tetter_force_inline static constexpr _tetter_decltype(_invoke<w, rt, i, ats, _enable_t<>, args...>::call(_declval<args>()...)) call_all(args&&... _args)
	{
		return _invoke<w, rt, i, ats, _enable_t<>, args...>::call(static_cast<args&&>(_args)...); 
	}

	_tetter_force_inline static constexpr _tetter_decltype(_invoke<w, rt, i, ats, _enable_t<>, args...>::call(_declval<args>()...)) call_any(args&&... _args)
	{
		return _invoke<w, rt, i, ats, _enable_t<>, args...>::call(static_cast<args&&>(_args)...); 
	}
};

template <_size_t i, typename w, typename ats, typename... args>
struct _call_bool_impl<i, w, _tetter<>, ats, args...>
{
	_tetter_force_inline static constexpr bool call_all(args&&... _args)
	{
		((void) _args, ...);
		return true;
	}

	_tetter_force_inline static constexpr bool call_any(args&&... _args)
	{
		((void) _args, ...);
		return false;
	}
};

template <_size_t i, typename w, typename rts, typename ats, typename... args>
struct _call_int_impl;

template <_size_t i, typename w, typename rt, typename srt, typename... rts, typename ats, typename... args>
struct _call_int_impl<i, w, _tetter<rt, srt, rts...>, ats, args...>
{
	_tetter_force_inline static constexpr _tetter_decltype(_invoke<w, rt, i, ats, _enable_t<>, args&...>::call(_declval<args&>()...) + 
		_call_int_impl<i+1, w, _tetter<srt, rts...>, ats, args...>::call_sum(_declval<args>()...)) call_sum(args&&... _args)
	{
		return _invoke<w, rt, i, ats, _enable_t<>, args&...>::call(static_cast<args&>(_args)...) +
			_call_int_impl<i+1, w, _tetter<srt, rts...>, ats, args...>::call_sum(static_cast<args&&>(_args)...);
	}

	_tetter_force_inline static constexpr decltype(_invoke<w, rt, i, ats, _enable_t<>, args&...>::call(_declval<args&>()...)) call_min(args&&... _args)
	{
		auto this_value = _invoke<w, rt, i, ats, _enable_t<>, args&...>::call(static_cast<args&>(_args)...);
		auto other_value = _call_int_impl<i+1, w, _tetter<srt, rts...>, ats, args...>::call_min(static_cast<args&&>(_args)...);

		if (this_value < other_value)
			return this_value;
		else
			return other_value;
	}

	_tetter_force_inline static constexpr decltype(_invoke<w, rt, i, ats, _enable_t<>, args&...>::call(_declval<args&>()...)) call_max(args&&... _args)
	{
		auto this_value = _invoke<w, rt, i, ats, _enable_t<>, args&...>::call(static_cast<args&>(_args)...);
		auto other_value = _call_int_impl<i+1, w, _tetter<srt, rts...>, ats, args...>::call_max(static_cast<args&&>(_args)...);

		if (this_value > other_value)
			return this_value;
		else
			return other_value;
	}
};

template <_size_t i, typename w, typename rt, typename ats, typename... args>
struct _call_int_impl<i, w, _tetter<rt>, ats, args...>
{
	_tetter_force_inline static constexpr _tetter_decltype(_invoke<w, rt, i, ats, _enable_t<>, args...>::call(_declval<args>()...)) call_sum(args&&... _args)
	{
		return _invoke<w, rt, i, ats, _enable_t<>, args...>::call(static_cast<args&&>(_args)...); 
	}

	_tetter_force_inline static constexpr _tetter_decltype(_invoke<w, rt, i, ats, _enable_t<>, args...>::call(_declval<args>()...)) call_min(args&&... _args)
	{
		return _invoke<w, rt, i, ats, _enable_t<>, args...>::call(static_cast<args&&>(_args)...); 
	}

	_tetter_force_inline static constexpr _tetter_decltype(_invoke<w, rt, i, ats, _enable_t<>, args...>::call(_declval<args>()...)) call_max(args&&... _args)
	{
		return _invoke<w, rt, i, ats, _enable_t<>, args...>::call(static_cast<args&&>(_args)...); 
	}
};

template <_size_t i, typename w, typename ats, typename... args>
struct _call_int_impl<i, w, _tetter<>, ats, args...>
{
	_tetter_force_inline static constexpr int call_sum(args&&... _args)
	{
		((void) _args, ...);
		return 0; 
	}

	_tetter_force_inline static constexpr int call_min(args&&... _args)
	{
		((void) _args, ...);
		return 0; 
	}

	_tetter_force_inline static constexpr int call_max(args&&... _args)
	{
		((void) _args, ...);
		return 0; 
	}
};

template <typename w, typename enable, typename... ts>
struct _try_type_impl
{
	static_assert(_false<w>::value, "to use tetter's ::map<>, iterator must have ::type");
};

template <typename w, typename... ts>
struct _try_type_impl<w, _enable_t<typename _type_impl<0, w, _tetter<ts...>, _tetter<ts...>, _tetter<>>::type>, ts...>
{
	using type = typename _type_impl<0, w, _tetter<ts...>, _tetter<ts...>, _tetter<>>::type;
};

template <typename w, typename enable, typename... ts>
struct _try_bool_value_impl
{
	static_assert(_false<w>::value, "to use tetter's ::value<>::all, ::value<>::any and ::value<>::none, iterator must have ::value (of type bool or anything with operators: &&, || and !)");
};

template <typename w, typename... ts>
struct _try_bool_value_impl<w, _enable_t<decltype(_bool_value_impl<0, w, _tetter<ts...>, _tetter<ts...>>::all),
                                        decltype(!_bool_value_impl<0, w, _tetter<ts...>, _tetter<ts...>>::any)>, ts...>
{
	static constexpr auto all = _bool_value_impl<0, w, _tetter<ts...>, _tetter<ts...>>::all;
	static constexpr auto any = _bool_value_impl<0, w, _tetter<ts...>, _tetter<ts...>>::any;
	static constexpr auto none = !any;
};

template <typename w, typename enable, typename... ts>
struct _try_int_value_impl
{
	static_assert(_false<w>::value, "to use tetter's ::value<>::sum, ::value<>::avg, ::value<>::min and ::value<>::max, iterator must have ::value (of type int or anything with operators: +, /, < and >)");
};

template <typename w, typename... ts>
struct _try_int_value_impl<w, _enable_t<decltype(_int_value_impl<0, w, _tetter<ts...>, _tetter<ts...>>::sum / _declval<_size_t>()),
                                        decltype(_int_value_impl<0, w, _tetter<ts...>, _tetter<ts...>>::min),
                                        decltype(_int_value_impl<0, w, _tetter<ts...>, _tetter<ts...>>::max)>, ts...>
{
	static constexpr auto sum = _int_value_impl<0, w, _tetter<ts...>, _tetter<ts...>>::sum;
	static constexpr auto avg = sum / static_cast<_size_t>(sizeof...(ts) > 0 ? sizeof...(ts) : 1);
	static constexpr auto min = _int_value_impl<0, w, _tetter<ts...>, _tetter<ts...>>::min;
	static constexpr auto max = _int_value_impl<0, w, _tetter<ts...>, _tetter<ts...>>::max;
};

template <typename w, typename enable, typename... ts>
struct _try_value_impl : public _try_bool_value_impl<w, _enable_t<>, ts...>, public _try_int_value_impl<w, _enable_t<>, ts...> {};

template <typename w, typename enable, typename... ts>
struct _try_find_impl
{
	static_assert(_false<w>::value, "to use tetter's ::find, iterator must have ::value (of type bool or anything boollike)");
};

template <typename w, typename... ts>
struct _try_find_impl<w, _enable_t<decltype(_find_impl<0, w, _tetter<ts...>, _tetter<ts...>>::index)>, ts...>
{
private:
	using result = _find_impl<0, w, _tetter<ts...>, _tetter<ts...>>;

public:
	static constexpr _size_t index = result::index;
	static constexpr bool value = index != sizeof...(ts);
	using type = typename result::type;
};

template <typename w, typename enable, typename... ts>
struct _try_filter_impl
{
	static_assert(_false<w>::value, "to use tetter's ::filter, iterator must have ::value (of type bool or anything boollike)");
};

template <typename w, typename... ts>
struct _try_filter_impl<w, _enable_t<typename _filter_impl<0, w, _tetter<ts...>, _tetter<>, _tetter<ts...>>::type>, ts...>
{
	using type = typename _filter_impl<0, w, _tetter<ts...>, _tetter<>, _tetter<ts...>>::type;
};

template <typename w, typename enable, typename... ts>
struct _try_sort_impl
{
	static_assert(_false<w>::value, "to use tetter's ::sort, sorter must have ::value (of type bool or anything boollike)");
};

template <typename w, typename... ts>
struct _try_sort_impl<w, _enable_t<typename _sort_impl<w, _tetter<ts...>, _tetter<>>::type>, ts...>
{
	using type = typename _sort_impl<w, _tetter<ts...>, _tetter<>>::type;
};

template <typename w, typename args, typename enable, typename... ts>
struct _try_call_impl
{
	static_assert(_false<w>::value, "to use tetter's ::call, iterator must have ::call or .operator() (with default constructor)");
};

template <typename w, typename... args, typename... ts>
struct _try_call_impl<w, _tetter<args...>, _enable_t<decltype(_call_impl<0, w, _tetter<ts...>, _tetter<ts...>, args...>::call(_declval<args>()...))>, ts...>
{
	_tetter_force_inline static constexpr void call(args&&... _args)
	{
		_call_impl<0, w, _tetter<ts...>, _tetter<ts...>, args...>::call(static_cast<args&&>(_args)...);
	}
};

template <typename w, typename it, typename args, typename enable, typename... ts>
struct _try_call_pipe_impl
{
	static_assert(_false<w>::value, "to use tetter's ::call_pipe, iterator must have ::call or .operator() (with default constructor)");
};

template <typename w, typename it, typename... args, typename... ts>
struct _try_call_pipe_impl<w, it, _tetter<args...>, _enable_t<decltype(_call_pipe_impl<0, w, _tetter<ts...>, it, _tetter<ts...>, args...>::call_pipe(_declval<it>(), _declval<args>()...))>, ts...>
{
	_tetter_force_inline static constexpr _tetter_decltype(_call_pipe_impl<0, w, _tetter<ts...>, it, _tetter<ts...>, args...>::call_pipe(_declval<it>(), _declval<args>()...)) call_pipe(it&& _iv, args&&... _args)
	{
		return _call_pipe_impl<0, w, _tetter<ts...>, it, _tetter<ts...>, args...>::call_pipe(static_cast<it&&>(_iv), static_cast<args&&>(_args)...);
	}
};

template <typename w, typename args, typename enable, typename... ts>
struct _try_call_bool_impl
{
	static_assert(_false<w>::value, "to use tetter's ::call_all, ::call_any and ::call_none, iterator must have ::call or .operator() (with default constructor) which returns bool or anything with operators: &&, || and !");
};

template <typename w, typename... args, typename... ts>
struct _try_call_bool_impl<w, _tetter<args...>, _enable_t<decltype(_call_bool_impl<0, w, _tetter<ts...>, _tetter<ts...>, args...>::call_all(_declval<args>()...)),
                                                          decltype(!_call_bool_impl<0, w, _tetter<ts...>, _tetter<ts...>, args...>::call_any(_declval<args>()...))>, ts...>
{
	_tetter_force_inline static constexpr _tetter_decltype(_call_bool_impl<0, w, _tetter<ts...>, _tetter<ts...>, args...>::call_all(_declval<args>()...)) call_all(args&&... _args)
	{
		return _call_bool_impl<0, w, _tetter<ts...>, _tetter<ts...>, args...>::call_all(static_cast<args&&>(_args)...);
	}

	_tetter_force_inline static constexpr _tetter_decltype(_call_bool_impl<0, w, _tetter<ts...>, _tetter<ts...>, args...>::call_any(_declval<args>()...)) call_any(args&&... _args)
	{
		return _call_bool_impl<0, w, _tetter<ts...>, _tetter<ts...>, args...>::call_any(static_cast<args&&>(_args)...);
	}

	_tetter_force_inline static constexpr _tetter_decltype(!call_any(_declval<args>()...)) call_none(args&&... _args)
	{
		return !call_any(static_cast<args&&>(_args)...);
	}
};

template <typename w, typename args, typename enable, typename... ts>
struct _try_call_int_impl
{
	static_assert(_false<w>::value, "to use tetter's ::call_sum, ::call_avg, ::call_min and ::call_min, iterator must have ::call or .operator() (with default constructor) which returns int or anything with operators: +, /, < and >");
};

template <typename w, typename... args, typename... ts>
struct _try_call_int_impl<w, _tetter<args...>, _enable_t<decltype(_call_int_impl<0, w, _tetter<ts...>, _tetter<ts...>, args...>::call_sum(_declval<args>()...) / _declval<_size_t>()),
                                                         decltype(_call_int_impl<0, w, _tetter<ts...>, _tetter<ts...>, args...>::call_min(_declval<args>()...)),
														 decltype(_call_int_impl<0, w, _tetter<ts...>, _tetter<ts...>, args...>::call_max(_declval<args>()...))>, ts...>
{
	_tetter_force_inline static constexpr _tetter_decltype(_call_int_impl<0, w, _tetter<ts...>, _tetter<ts...>, args...>::call_sum(_declval<args>()...)) call_sum(args&&... _args)
	{
		return _call_int_impl<0, w, _tetter<ts...>, _tetter<ts...>, args...>::call_sum(static_cast<args&&>(_args)...);
	}

	_tetter_force_inline static constexpr _tetter_decltype(call_sum(_declval<args>()...) / _declval<_size_t>()) call_avg(args&&... _args)
	{
		return call_sum(static_cast<args&&>(_args)...) / static_cast<_size_t>(sizeof...(ts) > 0 ? sizeof...(ts) : 1);
	}

	_tetter_force_inline static constexpr _tetter_decltype(_call_int_impl<0, w, _tetter<ts...>, _tetter<ts...>, args...>::call_min(_declval<args>()...)) call_min(args&&... _args)
	{
		return _call_int_impl<0, w, _tetter<ts...>, _tetter<ts...>, args...>::call_min(static_cast<args&&>(_args)...);
	}

	_tetter_force_inline static constexpr _tetter_decltype(_call_int_impl<0, w, _tetter<ts...>, _tetter<ts...>, args...>::call_max(_declval<args>()...)) call_max(args&&... _args)
	{
		return _call_int_impl<0, w, _tetter<ts...>, _tetter<ts...>, args...>::call_max(static_cast<args&&>(_args)...);
	}
};

template <bool invert, typename... ts>
struct _is_any_of
{
	template <typename t, _size_t i, typename ats>
	struct type
	{
		template <typename _t, _size_t>
		struct iterator_impl
		{
			static constexpr bool value = _is_same<_t, t>::value;
		};

		static constexpr bool value = invert ^ _try_value_impl<_wrapper<iterator_impl>, _enable_t<>, ts...>::any;
	};
};

/* template <typename w, typename ts>
struct _duplicats_impl_impl;

template <typename w, typename... ts>
struct _duplicats_impl_impl<w, _tetter<ts...>>
{
	static constexpr bool value = _try_find_impl<w, _enable_t<>, ts...>::value;
};

template <typename t, _size_t i, typename... ts>
struct _duplicats_impl
{
	static constexpr bool value = !_duplicats_impl_impl<_is_any_of<false, t>, typename _pop_front_impl<i+1, ts...>::type>::value;  
}; */

template <typename t, typename ts>
struct _join_impl_impl
{
	static_assert(_false<t>::value, "invalid use, tetter's ::join<t> requires t to have template args");
};

template <template <typename...> typename t, typename... jts, typename... ts>
struct _join_impl_impl<t<jts...>, _tetter<ts...>>
{
	using type = _tetter<ts..., jts...>;
};

template <typename gts, typename ts>
struct _join_impl;

template <typename gt, typename... gts, typename... ts>
struct _join_impl<_tetter<gt, gts...>, _tetter<ts...>>
{
	using type = typename _join_impl<_tetter<gts...>, typename _join_impl_impl<gt, _tetter<ts...>>::type>::type;
};

template <typename... ts>
struct _join_impl<_tetter<>, _tetter<ts...>>
{
	using type = _tetter<ts...>;
};

template <typename t, template <typename...> typename... cts>
struct _concat_impl_impl;

template <typename... ts, template <typename...> typename ct, template <typename...> typename... cts>
struct _concat_impl_impl<ct<ts...>, ct, cts...>
{
	using type = _tetter<ts...>;
};

template <typename t, template <typename...> typename ct, template <typename...> typename... cts>
struct _concat_impl_impl<t, ct, cts...>
{
	using type = typename _concat_impl_impl<t, cts...>::type;
};

template <typename t>
struct _concat_impl_impl<t>
{
	using type = t;
};

template <typename tli, typename... tlos>
struct _concat_impl_impl_impl
{
	using type = _tetter<tlos..., tli>;
};

template <typename... tlis, typename... tlos>
struct _concat_impl_impl_impl<_tetter<tlis...>, tlos...>
{
	using type = _tetter<tlos..., tlis...>;
};

template <typename tli, typename tlo, template <typename...> typename... cts>
struct _concat_impl;

template <typename tli, typename... tlis, typename... tlos, template <typename...> typename... cts>
struct _concat_impl<_tetter<tli, tlis...>, _tetter<tlos...>, cts...>
{
	using type = typename _concat_impl<_tetter<tlis...>, typename _concat_impl_impl_impl<typename _concat_impl_impl<tli, cts...>::type, tlos...>::type, cts...>::type;
};

template <typename... tlos, template <typename...> typename... cts>
struct _concat_impl<_tetter<>, _tetter<tlos...>, cts...>
{
	using type = _tetter<tlos...>;
};

template <typename t>
struct _concat_any_impl_impl
{
	using type = t;
};

template <template <typename...> typename ct, typename... ts>
struct _concat_any_impl_impl<ct<ts...>>
{
	using type = _tetter<ts...>;
};

template <typename tli, typename tlo>
struct _concat_any_impl;

template <typename tli, typename... tlis, typename... tlos>
struct _concat_any_impl<_tetter<tli, tlis...>, _tetter<tlos...>>
{
	using type = typename _concat_any_impl<_tetter<tlis...>, typename _concat_impl_impl_impl<typename _concat_any_impl_impl<tli>::type, tlos...>::type>::type;
};

template <typename... tlos>
struct _concat_any_impl<_tetter<>, _tetter<tlos...>>
{
	using type = _tetter<tlos...>;
};

template <_size_t i, typename tli, typename tlo, typename... ts>
struct _insert_impl;

template <_size_t i, typename tli, typename... tlis, typename... tlos, typename... ts>
struct _insert_impl<i, _tetter<tli, tlis...>, _tetter<tlos...>, ts...>
{
	using type = typename _insert_impl<i-1, _tetter<tlis...>, _tetter<tlos..., tli>, ts...>::type;
};

template <_size_t i, typename... tlos, typename... ts>
struct _insert_impl<i, _tetter<>, _tetter<tlos...>, ts...>
{
	using type = _tetter<tlos..., ts...>;
};

template <typename tli, typename... tlis, typename... tlos, typename... ts>
struct _insert_impl<0, _tetter<tli, tlis...>, _tetter<tlos...>, ts...>
{
	using type = _tetter<tlos..., ts..., tli, tlis...>;
};

template <typename... tlos, typename... ts>
struct _insert_impl<0, _tetter<>, _tetter<tlos...>, ts...>
{
	using type = _tetter<tlos..., ts...>;
};

template <_size_t i, _size_t c, typename tli, typename tlo>
struct _remove_impl;

template <_size_t i, _size_t c, typename tli, typename... tlis, typename... tlos>
struct _remove_impl<i, c, _tetter<tli, tlis...>, _tetter<tlos...>>
{
	using type = typename _remove_impl<i-1, c, _tetter<tlis...>, _tetter<tlos..., tli>>::type;
};

template <_size_t c, typename tli, typename... tlis, typename... tlos>
struct _remove_impl<0, c, _tetter<tli, tlis...>, _tetter<tlos...>>
{
	using type = typename _join_tetter<typename _pop_front_impl<c, tli, tlis...>::type, _tetter<tlos...>>::type;
};

template <_size_t i, _size_t c, typename... tlos>
struct _remove_impl<i, c, _tetter<>, _tetter<tlos...>>
{
	using type = _tetter<tlos...>;
};

template <_size_t i, typename t, typename tli, typename tlo>
struct _replace_impl;

template <_size_t i, typename t, typename tli, typename... tlis, typename... tlos>
struct _replace_impl<i, t, _tetter<tli, tlis...>, _tetter<tlos...>>
{
	using type = typename _replace_impl<i-1, t, _tetter<tlis...>, _tetter<tlos..., tli>>::type;
};

template <typename t, typename tli, typename... tlis, typename... tlos>
struct _replace_impl<0, t, _tetter<tli, tlis...>, _tetter<tlos...>>
{
	using type = _tetter<tlos..., t, tlis...>;
};

template <_size_t i, typename t, typename... tlos>
struct _replace_impl<i, t, _tetter<>, _tetter<tlos...>>
{
	static_assert(_false<t>::value, "out of bounds");
};

#if _tetter_generic_lambdas || _tetter_unevaluated_lambda

template <typename lt, typename t, _size_t i, typename ats, typename enable, typename... args>
struct _lambda_invoke_impl
{
	static_assert(_false<lt>::value, "lambda iterator must have operator()<t, i, optional_all_types...> or operator()<t, optional_all_types...> or operator<i, optional_all_types...>()");
};

template <typename lt, typename t, _size_t i, typename ats, typename... args>
struct _lambda_invoke_impl<lt, t, i, ats, _enable_t<decltype(_declval<lt>().template operator()<t>(_declval<args>()...))>, args...> 
{
	using result_type = decltype(_declval<lt>().template operator()<t>(_declval<args>()...));

	_tetter_force_inline static constexpr auto call(lt&& l, args&&... _args)
	{
		return static_cast<lt&&>(l).template operator()<t>(static_cast<args&&>(_args)...);
	}
};

template <typename lt, typename t, _size_t i, typename ats, typename... args>
struct _lambda_invoke_impl<lt, t, i, ats, _enable_t<decltype(_declval<lt>().template operator()<i>(_declval<args>()...))>, args...> 
{
	using result_type = decltype(_declval<lt>().template operator()<i>(_declval<args>()...));

	_tetter_force_inline static constexpr auto call(lt&& l, args&&... _args)
	{
		return static_cast<lt&&>(l).template operator()<i>(static_cast<args&&>(_args)...);
	}
};

template <typename lt, typename t, _size_t i, typename ats, typename... args>
struct _lambda_invoke_impl<lt, t, i, ats, _enable_t<decltype(_declval<lt>().template operator()<t, i>(_declval<args>()...))>, args...> 
{
	using result_type = decltype(_declval<lt>().template operator()<t, i>(_declval<args>()...));

	_tetter_force_inline static constexpr auto call(lt&& l, args&&... _args)
	{
		return static_cast<lt&&>(l).template operator()<t, i>(static_cast<args&&>(_args)...);
	}
};

template <typename lt, typename t, _size_t i, typename ats, typename enable, typename... args>
struct _lambda_invoke
{
	using result_type = typename _lambda_invoke_impl<lt, t, i, ats, _enable_t<>, args...>::result_type;

	_tetter_force_inline static constexpr auto call(lt&& l, args&&... _args)
	{
		return _lambda_invoke_impl<lt, t, i, ats, _enable_t<>, args...>::call(static_cast<lt&&>(l), static_cast<args&&>(_args)...);
	}
};

template <typename lt, typename t, _size_t i, typename... ats, typename... args>
struct _lambda_invoke<lt, t, i, _tetter<ats...>, _enable_t<decltype(_declval<lt>().template operator()<t, ats...>(_declval<args>()...))>, args...> 
{
	using result_type = decltype(_declval<lt>().template operator()<t, ats...>(_declval<args>()...));

	_tetter_force_inline static constexpr auto call(lt&& l, args&&... _args)
	{
		return static_cast<lt&&>(l).template operator()<t, ats...>(static_cast<args&&>(_args)...);
	}
};

template <typename lt, typename t, _size_t i, typename... ats, typename... args>
struct _lambda_invoke<lt, t, i, _tetter<ats...>, _enable_t<decltype(_declval<lt>().template operator()<i, ats...>(_declval<args>()...))>, args...> 
{
	using result_type = decltype(_declval<lt>().template operator()<i, ats...>(_declval<args>()...));

	_tetter_force_inline static constexpr auto call(lt&& l, args&&... _args)
	{
		return static_cast<lt&&>(l).template operator()<i, ats...>(static_cast<args&&>(_args)...);
	}
};

template <typename lt, typename t, _size_t i, typename... ats, typename... args>
struct _lambda_invoke<lt, t, i, _tetter<ats...>, _enable_t<decltype(_declval<lt>().template operator()<t, i, ats...>(_declval<args>()...))>, args...> 
{
	using result_type = decltype(_declval<lt>().template operator()<t, i, ats...>(_declval<args>()...));

	_tetter_force_inline static constexpr auto call(lt&& l, args&&... _args)
	{
		return static_cast<lt&&>(l).template operator()<t, i, ats...>(static_cast<args&&>(_args)...);
	}
};

template <typename lt, typename lft, typename rit, typename enable>
struct _lambda_invoke_sort
{
	static_assert(_false<lt>::value, "lambda sorter must have operator()<lt, rt>");
};

template <typename lt, typename lft, typename rit>
struct _lambda_invoke_sort<lt, lft, rit, _enable_t<decltype(_declval<lt>().template operator()<lft, rit>())>>
{
	using result_type = decltype(_declval<lt>().template operator()<lft, rit>());

	_tetter_force_inline static constexpr auto call(lt&& l)
	{
		return static_cast<lt&&>(l).template operator()<lft, rit>();
	}
};

#endif

#if _tetter_generic_lambdas

template <typename t, _size_t i, typename... ts>
struct _lambda_impl
{
	template <typename lt, typename... args>
	_tetter_force_inline static constexpr auto call(lt&& l, args&&... _args)
	{
		return _lambda_invoke<lt, t, i, _tetter<ts...>, _enable_t<>, args...>::call(static_cast<lt&&>(l), static_cast<args&&>(_args)...);
	}
};

template <typename t, _size_t i, typename... ts>
struct _lambda_pipe_impl
{
	template <typename ft, typename lt, typename... args>
	_tetter_force_inline static constexpr auto call(ft&& f, lt&& l, args&&... _args)
	{
		return _lambda_invoke<lt, t, i, _tetter<ts...>, _enable_t<>, ft, args...>::call(static_cast<lt&&>(l), static_cast<ft&&>(f), static_cast<args&&>(_args)...);
	}
};

#endif

#if _tetter_unevaluated_lambda

template <auto lambda>
struct _lambda_v_wrapper
{
	template <typename t, _size_t i, typename ats>
	struct type
	{
		static constexpr auto value = _lambda_invoke<const decltype(lambda)&, t, i, ats, _enable_t<>>::call(lambda);
	};
};

template <auto lambda>
struct _lambda_t_wrapper
{
	template <typename t, _size_t i, typename ats>
	struct type_impl
	{
		using type = typename _lambda_invoke<const decltype(lambda)&, t, i, ats, _enable_t<>>::result_type;
	};

	template <typename t, _size_t i, typename ats>
	using type = type_impl<t, i, ats>;
};

template <auto lambda>
struct _lambda_sort_wrapper
{
	template <typename lt, typename rt>
	struct type
	{
		static constexpr auto value = _lambda_invoke_sort<const decltype(lambda)&, lt, rt, _enable_t<>>::call(lambda);
	};
};

#endif

template <typename t>
struct _from_impl
{
	static_assert(_false<t>::value, "tetter_from requires generic type");
};

template <template <typename...> typename t, typename... ts>
struct _from_impl<t<ts...>>
{
	using type = _tetter<ts...>;
};

template <typename t, typename enable>
struct _from_args_impl
{
	static_assert(_false<t>::value, "tetter_from_args requires callable type");
};

template <typename t, typename... ts>
struct _from_args_impl<t(*)(ts...), _enable_t<>>
{
	using type = _tetter<ts...>; 
};

template <typename t, typename ct, typename... ts>
struct _from_args_impl<t(ct::*)(ts...), _enable_t<>>
{
	using type = _tetter<ct&, ts...>; 
};

template <typename t, typename ct, typename... ts>
struct _from_args_impl<t(ct::*)(ts...) const, _enable_t<>>
{
	using type = _tetter<const ct&, ts...>; 
};

template <typename t, typename ct, typename... ts>
struct _from_args_impl<t(ct::*)(ts...) volatile, _enable_t<>>
{
	using type = _tetter<volatile ct&, ts...>; 
};

template <typename t, typename ct, typename... ts>
struct _from_args_impl<t(ct::*)(ts...) const volatile, _enable_t<>>
{
	using type = _tetter<const volatile ct&, ts...>; 
};

template <typename t, typename ct, typename... ts>
struct _from_args_impl<t(ct::*)(ts...) &, _enable_t<>>
{
	using type = _tetter<ct&, ts...>; 
};

template <typename t, typename ct, typename... ts>
struct _from_args_impl<t(ct::*)(ts...) const &, _enable_t<>>
{
	using type = _tetter<const ct&, ts...>; 
};

template <typename t, typename ct, typename... ts>
struct _from_args_impl<t(ct::*)(ts...) volatile &, _enable_t<>>
{
	using type = _tetter<volatile ct&, ts...>; 
};

template <typename t, typename ct, typename... ts>
struct _from_args_impl<t(ct::*)(ts...) const volatile &, _enable_t<>>
{
	using type = _tetter<const volatile ct&, ts...>; 
};

template <typename t, typename ct, typename... ts>
struct _from_args_impl<t(ct::*)(ts...) &&, _enable_t<>>
{
	using type = _tetter<ct&&, ts...>; 
};

template <typename t, typename ct, typename... ts>
struct _from_args_impl<t(ct::*)(ts...) const &&, _enable_t<>>
{
	using type = _tetter<const ct&&, ts...>; 
};

template <typename t, typename ct, typename... ts>
struct _from_args_impl<t(ct::*)(ts...) volatile &&, _enable_t<>>
{
	using type = _tetter<volatile ct&&, ts...>; 
};

template <typename t, typename ct, typename... ts>
struct _from_args_impl<t(ct::*)(ts...) const volatile &&, _enable_t<>>
{
	using type = _tetter<const volatile ct&&, ts...>; 
};

template <typename t>
struct _from_args_impl<t, _enable_t<decltype(&t::operator())>>
{
	using type = typename _from_args_impl<decltype(&t::operator()), _enable_t<>>::type; 
};

template <_size_t i> 
struct _size_t_sequence
{
	static constexpr _size_t value = i;
};

template <_size_t i, template <_size_t> typename t, typename ts> 
struct _tetter_sequence_impl;

template <_size_t i, template <_size_t> typename t, typename... ts> 
struct _tetter_sequence_impl<i, t, _tetter<ts...>>
{
	using type = typename _tetter_sequence_impl<i-1, t, _tetter<t<i-1>, ts...>>::type; 
};

template <template <_size_t> typename t, typename... ts> 
struct _tetter_sequence_impl<0, t, _tetter<ts...>>
{
	using type = _tetter<ts...>; 
};

template <typename... ts>
struct _tetter_impl
{
	using front = typename _front_impl<ts...>::type;
	using back = typename _back_impl<ts...>::type;
};

template <>
struct _tetter_impl<>
{
};

} // _tetter

template <typename... ts>
struct tetter : public _tetter::_tetter_impl<ts...>
{
	static constexpr _tetter::_size_t count = sizeof...(ts);

	template <typename t>
	using front_or = typename _tetter::_front_impl<ts..., t>::type;
	template <typename t>
	using back_or = typename _tetter::_back_impl<t, ts...>::type;

	template <typename... types>
	using push_front = tetter<types..., ts...>; 
	template <typename... types>
	using push_back = tetter<ts..., types...>;

	template <_tetter::_size_t index, typename... types>
	using insert = typename _tetter::_insert_impl<index, _tetter::_tetter<ts...>, _tetter::_tetter<>, types...>::type::template cast<tetter>;

	template <_tetter::_size_t index, _tetter::_size_t amount = 1>
	using remove = typename _tetter::_remove_impl<index, amount, _tetter::_tetter<ts...>, _tetter::_tetter<>>::type::template cast<tetter>;

	template <_tetter::_size_t index, typename type>
	using replace = typename _tetter::_replace_impl<index, type, _tetter::_tetter<ts...>, _tetter::_tetter<>>::type::template cast<tetter>;

	template <typename... generic_types>
	using join = typename _tetter::_join_impl<_tetter::_tetter<generic_types...>, _tetter::_tetter<ts...>>::type::template cast<tetter>;

	using reverse = typename _tetter::_reverse_impl<_tetter::_tetter<ts...>, _tetter::_tetter<>>::type::template cast<tetter>;

	template <_tetter::_size_t amount>
	using pop_front_n = typename _tetter::_pop_front_impl<amount, ts...>::type::template cast<tetter>;
	template <_tetter::_size_t amount>
	using pop_back_n = typename _tetter::_conditional<(count > amount), typename _tetter::_pop_back_impl<count - amount, _tetter::_tetter<ts...>, _tetter::_tetter<>>::type, _tetter::_tetter<>>::type::template cast<tetter>;

	using pop_front = pop_front_n<1>;
	using pop_back = pop_back_n<1>;

	template <_tetter::_size_t index>
	using get = typename _tetter::_get_impl<index, ts...>::type;

	template <_tetter::_size_t index, _tetter::_size_t count>
	using slice = typename _tetter::_slice_impl<index, count, _tetter::_tetter<ts...>, _tetter::_tetter<>>::type::template cast<tetter>;

	template <template <typename...> typename to_type>
	using cast = to_type<ts...>;

	template <template <typename...> typename... types_to_concat> 
	using concat = typename _tetter::_concat_impl<_tetter::_tetter<ts...>, _tetter::_tetter<>, types_to_concat...>::type::template cast<tetter>;

	using concat_a = typename _tetter::_concat_any_impl<_tetter::_tetter<ts...>, _tetter::_tetter<>>::type::template cast<tetter>;

	// a, a, b, b, c, c
	template <_tetter::_size_t times>
	using repeat = typename _tetter::_repeat_impl<times + 1, _tetter::_tetter<ts...>, _tetter::_tetter<>>::type::template cast<tetter>;

	template <_tetter::_size_t times>
	using repeat_m = typename _tetter::_conditional<(times > 0), typename _tetter::_repeat_impl<times == 0 ? 1 : times, _tetter::_tetter<ts...>, _tetter::_tetter<>>::type::template cast<tetter>, tetter<>>::type;

	// a, b, c, a, b, c
	template <_tetter::_size_t times>
	using copy = typename _tetter::_copy_impl<times, _tetter::_tetter<ts...>, _tetter::_tetter<ts...>>::type::template cast<tetter>;

	template <_tetter::_size_t times>
	using copy_m = typename _tetter::_conditional<(times > 0), typename _tetter::_copy_impl<times == 0 ? 0 : times - 1, _tetter::_tetter<ts...>, _tetter::_tetter<ts...>>::type::template cast<tetter>, tetter<>>::type;

	template <template <typename current_type, _tetter::_size_t current_index, typename... optional_all_types> typename iterator>
	using find = _tetter::_try_find_impl<_tetter::_wrapper<iterator>, _tetter::_enable_t<>, ts...>;

	template <typename... types_to_find>
	using find_t = _tetter::_try_find_impl<_tetter::_is_any_of<false, types_to_find...>, _tetter::_enable_t<>, ts...>;

	template <template <typename current_type, _tetter::_size_t current_index, typename... optional_all_types> typename iterator>
	using filter = typename _tetter::_try_filter_impl<_tetter::_wrapper<iterator>, _tetter::_enable_t<>, ts...>::type::template cast<tetter>;

	template <typename... types_to_remove>
	using filter_t = typename _tetter::_try_filter_impl<_tetter::_is_any_of<true, types_to_remove...>, _tetter::_enable_t<>, ts...>::type::template cast<tetter>;

	template <typename... types_to_leave>
	using filter_t_l = typename _tetter::_try_filter_impl<_tetter::_is_any_of<false, types_to_leave...>, _tetter::_enable_t<>, ts...>::type::template cast<tetter>;

	template <template <typename left_type, typename right_type> typename sorter>
	using sort = typename _tetter::_try_sort_impl<_tetter::_sort_wrapper<sorter>, _tetter::_enable_t<>, ts...>::type::template cast<tetter>;

	template <template <typename current_type, _tetter::_size_t current_index, typename... optional_all_types> typename iterator>
	using map = typename _tetter::_try_type_impl<_tetter::_wrapper<iterator>, _tetter::_enable_t<>, ts...>::type::template cast<tetter>;

	/* TODO:
	template <_tetter::_size_t times, template<typename _tetter> action>
	using do_n = type_list<...>

	// tetter<...>::do_n_l<8, []<typename t>() -> t::map_l<[]<typename t, size_t>() -> t* {}> {}>;

	template <template<typename _tetter> action, template<typename _tetter> condition = until_type_changed>
	using do_until = type_list<...>
	
	// tetter<...>::do_until_l<[]<typename t>() -> t::concat_a {}>;
	*/

	template <template <typename current_type, _tetter::_size_t current_index, typename... optional_all_types> typename iterator>
	using value = _tetter::_try_value_impl<_tetter::_wrapper<iterator>, _tetter::_enable_t<>, ts...>;

	template <template <typename current_type, _tetter::_size_t current_index, typename... optional_all_types> typename iterator>
	using value_b = _tetter::_try_bool_value_impl<_tetter::_wrapper<iterator>, _tetter::_enable_t<>, ts...>;

	template <template <typename current_type, _tetter::_size_t current_index, typename... optional_all_types> typename iterator>
	using value_i = _tetter::_try_int_value_impl<_tetter::_wrapper<iterator>, _tetter::_enable_t<>, ts...>;

	// TODO: too slow 
	// static constexpr bool has_duplicats = !value<_tetter::_duplicats_impl>::all;
	// using remove_duplicats = typename reverse::template filter<_tetter::_duplicats_impl>::reverse;

	template <template <typename current_type, _tetter::_size_t current_index, typename... optional_all_types> typename iterator, typename... args>
	inline static constexpr void call(args&&... _args)
	{
		_tetter::_try_call_impl<_tetter::_wrapper<iterator>, _tetter::_tetter<args...>, _tetter::_enable_t<>, ts...>::call(static_cast<args&&>(_args)...);
	}

	// it<c, 2>(it<b, 1>(it<a, 0>(inital_value, args...), args...), args...)
	template <template <typename current_type, _tetter::_size_t current_index, typename... optional_all_types> typename iterator, typename inital_type, typename... args>
	inline static constexpr _tetter_decltype(_tetter::_try_call_pipe_impl<_tetter::_wrapper<iterator>, inital_type, _tetter::_tetter<args...>, _tetter::_enable_t<>, ts...>::call_pipe(_tetter::_declval<inital_type>(), _tetter::_declval<args>()...))
		call_pipe(inital_type&& inital_value = inital_type{}, args&&... _args)
	{
		return _tetter::_try_call_pipe_impl<_tetter::_wrapper<iterator>, inital_type, _tetter::_tetter<args...>, _tetter::_enable_t<>, ts...>::call_pipe(static_cast<inital_type&&>(inital_value), static_cast<args&&>(_args)...);
	}

	template <template <typename current_type, _tetter::_size_t current_index, typename... optional_all_types> typename iterator, typename... args>
	inline static constexpr _tetter_decltype(_tetter::_try_call_bool_impl<_tetter::_wrapper<iterator>, _tetter::_tetter<args...>, _tetter::_enable_t<>, ts...>::call_all(_tetter::_declval<args>()...))
		call_all(args&&... _args)
	{
		return _tetter::_try_call_bool_impl<_tetter::_wrapper<iterator>, _tetter::_tetter<args...>, _tetter::_enable_t<>, ts...>::call_all(static_cast<args&&>(_args)...);
	}

	template <template <typename current_type, _tetter::_size_t current_index, typename... optional_all_types> typename iterator, typename... args>
	inline static constexpr _tetter_decltype(_tetter::_try_call_bool_impl<_tetter::_wrapper<iterator>, _tetter::_tetter<args...>, _tetter::_enable_t<>, ts...>::call_any(_tetter::_declval<args>()...))
		call_any(args&&... _args)
	{
		return _tetter::_try_call_bool_impl<_tetter::_wrapper<iterator>, _tetter::_tetter<args...>, _tetter::_enable_t<>, ts...>::call_any(static_cast<args&&>(_args)...);
	}

	template <template <typename current_type, _tetter::_size_t current_index, typename... optional_all_types> typename iterator, typename... args>
	inline static constexpr _tetter_decltype(_tetter::_try_call_bool_impl<_tetter::_wrapper<iterator>, _tetter::_tetter<args...>, _tetter::_enable_t<>, ts...>::call_none(_tetter::_declval<args>()...)) 
		call_none(args&&... _args)
	{
		return _tetter::_try_call_bool_impl<_tetter::_wrapper<iterator>, _tetter::_tetter<args...>, _tetter::_enable_t<>, ts...>::call_none(static_cast<args&&>(_args)...);
	}

	template <template <typename current_type, _tetter::_size_t current_index, typename... optional_all_types> typename iterator, typename... args>
	inline static constexpr _tetter_decltype(_tetter::_try_call_int_impl<_tetter::_wrapper<iterator>, _tetter::_tetter<args...>, _tetter::_enable_t<>, ts...>::call_sum(_tetter::_declval<args>()...))
		call_sum(args&&... _args)
	{
		return _tetter::_try_call_int_impl<_tetter::_wrapper<iterator>, _tetter::_tetter<args...>, _tetter::_enable_t<>, ts...>::call_sum(static_cast<args&&>(_args)...);
	}
	
	template <template <typename current_type, _tetter::_size_t current_index, typename... optional_all_types> typename iterator, typename... args>
	inline static constexpr _tetter_decltype(_tetter::_try_call_int_impl<_tetter::_wrapper<iterator>, _tetter::_tetter<args...>, _tetter::_enable_t<>, ts...>::call_avg(_tetter::_declval<args>()...))
		call_avg(args&&... _args)
	{
		return _tetter::_try_call_int_impl<_tetter::_wrapper<iterator>, _tetter::_tetter<args...>, _tetter::_enable_t<>, ts...>::call_avg(static_cast<args&&>(_args)...);
	}

	template <template <typename current_type, _tetter::_size_t current_index, typename... optional_all_types> typename iterator, typename... args>
	inline static constexpr _tetter_decltype(_tetter::_try_call_int_impl<_tetter::_wrapper<iterator>, _tetter::_tetter<args...>, _tetter::_enable_t<>, ts...>::call_min(_tetter::_declval<args>()...))
		call_min(args&&... _args)
	{
		return _tetter::_try_call_int_impl<_tetter::_wrapper<iterator>, _tetter::_tetter<args...>, _tetter::_enable_t<>, ts...>::call_min(static_cast<args&&>(_args)...);
	}

	template <template <typename current_type, _tetter::_size_t current_index, typename... optional_all_types> typename iterator, typename... args>
	inline static constexpr _tetter_decltype(_tetter::_try_call_int_impl<_tetter::_wrapper<iterator>, _tetter::_tetter<args...>, _tetter::_enable_t<>, ts...>::call_max(_tetter::_declval<args>()...))
		call_max(args&&... _args)
	{
		return _tetter::_try_call_int_impl<_tetter::_wrapper<iterator>, _tetter::_tetter<args...>, _tetter::_enable_t<>, ts...>::call_max(static_cast<args&&>(_args)...);
	}

#if _tetter_unevaluated_lambda // (C++20)
	template <auto lambda_caster>
	using cast_l = decltype(lambda_caster.template operator()<ts...>());

	template <auto lambda_iterator>
	using find_l = _tetter::_try_find_impl<_tetter::_lambda_v_wrapper<lambda_iterator>, _tetter::_enable_t<>, ts...>;

	template <auto lambda_iterator>
	using filter_l = typename _tetter::_try_filter_impl<_tetter::_lambda_v_wrapper<lambda_iterator>, _tetter::_enable_t<>, ts...>::type::template cast<tetter>;

	template <auto lambda_sorter>
	using sort_l = typename _tetter::_try_sort_impl<_tetter::_lambda_sort_wrapper<lambda_sorter>, _tetter::_enable_t<>, ts...>::type::template cast<tetter>;

	template <auto lambda_iterator>
	using map_l = typename _tetter::_try_type_impl<_tetter::_lambda_t_wrapper<lambda_iterator>, _tetter::_enable_t<>, ts...>::type::template cast<tetter>;

	template <auto lambda_iterator>
	using value_l = _tetter::_try_value_impl<_tetter::_lambda_v_wrapper<lambda_iterator>, _tetter::_enable_t<>, ts...>;

	template <auto lambda_iterator>
	using value_b_l = _tetter::_try_bool_value_impl<_tetter::_lambda_v_wrapper<lambda_iterator>, _tetter::_enable_t<>, ts...>;

	template <auto lambda_iterator>
	using value_i_l = _tetter::_try_int_value_impl<_tetter::_lambda_v_wrapper<lambda_iterator>, _tetter::_enable_t<>, ts...>;
#endif

#if _tetter_generic_lambdas // (C++20)
	template <typename lambda, typename... args>
	inline static constexpr void invoke(lambda&& _lambda, args&&... _args)
	{
		call<_tetter::_lambda_impl>(static_cast<lambda&&>(_lambda), static_cast<args&&>(_args)...);
	}

	template <typename lambda, typename inital_type, typename... args>
	inline static constexpr auto invoke_pipe(lambda&& _lambda, inital_type&& inital_value = inital_type{}, args&&... _args)
	{
		return call_pipe<_tetter::_lambda_pipe_impl>(static_cast<inital_type&&>(inital_value), static_cast<lambda&&>(_lambda), static_cast<args&&>(_args)...);
	}

	template <typename lambda, typename... args>
	inline static constexpr auto invoke_all(lambda&& _lambda, args&&... _args)
	{
		return call_all<_tetter::_lambda_impl>(static_cast<lambda&&>(_lambda), static_cast<args&&>(_args)...);
	}

	template <typename lambda, typename... args>
	inline static constexpr auto invoke_any(lambda&& _lambda, args&&... _args)
	{
		return call_any<_tetter::_lambda_impl>(static_cast<lambda&&>(_lambda), static_cast<args&&>(_args)...);
	}

	template <typename lambda, typename... args>
	inline static constexpr auto invoke_none(lambda&& _lambda, args&&... _args)
	{
		return call_none<_tetter::_lambda_impl>(static_cast<lambda&&>(_lambda), static_cast<args&&>(_args)...);
	}

	template <typename lambda, typename... args>
	inline static constexpr auto invoke_sum(lambda&& _lambda, args&&... _args)
	{
		return call_sum<_tetter::_lambda_impl>(static_cast<lambda&&>(_lambda), static_cast<args&&>(_args)...);
	}

	template <typename lambda, typename... args>
	inline static constexpr auto invoke_avg(lambda&& _lambda, args&&... _args)
	{
		return call_avg<_tetter::_lambda_impl>(static_cast<lambda&&>(_lambda), static_cast<args&&>(_args)...);
	}

	template <typename lambda, typename... args>
	inline static constexpr auto invoke_min(lambda&& _lambda, args&&... _args)
	{
		return call_min<_tetter::_lambda_impl>(static_cast<lambda&&>(_lambda), static_cast<args&&>(_args)...);
	}

	template <typename lambda, typename... args>
	inline static constexpr auto invoke_max(lambda&& _lambda, args&&... _args)
	{
		return call_max<_tetter::_lambda_impl>(static_cast<lambda&&>(_lambda), static_cast<args&&>(_args)...);
	}

	template <typename lambda, typename... args>
	inline static constexpr auto cast_invoke(lambda&& _lambda, args&&... _args)
	{
		return static_cast<lambda&&>(_lambda).template operator()<ts...>(static_cast<args&&>(_args)...);
	}
#endif
};

template <typename t>
using tetter_from = typename _tetter::_from_impl<typename _tetter::_clean<t>::type>::type::template cast<tetter>;

template <typename function>
using tetter_from_args = typename _tetter::_from_args_impl<typename _tetter::_clean<function>::type, _tetter::_enable_t<>>::type::template cast<tetter>;

template <_tetter::_size_t n, template <_tetter::_size_t> typename sequence_type = _tetter::_size_t_sequence>
using tetter_sequence = typename _tetter::_tetter_sequence_impl<n, sequence_type, _tetter::_tetter<>>::type::template cast<tetter>;

template <typename t>
struct is_tetter
{
	static constexpr bool value = false;
};

template <typename... ts>
struct is_tetter<tetter<ts...>>
{
	static constexpr bool value = true;
};

#if _tetter_variable_templates // (C++14)
template <typename t>
inline static constexpr bool is_tetter_v = is_tetter<t>::value;
#endif

#if _tetter_concepts // (C++20)
template <typename t>
concept tetter_c = is_tetter_v<t>;
#endif

#endif

#include <variant>
#include <vector>

_ARGLESS_CORE_BEGIN

template <typename char_t>
struct expected
{
	constexpr inline expected(const char_t* type_name) : m_type_name(type_name) {}

public:
	constexpr inline const char_t* what() const { return m_type_name; }

private:
	const char_t* m_type_name;
};

template <typename t>
struct make_default
{
	using type = t;
	t m_value;
};

template <typename t>
make_default(t&&) -> make_default<t&&>;

template <typename t>
struct is_make_default : public std::false_type {};

template <typename t>
struct is_make_default<make_default<t>> : public std::true_type {};

template <typename t>
struct _expected_t;

template <typename t>
constexpr inline _expected_t<t> make_expected;

template <typename t, typename char_t>
struct parse_result
{
	constexpr inline parse_result(parse_result<t, char_t>&& value) : m_value(std::move(value.m_value)), m_default_constructed(value.m_default_constructed) {}

	template <typename other_t>
	constexpr inline parse_result(parse_result<other_t, char_t>&& value) : m_default_constructed(value.m_default_constructed) 
	{
		if (value.is_valid())
			m_value = t(std::move(value).get());
		else
			m_value = value.expected();
	}

	constexpr inline parse_result(t&& value)
	{
		m_value = std::move(value);
		m_default_constructed = false;
	}

	constexpr inline parse_result(const t& value)
	{
		m_value = value;
		m_default_constructed = false;
	}

	template <typename default_t>
	constexpr inline parse_result(make_default<default_t>&& value)
	{
		m_value = static_cast<default_t>(value.m_value);
		m_default_constructed = true;
	}

	constexpr inline parse_result(const _ARGLESS_CORE expected<char_t>& value)
	{
		m_value = value;
		m_default_constructed = false;
	}

	template <typename u>
	constexpr inline parse_result(const _expected_t<u>&) : parse_result(_ARGLESS_CORE expected<char_t>(_expected_t<u>::template name<char_t>)) {}

public:
	constexpr inline bool is_valid() const noexcept { return std::holds_alternative<t>(m_value); } 
	constexpr inline bool is_valid_no_default() const noexcept { return std::holds_alternative<t>(m_value) && !is_default(); } 

	constexpr inline bool is_default() const noexcept { return m_default_constructed; } 
	constexpr inline parse_result& make_default() & { m_default_constructed = true; return *this; }
	constexpr inline parse_result&& make_default() && { m_default_constructed = true; return *this; }

	constexpr inline t& get() & { return std::get<t>(m_value); }
	constexpr inline t&& get() && { return std::get<t>(std::move(m_value)); }

	constexpr inline _ARGLESS_CORE expected<char_t>& expected() & { return std::get<_ARGLESS_CORE expected<char_t>>(m_value); }
	constexpr inline _ARGLESS_CORE expected<char_t>&& expected() && { return std::get<_ARGLESS_CORE expected<char_t>>(std::move(m_value)); }

private:
	template <typename, typename>
	friend struct parse_result;
	std::variant<std::monostate, _ARGLESS_CORE expected<char_t>, t> m_value;
	bool m_default_constructed = false;
};

template <typename char_t>
struct args
{
	struct arg
	{
		const char_t* m_value;
		std::size_t m_arg = 0;
	};

	inline args(std::vector<arg>&& args) : m_args(std::move(args)), m_index(0), m_limit(m_args.size()), m_force(false) {}

public:
	constexpr inline const char_t* const* peak() const
	{
		if (m_index < m_args.size() && m_index < m_limit && (!m_args[m_index].m_arg || m_force))
			return &m_args[m_index].m_value;
		return nullptr;
	}

	constexpr inline void consume() { ++m_index; }

public:
	std::vector<arg> m_args;
	size_t m_index = 0;
	size_t m_limit = 0;
	bool m_force = false;
};

template <typename t>
struct parser
{
	static_assert(false, "type t is not supported (u can template specialize argless::core::parser to support type t)");

	/* 
	using type = t;

	template <typename char_t>
	static inline parse_result<type, char_t> parse(args<char_t>& args);

	template <typename char_t>
	static constexpr inline _ARGLESS_CORE str<char_t, x> name;
	*/
};

template <>
struct parser<void>
{
	using type = void;

	template <typename char_t>
	static constexpr inline auto name = str_from<char_t, "<>">();
};

_ARGLESS_CORE_END
_ARGLESS_BEGIN

template <typename t, typename char_t = char>
inline consteval const char_t* get_name()
{
	return _ARGLESS_CORE parser<t>::template name<char_t>;
}

_ARGLESS_END
_ARGLESS_CORE_BEGIN

template <typename t>
struct _expected_t
{
	template <typename char_t>
	static constexpr inline auto name = get_name<t, char_t>();
};

template <typename t, typename char_t>
inline consteval auto type_name()
{
	return _ARGLESS_CORE parser<t>::template name<char_t>;
}

template <auto str>
inline consteval auto wrapped_name()
{
	using char_t = typename decltype(str)::char_t;
	if constexpr (str.size() == 1 || (str.m_buffer[0] != static_cast<char_t>('<') || str.m_buffer[str.size()-2] != static_cast<char_t>('>')))
	{
		_ARGLESS_CORE str<char_t, str.size() + 2> result;
		result.m_buffer[0] = static_cast<char_t>('<');
		for (std::size_t i = 0; i < str.size() - 1; ++i) result.m_buffer[1+i] = str.m_buffer[i];
		result.m_buffer[result.size() - 2] = static_cast<char_t>('>');
		result.m_buffer[result.size() - 1] = static_cast<char_t>('\0');
		return result;
	}
	else return str;
}

template <auto str>
inline consteval auto optional_name()
{
	using char_t = typename decltype(str)::char_t;
	if constexpr (str.size() == 3 && str.m_buffer[0] == static_cast<char_t>('<') && str.m_buffer[1] == static_cast<char_t>('>'))
		return str;
	if constexpr (str.size() == 1 || str.m_buffer[str.size()-2] != static_cast<char_t>('?'))
	{
		auto str_ = wrapped_name<str>();
		_ARGLESS_CORE str<char_t, str_.size() + 1> result;
		for (std::size_t i = 0; i < str_.size() - 1; ++i) result.m_buffer[i] = str_.m_buffer[i];
		result.m_buffer[result.size() - 2] = static_cast<char_t>('?');
		result.m_buffer[result.size() - 1] = static_cast<char_t>('\0');
		return result;
	}
	else return str;
}

template <auto str>
inline consteval auto array_wrap_name()
{
	using char_t = typename decltype(str)::char_t;
	if constexpr (str.size() == 1 || str.m_buffer[str.size()-2] != static_cast<char_t>(']')) return wrapped_name<str>();
	else return str;
}

/* struct _name_info
{
	enum class wrap {
		none,
		and_wrapped,
		or_wrapped,
		wrapped,
		void_wrap,
	} m_wrap = wrap::none;
	bool m_optional = false;
};

template <auto str>
inline consteval auto name_info()
{
	using char_t = typename decltype(str)::char_t;
} */

template <auto lstr, auto rstr>
	requires std::is_same_v<typename decltype(lstr)::char_t, typename decltype(rstr)::char_t> 
inline consteval auto or_name()
{
	// TODO:
	// <..> + ... -> <..|...>
	// <..>? + <...> -> <..|...>?
	// .. + <> -> ..
	// <..|...> + .... -> <..|...|....>
	// <.., ...> + <....|.....> -> <<.., ...>|....|.....>

	using char_t = typename decltype(lstr)::char_t;
	return wrapped_name<lstr + str_from<char_t, "|">() + rstr>();
}

template <auto lstr, auto rstr>
	requires std::is_same_v<typename decltype(lstr)::char_t, typename decltype(rstr)::char_t> 
inline consteval auto and_name()
{
	// TODO:
	// <..> + ... -> <.., ...>
	// <..>? + <...> -> <.., ...>?
	// .. + <> -> ..
	// <.., ...> + .... -> <.., ..., ....>
	// <..|...> + <...., .....> -> <<..|...>, ...., .....>

	using char_t = typename decltype(lstr)::char_t;
	return wrapped_name<lstr + str_from<char_t, ", ">() + rstr>();
}

template <typename char_t, std::size_t n>
inline consteval auto number_name() 
{
	str<char_t, []() { 
		std::size_t v = n, c = 1;
		while (v >= 10) ++c, v /= 10;
		return c + 1;
	}()> str_;

	constexpr char_t digits[] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', };

	std::size_t v = n;
	for (std::size_t i = str_.size() - 2; v >= 10; --i, v /= 10)
		str_.m_buffer[i] = static_cast<char_t>(digits[v%10]); 
	str_.m_buffer[0] = static_cast<char_t>(digits[v%10]);
	str_.m_buffer[str_.size() - 1] = static_cast<char_t>('\0');

	return str_;
}

_ARGLESS_CORE_END

#include <optional>
#include <vector>
#include <type_traits>

_ARGLESS_CORE_BEGIN

template <typename t>
concept parsable = (!std::is_reference_v<t>) && requires { typename parser<t>; };

template <typename t, typename value_type>
concept containerable = std::is_default_constructible_v<t> && (std::is_void_v<t> ?
		requires (t c) { ++c; } : (
		requires (t c, value_type&& v) { c.push_back(std::move(v)); } ||
		requires (t c, value_type&& v) { c.push(std::move(v)); } ||
		requires (t c, value_type&& v) { c.insert(std::move(v)); } ||
		requires (t c) { ++c; }
	));

template <typename... values>
concept no_name_collision = tetter<values...>::template value_l<[]<typename t, std::size_t i>() {
		return tetter<values...>::template pop_front_n<i + 1>::template value_l<[]<typename st, std::size_t>() {
			return _ARGLESS_CORE seq(t::value.data(), st::value.data()); 
		}>::any;
	}>::none;

template <bool derivable, typename t>
struct _derive_impl;

template <typename t>
struct _derive_impl<true, t> : public t
{
	using t::t;
	using t::operator=;

	inline constexpr _derive_impl(const t& v) : t(v) {}
	inline constexpr _derive_impl(t&& v) : t(std::move(v)) {}
	inline constexpr _derive_impl(const _derive_impl&) = default;
	inline constexpr _derive_impl(_derive_impl&&) = default;
	inline constexpr _derive_impl& operator=(const _derive_impl&) = default;
	inline constexpr _derive_impl& operator=(_derive_impl&&) = default;
};

template <typename t>
struct _derive_impl<false, t>
{
	inline constexpr _derive_impl() {}
	inline constexpr _derive_impl(t value) : m_value(value) {}

	inline constexpr _derive_impl& operator=(t value) { m_value = value; return *this; }

	inline constexpr _derive_impl(const _derive_impl&) = default;
	inline constexpr _derive_impl(_derive_impl&&) = default;

	inline constexpr _derive_impl& operator=(const _derive_impl&) = default;
	inline constexpr _derive_impl& operator=(_derive_impl&&) = default;

	inline constexpr operator t&() { return m_value; }
	inline constexpr operator const t&() const { return m_value; }

	inline constexpr auto& pure_cast() { return m_value; }
	inline constexpr const auto& pure_cast() const { return m_value; }

private:
	t m_value;
};

template <typename t>
using derive = _derive_impl<std::is_class_v<t>, t>;

_ARGLESS_CORE_END



_ARGLESS_BEGIN

template <_ARGLESS_CORE parsable t, _ARGLESS_CORE containerable<t> container = 
	std::conditional_t<std::is_void_v<t>, std::size_t, std::vector<t>>>
struct accumulate : public _ARGLESS_CORE derive<container>
{
	using _ARGLESS_CORE derive<container>::derive;
	using _ARGLESS_CORE derive<container>::operator=;

	inline constexpr operator bool() const
	{
		if constexpr (requires (container c) { static_cast<bool>(c); })
			return _ARGLESS_CORE derive<container>::operator bool();
		else
			return !this->empty();
	}
};

template <_ARGLESS_CORE parsable t>
struct required {};

template <_ARGLESS_CORE parsable t, auto value_or_lambda>
	requires std::is_invocable_r_v<t, decltype(value_or_lambda)> ||
		std::is_convertible_v<decltype(value_or_lambda), typename _ARGLESS_CORE parser<t>::type>
struct default_value { static constexpr inline auto value = value_or_lambda; };


template <_ARGLESS_CORE parsable t, _ARGLESS_CORE str new_name>
struct rename : public _ARGLESS_CORE derive<t>
{
	using _ARGLESS_CORE derive<t>::derive;
	using _ARGLESS_CORE derive<t>::operator=;
};

template <_ARGLESS_CORE parsable t>
struct force : public _ARGLESS_CORE derive<t>
{
	using _ARGLESS_CORE derive<t>::derive;
	using _ARGLESS_CORE derive<t>::operator=;
};

template <_ARGLESS_CORE parsable t, auto value_or_lambda>
	requires std::is_invocable_r_v<t, decltype(value_or_lambda)> ||
		std::is_convertible_v<decltype(value_or_lambda), typename _ARGLESS_CORE parser<t>::type>
struct fallback : public _ARGLESS_CORE derive<t>
{
	using _ARGLESS_CORE derive<t>::derive;
	using _ARGLESS_CORE derive<t>::operator=;
};

template <auto lambda>
	requires requires { typename tetter_from_args<decltype(lambda)>::template get<1>; } &&
		_ARGLESS_CORE parsable<std::remove_cvref_t<typename tetter_from_args<decltype(lambda)>::template get<1>>> &&
		std::is_invocable_v<decltype(lambda), std::remove_cvref_t<typename tetter_from_args<decltype(lambda)>::template get<1>>&&>
struct transform : public _ARGLESS_CORE derive<std::invoke_result_t<decltype(lambda), std::remove_cvref_t<typename tetter_from_args<decltype(lambda)>::template get<1>>&&>>
{
	using _ARGLESS_CORE derive<std::invoke_result_t<decltype(lambda), std::remove_cvref_t<typename tetter_from_args<decltype(lambda)>::template get<1>>&&>>::derive;
	using _ARGLESS_CORE derive<std::invoke_result_t<decltype(lambda), std::remove_cvref_t<typename tetter_from_args<decltype(lambda)>::template get<1>>&&>>::operator=;
};

template <auto lambda>
	requires requires { typename tetter_from_args<decltype(lambda)>::template get<1>; } &&
		_ARGLESS_CORE parsable<std::remove_cvref_t<typename tetter_from_args<decltype(lambda)>::template get<1>>> &&
		std::is_invocable_r_v<bool, decltype(lambda), std::remove_cvref_t<typename tetter_from_args<decltype(lambda)>::template get<1>>&&>
struct validate : public _ARGLESS_CORE derive<std::remove_cvref_t<typename tetter_from_args<decltype(lambda)>::template get<1>>>
{
	using _ARGLESS_CORE derive<std::remove_cvref_t<typename tetter_from_args<decltype(lambda)>::template get<1>>>::derive;
	using _ARGLESS_CORE derive<std::remove_cvref_t<typename tetter_from_args<decltype(lambda)>::template get<1>>>::operator=;
};

template <_ARGLESS_CORE str... names>
	requires _ARGLESS_CORE no_name_collision<std::integral_constant<decltype(names), names>...>
struct option
{
private:
	template <_ARGLESS_CORE str name>
	static constexpr inline auto name_index = [](){
		return tetter<std::integral_constant<decltype(names), names>...>::template find_l<[]<typename t, std::size_t>() {
			return _ARGLESS_CORE seq(t::value.data(), name.data());
		}>::index;
	}();

public:
	inline constexpr option() {}
	inline constexpr option(std::size_t index) : m_index(index + 1) {}

	template <_ARGLESS_CORE str name>
		requires (name_index<name> != sizeof...(names))
	inline constexpr bool is()
	{
		return m_index == name_index<name> + 1;
	}

private:
	std::size_t m_index = 0;
};

_ARGLESS_END



_ARGLESS_CORE_BEGIN

template <typename t>
struct result_cast_impl
{
	using type = std::optional<t>;
};

template <>
struct result_cast_impl<void>
{
	using type = bool;
};

template <typename t, typename c>
struct result_cast_impl<accumulate<t, c>>
{
	using type = accumulate<t, c>;
};

template <typename t>
struct result_cast_impl<required<t>>
{
	using type = std::optional<t>;
};

template <typename t, auto v>
struct result_cast_impl<default_value<t, v>>
{
	using type = std::optional<t>;
};

// result-type for non-parse types
template <typename t>
using result_cast = typename result_cast_impl<t>::type;

struct void_noname {};

template <typename t>
using result_cast_noname = std::conditional_t<std::is_void_v<t>, void_noname, result_cast<t>>;

template <typename t>
struct parse_cast_impl
{
	using type = t;
};

template <typename t, typename c>
struct parse_cast_impl<accumulate<t, c>>
{
	using type = t;
};

template <typename t>
struct parse_cast_impl<required<t>>
{
	using type = t;
};

template <typename t, auto v>
struct parse_cast_impl<default_value<t, v>>
{
	using type = t;
};

// parsable-type for non-parse types
template <typename t>
using parse_cast = typename parse_cast_impl<t>::type;

template <typename t>
struct result_insert
{
	static inline constexpr bool call(result_cast<t>& result, parse_cast<t>&& value) // TODO: maybe do value auto
	{
		if (result) return false;
		result = std::move(value);
		return true;
	}
};

template <typename t, typename c>
struct result_insert<accumulate<t, c>>
{
	static inline constexpr bool call(accumulate<t, c>& result, t&& value)
	{
		if constexpr (requires (c r, t&& v) { r.push_back(std::move(v)); })
			result.push_back(std::move(value));
		else if constexpr (requires (c r, t&& v) { r.push(std::move(v)); })
			result.push(std::move(value));
		else if constexpr (requires (c r, t&& v) { r.insert(std::move(v)); })
			result.insert(std::move(value));
		else if constexpr (requires (c r) { ++r; })
			++result;
		else
			static_assert(false);

		return true;
	}
};

template <>
struct result_insert<void>
{
	static inline constexpr bool call(bool& result)
	{
		if (result) return false;
		result = true;
		return true;
	}
};

template <typename c>
struct result_insert<accumulate<void, c>>
{
	static inline constexpr bool call(c& result)
	{
		if constexpr (requires (c r) { ++r; })
			++result;
		else
			static_assert(false);

		return true;
	}
};

template <typename t>
struct result_get
{
	static inline constexpr auto&& call(auto&& value) { return std::forward<decltype(value)>(value); }
};

template <typename t>
struct result_get<required<t>>
{
	static inline constexpr auto&& call(auto&& value)
	{
		ARGLESS_ASSERT(static_cast<bool>(value), "you didnt check argless::result<argless::app<...>>{}.error()");
		return *std::forward<decltype(value)>(value);
	}
};

template <typename t, auto v>
struct result_get<default_value<t, v>>
{
	static inline constexpr auto&& call(auto&& value)
	{
		ARGLESS_ASSERT(static_cast<bool>(value), "you didnt check argless::result<argless::app<...>>{}.error()");
		return *std::forward<decltype(value)>(value);
	}
};

template <_ARGLESS_CORE str... names>
struct result_find;

template <_ARGLESS_CORE str name, _ARGLESS_CORE str... names>
struct result_find<name, names...>
{
	static inline auto&& call(auto&& self)
	{
		using This = std::remove_cvref_t<decltype(self)>;
		using Values = decltype([](){
				if constexpr (requires { typename This::app; })
					return typename This::app::args{};
				else
					return typename This::__todo_for_page{};
			}());
		static constexpr auto arg_index = [](){
			return Values::template find_l<[]<typename t>() {
				return _ARGLESS_CORE seq(t::name.data(), name.data()) ||
					tetter_sequence<tetter_from<decltype(t::aliases)>::count>::template value_l<[]<std::size_t i>() {
						return _ARGLESS_CORE seq(std::get<i>(t::aliases).data(), name.data());
					}>::any; 
			}>::index;
		}();
		static_assert(arg_index != Values::count, "missing arg name");

		if constexpr (sizeof...(names))
			return _ARGLESS_CORE result_get<typename Values::template get<arg_index>::type>::call(std::get<arg_index>(std::forward<decltype(self)>(self).m_values)).template get<names...>();
		else
			return _ARGLESS_CORE result_get<typename Values::template get<arg_index>::type>::call(std::get<arg_index>(std::forward<decltype(self)>(self).m_values).m_value);
	}
};

template <>
struct result_find<>
{
	static inline auto&& call(auto&& self)
	{
		using This = std::remove_cvref_t<decltype(self)>;
		return _ARGLESS_CORE result_get<typename This::app::noname_arg_type>::call(self.m_noname_value);
	}
};

template <typename t>
struct is_required : public std::false_type {};

template <typename t>
struct is_required<required<t>> : public std::true_type {};

template <typename t>
struct is_default_value : public std::false_type {};

template <typename t, auto v>
struct is_default_value<default_value<t, v>> : public std::true_type {};

template <typename... args>
concept no_name_or_alias_collision = (tetter<args...>::template map_l<
		[]<typename t, std::size_t>() -> typename tetter<std::integral_constant<decltype(t::name), t::name>>::template join<
			typename tetter_sequence<tetter_from<decltype(t::aliases)>::count>::template map_l<
				[]<std::size_t i>() -> std::integral_constant<decltype(std::get<i>(t::aliases)), std::get<i>(t::aliases)> {}
			>
		> {}
	>::concat_a::template value_l<[]<typename t, std::size_t i, typename... ts>() {
		return tetter<ts...>::template pop_front_n<i + 1>::template value_l<[]<typename st>() {
			return _ARGLESS_CORE seq(t::value.data(), st::value.data()); 
		}>::any;
	}>::none);

_ARGLESS_CORE_END
_ARGLESS_BEGIN

template <_ARGLESS_CORE parsable, _ARGLESS_CORE str, _ARGLESS_CORE str, _ARGLESS_CORE str...>
struct arg;

_ARGLESS_END
_ARGLESS_CORE_BEGIN

template <typename t>
struct is_arg : public std::false_type {};

template <typename t, _ARGLESS_CORE str name, _ARGLESS_CORE str desc, _ARGLESS_CORE str... aliases>
struct is_arg<arg<t, name, desc, aliases...>> : public std::true_type {};

template <typename t>
concept arg_t = is_arg<t>::value;

template <typename t>
concept opt_arg_t = is_arg<t>::value || std::is_same<t, void>::value;

_ARGLESS_CORE_END
_ARGLESS_BEGIN

template <_ARGLESS_CORE str, _ARGLESS_CORE str, _ARGLESS_CORE parsable, _ARGLESS_CORE arg_t... args_>
	requires _ARGLESS_CORE no_name_or_alias_collision<args_...>
struct app;

_ARGLESS_END
_ARGLESS_CORE_BEGIN

template <typename t>
struct is_app : public std::false_type {};

template <_ARGLESS_CORE str name, _ARGLESS_CORE str desc, _ARGLESS_CORE parsable noname_arg_type, _ARGLESS_CORE arg_t... args_>
struct is_app<app<name, desc, noname_arg_type, args_...>> : public std::true_type {};

template <typename t>
concept app_t = is_app<t>::value;

_ARGLESS_CORE_END











_ARGLESS_BEGIN

constexpr inline auto nodesc = _ARGLESS_CORE str("");

template <_ARGLESS_CORE parsable type_, _ARGLESS_CORE str name_, _ARGLESS_CORE str desc_ = nodesc, _ARGLESS_CORE str... aliases_>
struct arg
{
public:
	using type = type_;
	static constexpr auto name = name_;
	static constexpr auto desc = desc_;
	static constexpr auto aliases = std::make_tuple(aliases_...);

private:
	template <_ARGLESS_CORE str...>
	friend struct _ARGLESS_CORE result_find;

	template <_ARGLESS_CORE str, _ARGLESS_CORE str, _ARGLESS_CORE parsable, _ARGLESS_CORE arg_t... args_>
		requires _ARGLESS_CORE no_name_or_alias_collision<args_...>
	friend struct app;

	_ARGLESS_CORE result_cast<type> m_value;
};


enum class result_error_type 
{
	none = 0,
	unknown,
	stray_value,
	ambiguous_arg_value,
	invalid_arg_value,
	missing_arg,
};

template <typename char_t>
struct result_error
{
public:
	inline constexpr result_error() = default;
	inline constexpr result_error(result_error_type type) : m_type(type) {}

public:
	inline constexpr result_error_type type() const { return m_type; }
	inline constexpr operator bool() const { return m_type != result_error_type::none; }

	inline constexpr std::size_t where_arg() const { return m_arg_index; }
	inline constexpr const char_t* what_arg() const { return m_arg_name; }
	inline constexpr const char_t* what_arg_type() const { return m_arg_type; }
	inline constexpr std::size_t where() const { return m_index; }
	inline constexpr const char_t* what_type() const { return m_expected_type; }

private:
	template <_ARGLESS_CORE str, _ARGLESS_CORE str, _ARGLESS_CORE parsable, _ARGLESS_CORE arg_t... args_>
		requires _ARGLESS_CORE no_name_or_alias_collision<args_...>
	friend struct app;

	result_error_type m_type = result_error_type::none; 

	std::size_t m_arg_index = 0;
	const char_t* m_arg_name = nullptr;
	const char_t* m_arg_type = nullptr;
	std::size_t m_index = 0;
	const char_t* m_expected_type = nullptr;
};

template <_ARGLESS_CORE app_t app_, typename char_t>
struct result
{
public:
	using app = app_;

public:
	inline result() = default;
	inline result(const result&) = default;
	inline result(result&&) = default;
	inline result& operator=(const result&) = default;
	inline result& operator=(result&&) = default;
	inline ~result() = default;

public:
	inline const char_t* const& path() const { return m_path; };

public:
	template <_ARGLESS_CORE str... names>
	inline auto& get() & { return _ARGLESS_CORE result_find<names...>::call(*this); }

	template <_ARGLESS_CORE str... names>
	inline auto&& get() && { return _ARGLESS_CORE result_find<names...>::call(*this); }

	template <_ARGLESS_CORE str... names>
	inline const auto& get() const & { return _ARGLESS_CORE result_find<names...>::call(*this); }

	template <_ARGLESS_CORE str... names>
	inline const auto&& get() const && { return _ARGLESS_CORE result_find<names...>::call(*this); }

public:
	inline const result_error<char_t>& error() const { return m_error; }

private:
	template <_ARGLESS_CORE str, _ARGLESS_CORE str, _ARGLESS_CORE parsable, _ARGLESS_CORE arg_t... args_>
		requires _ARGLESS_CORE no_name_or_alias_collision<args_...>
	friend struct _ARGLESS app;

	template <_ARGLESS_CORE str...>
	friend struct _ARGLESS_CORE result_find;

	const char_t* m_path = nullptr;
	result_error<char_t> m_error;

	_ARGLESS_CORE result_cast_noname<typename app::noname_arg_type> m_noname_value;
	typename app::args::template cast<std::tuple> m_values;
};


template <_ARGLESS_CORE str name_, _ARGLESS_CORE str desc_, _ARGLESS_CORE parsable noname_arg_type_, _ARGLESS_CORE arg_t... args_>
	requires _ARGLESS_CORE no_name_or_alias_collision<args_...>
struct app
{
public:
	static constexpr auto name = name_;
	static constexpr auto desc = desc_;
	using noname_arg_type = noname_arg_type_;
	using args = tetter<args_...>;

public:
	template <typename char_t>
	static result<app, char_t> parse(int argc, const char_t** argv)
	{
		_ARGLESS_CORE args<char_t> args = [&](){
			std::vector<typename _ARGLESS_CORE args<char_t>::arg> _args;
			_args.reserve(argc);
			for (decltype(argc) i = 0; i < argc; ++i)
				_args.emplace_back(argv[i],
					args::invoke_pipe([&]<typename t, std::size_t t_i>(std::size_t index) {
						if (index) return index;
						return (_ARGLESS_CORE seq(t::name.data(), argv[i]) ||
							tetter_sequence<tetter_from<decltype(t::aliases)>::count>::invoke_any([&]<typename, std::size_t index>() {
								return _ARGLESS_CORE seq(std::get<index>(t::aliases).data(), argv[i]);
							})
						) ? t_i + 1 : 0;
					}, 0)
				);
			return _args;
		}();
		result<app, char_t> result;
		result.m_error.m_type = result_error_type::unknown;

		if (auto path = args.peak())
			result.m_path = (args.consume(), *path);

		while (args.m_index < args.m_limit)
		{
			auto& arg_index = args.m_index;
			auto& arg = args.m_args[arg_index];

			auto parse = [&]<typename t, std::size_t i>() -> bool
			{
				using type = _ARGLESS_CORE parse_cast<t>;
				auto insert = [&](auto&&... as) -> bool {
					if (!_ARGLESS_CORE result_insert<t>::call(std::forward<decltype(as)>(as)...))
					{
						result.m_error.m_type = result_error_type::ambiguous_arg_value;
						result.m_error.m_arg_index = arg_index;
						if constexpr (i) result.m_error.m_arg_name = _ARGLESS_CORE static_str<_ARGLESS_CORE str_cast<char_t, args::template get<i-1>::name>()>;
						result.m_error.m_arg_type = get_name<type, char_t>();
						return true;
					}
					return false;
				};

				if constexpr (std::is_void_v<type>)
				{
					if constexpr (i)
						return insert(std::get<i - 1>(result.m_values).m_value);
					else
						return insert(result.m_noname_value);
				}
				else
				{
					auto presult = _ARGLESS_CORE parser<type>::template parse<char_t>(args);

					if (!presult.is_valid())
					{
						result.m_error.m_type = result_error_type::invalid_arg_value;
						result.m_error.m_arg_index = arg_index;
						if constexpr (i) result.m_error.m_arg_name = _ARGLESS_CORE static_str<_ARGLESS_CORE str_cast<char_t, args::template get<i-1>::name>()>;
						result.m_error.m_arg_type = get_name<type, char_t>();
						result.m_error.m_index = args.m_index;
						result.m_error.m_expected_type = presult.expected().what();
						return true;
					}

					if constexpr (i)
						return insert(std::get<i - 1>(result.m_values).m_value, std::move(presult).get());
					else
						return insert(result.m_noname_value, std::move(presult).get());
				}

				return false;
			};

			if (arg.m_arg)
			{
				args.consume();
				if (args::invoke_none([&]<typename t, std::size_t i>() -> bool {
					if (i != arg.m_arg - 1) return false;
					if (parse.template operator()<typename t::type, i + 1>()) return false;
					return true;
				})) return result;
			}
			else if constexpr (!std::is_void_v<noname_arg_type>)
			{
				if (parse.template operator()<noname_arg_type, 0>()) return result;
			}
			else
			{
				result.m_error.m_type = result_error_type::stray_value;
				result.m_error.m_index = arg_index;
				return result;
			}
		}

		if (args::invoke_any([&]<typename t, std::size_t i>() -> bool {
			if constexpr (_ARGLESS_CORE is_required<typename t::type>::value)
			{
				using type = typename t::type;
				auto& value = std::get<i>(result.m_values).m_value;
				if (!static_cast<bool>(value))
				{
					result.m_error.m_type = result_error_type::missing_arg;
					result.m_error.m_arg_name = _ARGLESS_CORE static_str<_ARGLESS_CORE str_cast<char_t, t::name>()>;
					result.m_error.m_arg_type = get_name<_ARGLESS_CORE parse_cast<type>, char_t>();
					return true;
				}
			}
			else if constexpr(_ARGLESS_CORE is_default_value<typename t::type>::value)
			{
				using type = typename t::type;
				auto& value = std::get<i>(result.m_values).m_value;
				if (!value)
				{
					if constexpr (std::is_convertible_v<decltype(type::value), typename _ARGLESS_CORE parser<_ARGLESS_CORE parse_cast<type>>::type>) 
						value = type::value;
					else
						value = type::value();
				}
			}
			return false;
		})) return result;

		result.m_error.m_type = result_error_type::none;
		return result;
	}
};

_ARGLESS_END










_ARGLESS_CORE_BEGIN

template <typename t, _ARGLESS_CORE str new_name>
struct parser<rename<t, new_name>>
{
	using type = rename<t, new_name>;

	template <typename char_t>
	static inline parse_result<type, char_t> parse(args<char_t>& args) { return parser<t>::parse(args); }

	template <typename char_t>
	static constexpr inline auto name = _ARGLESS_CORE str_cast<char_t, new_name>();
};

template <typename t>
struct parser<force<t>>
{
	using type = force<t>;

	template <typename char_t>
	static inline parse_result<type, char_t> parse(args<char_t>& args)
	{
		bool prev_force = args.m_force;
		args.m_force = true;
		auto result = parser<t>::parse(args);
		args.m_force = prev_force;
		return result;
	}

	template <typename char_t>
	static constexpr inline auto name = parser<t>::template name<char_t>;
};

template <typename t, auto value>
struct parser<fallback<t, value>>
{
	using type = fallback<t, value>;

	template <typename char_t>
	static inline parse_result<type, char_t> parse(args<char_t>& args)
	{
		auto result = parser<t>::template parse<char_t>(args);
		if (!result.is_valid())
		{
			if constexpr (std::is_invocable_v<decltype(value)>) return t(value());
			else return t(value);
		}

		return result;
	}

	template <typename char_t>
	static constexpr inline auto name = parser<t>::template name<char_t>;
};

template <auto lambda>
struct parser<transform<lambda>>
{
	using t = std::remove_cvref_t<typename tetter_from_args<decltype(lambda)>::template get<1>>;
	using type = std::invoke_result_t<decltype(lambda), t&&>;

	template <typename char_t>
	static inline parse_result<type, char_t> parse(args<char_t>& args)
	{
		auto result = parser<t>::template parse<char_t>(args);
		if (!result.is_valid())
			return result.expected();

		return lambda(std::move(result).get());
	}

	template <typename char_t>
	static constexpr inline auto name = parser<t>::template name<char_t>;
};

template <auto lambda>
struct parser<validate<lambda>>
{
	using type = std::remove_cvref_t<typename tetter_from_args<decltype(lambda)>::template get<1>>;

	template <typename char_t>
	static inline parse_result<type, char_t> parse(args<char_t>& args)
	{
		auto result = parser<type>::template parse<char_t>(args);
		if (!result.is_valid())
			return result.expected();

		if (!lambda(result.get()))
			return make_expected<type>;

		return result;
	}

	template <typename char_t>
	static constexpr inline auto name = parser<type>::template name<char_t>;
};

template <>
struct parser<option<>>
{
	using type = option<>; 

	template <typename char_t>
	static inline parse_result<type, char_t> parse(args<char_t>& args)
	{
		return make_default(type());
	}

	template <typename char_t>
	static constexpr inline auto name = str_from<char_t, "<>">();
};

template <str... names>
struct parser<option<names...>>
{
	using type = option<names...>; 

	using vs = tetter<std::integral_constant<decltype(names), names>...>;

	template <typename char_t>
	static inline parse_result<type, char_t> parse(args<char_t>& args)
	{
		auto arg = args.peak();
		if (!arg)
			return make_expected<type>; 
		auto& value = *arg;

		return vs::pop_front::invoke_pipe([&]<typename t, std::size_t i>(parse_result<type, char_t>&& result) -> parse_result<type, char_t> {
			if (result.is_valid()) return result;
			if(seq(t::value.data(), value))
				return args.consume(), type(i + 1);
			else
				return result;
		}, [&]() -> parse_result<type, char_t> {
			using t = typename vs::front;
			if(seq(t::value.data(), value))
				return args.consume(), type(0);
			else
				return make_expected<type>;
		}());
	}

	template <typename char_t>
	static constexpr inline auto name = vs::pop_front::invoke_pipe([]<typename t>(auto str){ return [](){ return or_name<decltype(str){}(), (str_from<char_t, "'">() + str_cast<char_t, t::value>() + str_from<char_t, "'">())>(); }; },
		[](){ return str_from<char_t, "'">() + str_cast<char_t, vs::front::value>() + str_from<char_t, "'">(); })();
};

_ARGLESS_CORE_END
#include <sstream>

_ARGLESS_BEGIN

template <_ARGLESS_CORE app_t app, typename char_t>
inline constexpr std::basic_string<char_t> help_error(const result_error<char_t>& error, int argc, const char_t** argv)
{
	(void) argc;
	(void) argv;
	std::basic_stringstream<char_t> out;

	out << "error type: ";
	switch (error.type())
	{
		case argless::result_error_type::none:;
		case argless::result_error_type::unknown: out << "unknown"; break;
		case argless::result_error_type::stray_value: out << "stray_value"; break;
		case argless::result_error_type::ambiguous_arg_value: out << "ambiguous_arg_value"; break;
		case argless::result_error_type::invalid_arg_value: out << "invalid_arg_value"; break;
		case argless::result_error_type::missing_arg: out << "missing_arg"; break;
	}
	out << '\n' 

		<< "\twhat type: " << (error.what_type() ? error.what_type() : "null") << '\n'
		<< "\twhere: " << error.where() << '\n'
		<< "\twhat arg: " << (error.what_arg() ? error.what_arg() : "null") << '\n'
		<< "\twhat arg type: " << (error.what_arg_type() ? error.what_arg_type() : "null") << '\n'
		<< "\twhere arg: " << error.where_arg() << '\n';

	return out.str();
}

/*
 * ARG::NAME \(ARGS::RQUIRED ? "(Required)" : "")    \t   Type: ARG::TYPE
 * ARG::ALIASES                                      \t   ARG::DESC
 *
 */

template <_ARGLESS_CORE app_t app, typename char_t = char>
inline std::basic_string<char_t> help_arg(const char_t* arg)
{
	(void) arg;
	std::basic_stringstream<char_t> out;
	out << "TODO";
	return out.str();
}

/*
 * APP::NAME
 * \t APP:DESC
 *
 * Args: \(if 0 required)
 *
 * Required:
 * \t ARG::NAME      \t   Type: ARG::TYPE
 * \t ARG::ALIASES   \t   ARG::DESC
 *
 * Other:
 * \t ARG::NAME      \t   Type: ARG::TYPE
 * \t ARG::ALIASES   \t   ARG::DESC
 *
 */

template <_ARGLESS_CORE app_t app, typename char_t = char>
inline constexpr std::basic_string<char_t> help_app()
{
	std::basic_stringstream<char_t> out;

	out << _ARGLESS_CORE str_cast<char_t, app::name>() << '\n'; 
	out << _ARGLESS_CORE str_cast<char_t, app::desc>() << '\n';

	app::args::invoke([&]<typename t>(){
		out << '\n'; 
		out << _ARGLESS_CORE str_cast<char_t, t::name>(); 
		tetter_sequence<tetter_from<decltype(t::aliases)>::count>::invoke([&]<std::size_t i>(){
			out << _ARGLESS_CORE str_from<char_t, ", ">() << _ARGLESS_CORE str_cast<char_t, std::get<i>(t::aliases)>(); 
		});
		out << '\n' << "type: ";
		out << get_name<_ARGLESS_CORE parse_cast<typename t::type>, char_t>();
		out << '\n';
		out << _ARGLESS_CORE str_cast<char_t, t::desc>();
		out << '\n';
	});

	return out.str();
}

_ARGLESS_END
#if defined(ARGLESS_STDH_ALL)
#define ARGLESS_STDH_ARRAY
#define ARGLESS_STDH_VECTOR
#define ARGLESS_STDH_LIST
#define ARGLESS_STDH_DEQUE
#define ARGLESS_STDH_STACK
#define ARGLESS_STDH_QUEUE
#define ARGLESS_STDH_STRING
#define ARGLESS_STDH_STRING_VIEW
#define ARGLESS_STDH_FILESYSTEM
#define ARGLESS_STDH_OPTIONAL
#define ARGLESS_STDH_VARIANT
#define ARGLESS_STDH_TUPLE
#endif


_ARGLESS_CORE_BEGIN

template <>
struct parser<bool>
{
	using type = bool; 

	template <typename char_t>
	static inline parse_result<type, char_t> parse(args<char_t>& args)
	{
		auto arg = args.peak();
		if (!arg)
			return make_expected<type>; 
		auto& value = *arg;

		if (seq_nocase(value, "true"))
			return args.consume(), true;
		else if (seq_nocase(value, "false"))
			return args.consume(), false;

		return make_expected<type>; 
	}

	template <typename char_t>
	static constexpr inline auto name = str_from<char_t, "boolean">();
};

template <typename t>
	requires tetter<int, unsigned int, short, unsigned short, signed char, unsigned char, long, unsigned long, long long, unsigned long long>::template find_t<t>::value
struct parser<t>
{
	using type = t; 

	template <typename char_t>
	static inline parse_result<type, char_t> parse(args<char_t>& args)
	{
		auto arg = args.peak();
		if (!arg)
			return make_expected<type>;
		auto& value = *arg;

		auto integer = stot<type>(value);

		if (!integer)
			return make_expected<type>;

		return args.consume(), *integer;
	}

	template <typename char_t>
	static constexpr inline auto name = [](){
		if constexpr (std::is_unsigned_v<t>)
			return str_from<char_t, "+integer">();
		else
			return str_from<char_t, "integer">();
	}();
};

template <typename t>
	requires tetter<float, double, long double>::template find_t<t>::value
struct parser<t>
{
	using type = t; 

	template <typename char_t>
	static inline parse_result<type, char_t> parse(args<char_t>& args)
	{
		auto arg = args.peak();
		if (!arg)
			return make_expected<type>;
		auto& value = *arg;

		auto number = stot<type>(value);

		if (!number)
			return make_expected<type>;

		return args.consume(), *number;
	}

	template <typename char_t>
	static constexpr inline auto name = str_from<char_t, "number">();
};

template <typename t>
	requires tetter<char, wchar_t, char8_t, char16_t, char32_t>::template find_t<t>::value
struct parser<t>
{
	using type = t; 

	template <typename char_t>
	static inline parse_result<type, char_t> parse(args<char_t>& args)
	{
		auto arg = args.peak();
		if (!arg)
			return make_expected<type>; 
		const auto* value = *arg;

		if constexpr (std::is_same_v<t, char> && std::is_same_v<char_t, char>)
		{
			if (slen(value) != 1)
				return make_expected<char>;
			else
				return args.consume(), *value;
		}
		else
		{
			auto chars = get_charu<type>(value);

			// lenght check
			if (chars.m_buffer[0] == 0 || *value != 0)
				return make_expected<type>;

			// too big to cast
			if (chars.m_buffer[1] != 0)
			{
				if constexpr (sizeof(type) >= 2)
					return args.consume(), char_cast<type>(0xFFFD); 
				else
					return args.consume(), char_cast<type>(0x0);
			}

			return args.consume(), chars.m_buffer[0];
		}
	}

	template <typename char_t>
	static constexpr inline auto name = str_from<char_t, "char">();
};

template <typename t>
	requires tetter<char[4], wchar_t[4 / sizeof(wchar_t)], char8_t[4], char16_t[2], char32_t[1]>::template find_t<t>::value
struct parser<t>
{
	using type = t; 

	template <typename char_t>
	static inline parse_result<type, char_t> parse(args<char_t>& args)
	{
		auto arg = args.peak();
		if (!arg)
			return make_expected<type>; 
		const auto* value = *arg;

		auto chars = get_charu<type>(value);

		// lenght check
		if (chars.m_buffer[0] == 0 || *value != 0)
			return make_expected<type>;

		type result;
		for (size_t i = 0; i < 4; ++i)
			static_cast<unsigned char*>(result)[i] = static_cast<unsigned char*>(chars)[i];
		return args.consume(), result;
	}

	template <typename char_t>
	static constexpr inline auto name = str_from<char_t, "char">();
};

_ARGLESS_CORE_END

#if defined(ARGLESS_STDH_ARRAY) || defined(_GLIBCXX_ARRAY) || defined(_LIBCPP_ARRAY) || defined(_ARRAY_)

#include <array>

_ARGLESS_CORE_BEGIN

template <typename arr_t>
struct parser<std::array<arr_t, 0>>
{
	using type = std::array<arr_t, 0>; 

	template <typename char_t>
	static inline parse_result<type, char_t> parse(args<char_t>& args)
	{
		return make_default(type());
	}

	template <typename char_t>
	static constexpr inline auto name = str_from<char_t>("<>");
};

template <typename arr_t, std::size_t arr_n>
struct parser<std::array<arr_t, arr_n>>
{
	using type = std::array<arr_t, arr_n>; 

	template <typename char_t>
	static inline parse_result<type, char_t> parse(args<char_t>& args)
	{
		type value;

		bool all_default = true;
		for (size_t i = 0; i < value.size(); ++i)
		{
			auto result = parser<arr_t>::parse(args);
			if (!result.is_valid())
				return std::move(result).expected();

			if (!result.is_default())
				all_default = false;

			value[i] = std::move(result).get();
		}

		if (all_default)
			return make_default(std::move(value));
		else
			return value;
	}

	template <typename char_t>
	static constexpr inline auto name = array_wrap_name<type_name<arr_t, char_t>()>() + str_from<char_t, "[">() + number_name<char_t, arr_n>() + str_from<char_t, "]">();
};

_ARGLESS_CORE_END

#endif

#if defined(ARGLESS_STDH_VECTOR) || defined(_GLIBCXX_VECTOR) || defined(_LIBCPP_VECTOR) || defined(_VECTOR_) || \
	defined(ARGLESS_STDH_LIST) || defined(_GLIBCXX_LIST) || defined(_LIBCPP_LIST) || defined(_LIST_) || \
	defined(ARGLESS_STDH_DEQUE) || defined(_GLIBCXX_DEQUE) || defined(_LIBCPP_DEQUE) || defined(_DEQUE_) || \
	defined(ARGLESS_STDH_STACK) || defined(_GLIBCXX_STACK) || defined(_LIBCPP_STACK) || defined(_STACK_) || \
	defined(ARGLESS_STDH_QUEUE) || defined(_GLIBCXX_QUEUE) || defined(_LIBCPP_QUEUE) || defined(_QUEUE_)

#if defined(ARGLESS_STDH_VECTOR) || defined(_GLIBCXX_VECTOR) || defined(_LIBCPP_VECTOR) || defined(_VECTOR_)
#include <vector>
#endif

#if	defined(ARGLESS_STDH_LIST) || defined(_GLIBCXX_LIST) || defined(_LIBCPP_LIST) || defined(_LIST_)
#include <list>
#endif

#if defined(ARGLESS_STDH_DEQUE) || defined(_GLIBCXX_DEQUE) || defined(_LIBCPP_DEQUE) || defined(_DEQUE_)
#include <deque>
#endif

#if defined(ARGLESS_STDH_STACK) || defined(_GLIBCXX_STACK) || defined(_LIBCPP_STACK) || defined(_STACK_)
#include <stack>
#endif

#if defined(ARGLESS_STDH_QUEUE) || defined(_GLIBCXX_QUEUE) || defined(_LIBCPP_QUEUE) || defined(_QUEUE_)
#include <queue>
#endif

_ARGLESS_CORE_BEGIN

template <template <typename, typename> typename t, typename t_t, typename t_st>
	requires tetter<
#if defined(ARGLESS_STDH_VECTOR) || defined(_GLIBCXX_VECTOR) || defined(_LIBCPP_VECTOR) || defined(_VECTOR_)
		std::vector<t_t, t_st>,
#endif
#if	defined(ARGLESS_STDH_LIST) || defined(_GLIBCXX_LIST) || defined(_LIBCPP_LIST) || defined(_LIST_)
		std::list<t_t, t_st>,
#endif
#if defined(ARGLESS_STDH_DEQUE) || defined(_GLIBCXX_DEQUE) || defined(_LIBCPP_DEQUE) || defined(_DEQUE_)
		std::deque<t_t, t_st>,
#endif
#if defined(ARGLESS_STDH_STACK) || defined(_GLIBCXX_STACK) || defined(_LIBCPP_STACK) || defined(_STACK_)
		std::stack<t_t, t_st>,
#endif
#if defined(ARGLESS_STDH_QUEUE) || defined(_GLIBCXX_QUEUE) || defined(_LIBCPP_QUEUE) || defined(_QUEUE_)
		std::queue<t_t, t_st>,
#endif
	void>::pop_back::template find_t<t<t_t, t_st>>::value
struct parser<t<t_t, t_st>>
{
	using type = t<t_t, t_st>; 

	template <typename char_t>
	static inline parse_result<type, char_t> parse(args<char_t>& args)
	{
		type value;

		while (true)
		{
			auto result = parser<t_t>::template parse<char_t>(args);
			if (!result.is_valid_no_default())
				break;

			if constexpr (requires (type r, t_t&& v) { r.push_back(v); })
				value.push_back(std::move(result).get());
			else if constexpr (requires (type r, t_t&& v) { r.push(v); })
				value.push(std::move(result).get());
			else
				static_assert(false);
		}

		if (value.empty())
			return make_default(std::move(value));
		else
			return value;
	}

	template <typename char_t>
	static constexpr inline auto name = array_wrap_name<type_name<t_t, char_t>()>() + str_from<char_t, "[]">();
};

_ARGLESS_CORE_END

#endif
#include <type_traits>

_ARGLESS_BEGIN

template <_ARGLESS_CORE str, auto value_, bool>
	requires std::is_enum_v<decltype(value_)>
struct enum_value;

_ARGLESS_END
_ARGLESS_CORE_BEGIN

template <typename t>
struct is_enum_value : public std::false_type {};

template <auto name, auto value, bool nocase>
struct is_enum_value<enum_value<name, value, nocase>> : public std::true_type {};

template <typename t>
concept enum_value_t = is_enum_value<t>::value;

template <typename... values>
concept no_enum_name_collision = tetter<values...>::template value_l<[]<typename t, std::size_t i>() {
		return tetter<values...>::template pop_front_n<i + 1>::template value_l<[]<typename st>() {
			if constexpr (t::nocase || st::nocase)
				return _ARGLESS_CORE seq_nocase(t::name.data(), st::name.data()); 
			else
				return _ARGLESS_CORE seq(t::name.data(), st::name.data()); 
		}>::any;
	}>::none;

_ARGLESS_CORE_END




_ARGLESS_BEGIN

template <_ARGLESS_CORE str name_, auto value_, bool nocase_ = false>
	requires std::is_enum_v<decltype(value_)>
struct enum_value
{
	static constexpr auto name = name_;
	static constexpr auto value = value_;
	static constexpr auto nocase = nocase_;
};

template <_ARGLESS_CORE enum_value_t... values_>
	requires (sizeof...(values_) > 0) && _ARGLESS_CORE no_enum_name_collision<values_...>
struct enum_values
{
	using values = tetter<values_...>;
};

// Example specialization:
/*
template <>
struct argless::enum_refl<ur_enum>
{
	using values = argless::enum_values<
		argless::enum_value<"name", ur_enum::value>, ...
	>;
};
*/
template <typename t>
struct enum_refl;

_ARGLESS_END



_ARGLESS_CORE_BEGIN

template <typename t>
struct is_scoped_enum : std::integral_constant<bool, std::is_enum_v<t> && !std::is_convertible_v<t, std::underlying_type_t<t>>> {};

template <typename t>
struct is_enum_values : public std::false_type {};

template <typename... ts>
struct is_enum_values<enum_values<ts...>> : public std::true_type {};

template <typename enum_t>
	requires std::is_enum_v<enum_t>
struct parser<enum_t>
{
	static_assert(requires { typename enum_refl<enum_t>::values; } &&
			is_enum_values<typename enum_refl<enum_t>::values>::value &&
			enum_refl<enum_t>::values::values::template value_l<[]<typename t, std::size_t>(){ return std::is_same_v<std::remove_cvref_t<decltype(t::value)>, enum_t>; }>::all, 
			"this enum is not specialized (use argless::enum_refl) or used wrong type for ::values (must be argless::enum_values) or u used diffrent enum type in argless::enum_value");

	using type = enum_t; 

	using vs = typename enum_refl<enum_t>::values::values;

	template <typename char_t>
	static inline parse_result<type, char_t> parse(args<char_t>& args)
	{
		auto arg = args.peak();
		if (!arg)
			return make_expected<type>; 
		auto& value = *arg;

		auto result = vs::pop_front::invoke_pipe([&]<typename t>(parse_result<type, char_t>&& result) -> parse_result<type, char_t> {
			if (result.is_valid()) return result;
			if([&](){
				if constexpr (t::nocase)
					return seq_nocase(t::name.data(), value); 
				else
					return seq(t::name.data(), value); 
			}())
				return args.consume(), t::value;
			else
				return result;
		}, [&]() -> parse_result<type, char_t> {
			using t = typename vs::front;
			if([&](){
				if constexpr (t::nocase)
					return seq_nocase(t::name.data(), value); 
				else
					return seq(t::name.data(), value); 
			}())
				return args.consume(), t::value;
			else
				return make_expected<type>;
		}());

		if constexpr (!is_scoped_enum<enum_t>::value)
		{
			if (!result.is_valid())
			{
				auto u_result = parser<std::underlying_type_t<enum_t>>::parse(args);

				if (u_result.is_valid())
					return u_result;
			}
		}

		return result;
	}

	template <typename char_t>
	static constexpr inline auto name = [](){
		auto str = vs::pop_front::invoke_pipe([]<typename t>(auto str){ return [](){ return or_name<decltype(str){}(), (str_from<char_t, "'">() + str_cast<char_t, t::name>() + str_from<char_t, "'">())>(); }; },
				[](){ return str_from<char_t, "'">() + str_cast<char_t, vs::front::name>() + str_from<char_t, "'">(); });
		if constexpr (!is_scoped_enum<enum_t>::value)
			return or_name<decltype(str){}(), type_name<std::underlying_type_t<enum_t>, char_t>()>(); 
		else
			return str();
	}();
};

_ARGLESS_CORE_END

#if defined(ARGLESS_STDH_STRING) || defined(_GLIBCXX_STRING) || defined(_LIBCPP_STRING) || defined(_STRING_)

#include <string>

_ARGLESS_CORE_BEGIN

template <typename str_char, typename str_traits, typename str_alloc>
struct parser<std::basic_string<str_char, str_traits, str_alloc>>
{
	using type = std::basic_string<str_char, str_traits, str_alloc>; 

	template <typename char_t>
	static inline parse_result<type, char_t> parse(args<char_t>& args)
	{
		auto arg = args.peak();
		if (!arg)
			return make_expected<type>;
		auto& value = *arg;

		if constexpr (std::is_same_v<str_char, char> && std::is_same_v<char_t, char>)
			return args.consume(), type(value);
		else
		{
			std::size_t size = slen(value);

			type result;
			result.reserve(size);

			for (auto it = value; it < value + size;)
				result += get_charu<str_char>(it).data();

			return args.consume(), std::move(result);
		}
	}

	template <typename char_t>
	static constexpr inline auto name = str_from<char, "text">();
};

_ARGLESS_CORE_END

#endif

#if defined(ARGLESS_STDH_STRING_VIEW) || defined(_GLIBCXX_STRING_VIEW) || defined(_LIBCPP_STRING_VIEW) || defined(_STRING_VIEW_)

#include <string_view>

_ARGLESS_CORE_BEGIN

template <typename str_char, typename str_traits>
struct parser<std::basic_string_view<str_char, str_traits>>
{
	using type = std::basic_string_view<str_char, str_traits>; 

	template <typename char_t>
	static inline parse_result<type, char_t> parse(args<char_t>& args)
	{
		static_assert(std::same_as<char_t, str_char>, "argless parse char_t is diffrent then string_view char_t");

		auto arg = args.peak();
		if (!arg)
			return make_expected<type>; 
		auto& value = *arg;

		return args.consume(), type(value, value + slen(value));
	}

	template <typename char_t>
	static constexpr inline auto name = str_from<char, "text">();
};

_ARGLESS_CORE_END
 
#endif

_ARGLESS_CORE_BEGIN

template <typename str_char>
struct parser<const str_char*>
{
	using type = const str_char*; 

	template <typename char_t>
	static inline parse_result<type, char_t> parse(args<char_t>& args)
	{
		static_assert(std::same_as<char_t, str_char>, "argless parse char_t is diffrent then ptr view char_t");

		auto arg = args.peak();
		if (!arg)
			return make_expected<type>; 
		auto& value = *arg;

		return args.consume(), value;
	}

	template <typename char_t>
	static constexpr inline auto name = str_from<char, "text">();
};

_ARGLESS_CORE_END

#if defined(ARGLESS_STDH_FILESYSTEM) || defined(_GLIBCXX_EXPERIMENTAL_FS_PATH_H) || defined(_GLIBCXX_FILESYSTEM) || defined(_LIBCPP_FILESYSTEM) || defined(_FILESYSTEM_)

#include <filesystem>

_ARGLESS_CORE_BEGIN

template <>
struct parser<std::filesystem::path>
{
	using type = std::filesystem::path; 

	template <typename char_t>
	static inline parse_result<type, char_t> parse(args<char_t>& args)
	{
		auto arg = args.peak();
		if (!arg)
			return make_expected<type>; 
		auto& value = *arg;

		auto result = [&]() -> parse_result<type, char_t>
		{
			try
			{
				return type(value, value + slen(value));
			}
			catch (...)
			{
				return make_expected<type>;
			}
		}();

		if (result.is_valid())
			args.consume();

		return result;
	}

	template <typename char_t>
	static constexpr inline auto name = str_from<char_t, "path">();
};

_ARGLESS_CORE_END

#endif

#if defined(ARGLESS_STDH_OPTIONAL) || defined(_GLIBCXX_OPTIONAL) || defined(_LIBCPP_OPTIONAL) || defined(_OPTIONAL_)

#include <optional>

_ARGLESS_CORE_BEGIN

template <typename opt_t>
struct parser<std::optional<opt_t>>
{
	using type = std::optional<opt_t>; 

	template <typename char_t>
	static inline parse_result<type, char_t> parse(args<char_t>& args)
	{
		auto result = parser<opt_t>::parse(args);

		if (!result.is_valid())
			return make_default(type());

		return result;
	}

	template <typename char_t>
	static constexpr inline auto name = optional_name<type_name<opt_t, char_t>()>();
};

_ARGLESS_CORE_END

#endif

#if defined(ARGLESS_STDH_VARIANT) || defined(_GLIBCXX_VARIANT) || defined(_LIBCPP_VARIANT) || defined(_VARIANT_)

#include <variant>

_ARGLESS_CORE_BEGIN

template <>
struct parser<std::variant<std::monostate>>
{
	using type = std::variant<std::monostate>; 

	template <typename char_t>
	static inline parse_result<type, char_t> parse(args<char_t>& args)
	{
		return make_default(type());
	}

	template <typename char_t>
	static constexpr inline auto name = str_from<char_t, "<>">();
};

template <typename... var_ts>
struct parser<std::variant<var_ts...>>
{
	static_assert(sizeof...(var_ts) >= 1, "invalid std::variant type");

	using type = std::variant<var_ts...>; 

	using ts = typename tetter<var_ts...>::template filter_t<std::monostate>;

	template <typename char_t>
	static inline parse_result<type, char_t> parse(args<char_t>& args)
	{
		return ts::pop_front::invoke_pipe([&]<typename t, std::size_t>(parse_result<type, char_t>&& result) -> parse_result<type, char_t> {
			if (result.is_valid_no_default())
				return result;

			auto next_result = parser<t>::parse(args);

			if (!next_result.is_valid())
				return result;

			if (next_result.is_default())
			{
				if (result.is_valid())
					return result;
				else
					return make_default(type(std::move(next_result).get())); 
			}
			else
				return type(std::move(next_result).get());
		}, [&]() -> parse_result<type, char_t> {
			using t = typename ts::front;
			auto result = parser<t>::parse(args);

			if (!result.is_valid())
				return make_expected<type>;

			if (result.is_default())
				return make_default(type(std::move(result).get()));
			else
				return type(std::move(result).get());
		}());
	}

	template <typename char_t>
	static constexpr inline auto name = []() {
		auto str = ts::pop_front::invoke_pipe([]<typename t>(auto str){ return [](){
				return or_name<std::remove_cvref_t<decltype(str)>{}(), type_name<t, char_t>()>(); 
			}; }, [](){
				return type_name<typename ts::front, char_t>();
			});
		if constexpr (tetter<var_ts...>::template find_t<std::monostate>::value) return optional_name<char_t, decltype(str){}()>();
		else return str();
	}();
};

_ARGLESS_CORE_END

#endif

#if defined(ARGLESS_STDH_TUPLE) || defined(_GLIBCXX_TUPLE) || defined(_LIBCPP_TUPLE) || defined(_TUPLE_)

#include <tuple>

_ARGLESS_CORE_BEGIN

template <typename tup_t, typename tup_st, typename... tup_ts>
struct parser<std::tuple<tup_t, tup_st, tup_ts...>>
{
	using type = std::tuple<tup_t, tup_st, tup_ts...>; 

	template <typename char_t>
	static inline parse_result<type, char_t> parse(args<char_t>& args)
	{
		const auto from = args.m_index; 
		const auto to = args.m_limit;

		for (size_t l = args.m_limit; l >= from; --l)
		{
			args.m_index = from;
			args.m_limit = l;

			auto result = parser<tup_t>::parse(args);

			if (!result.is_valid())
				continue;

			args.m_limit = to;
			auto rest_result = parser<std::tuple<tup_st, tup_ts...>>::parse(args);

			if (!rest_result.is_valid())
				continue;

			if (result.is_default() && rest_result.is_default())
				return make_default(std::tuple_cat(std::make_tuple(std::move(result.get())), std::move(rest_result.get())));
			else
				return std::tuple_cat(std::make_tuple(std::move(result.get())), std::move(rest_result.get()));
		}

		args.m_index = from;
		args.m_limit = to;
		return make_expected<type>;
	}

	template <typename char_t>
	static constexpr inline auto name = and_name<type_name<tup_t, char_t>(), type_name<std::tuple<tup_st, tup_ts...>, char_t>()>();
};

template <typename tup_t>
struct parser<std::tuple<tup_t>>
{
	using type = std::tuple<tup_t>; 

	template <typename char_t>
	static inline parse_result<type, char_t> parse(args<char_t>& args)
	{
		auto result = parser<tup_t>::parse(args);

		if (!result.is_valid())
			return make_expected<type>;

		if (result.is_default())
			return make_default(type(std::move(result).get()));
		else
			return type(std::move(result).get());
	}

	template <typename char_t>
	static constexpr inline auto name = type_name<tup_t, char_t>();
};

template <>
struct parser<std::tuple<>>
{
	using type = std::tuple<>; 

	template <typename char_t>
	static inline parse_result<type, char_t> parse(args<char_t>& args)
	{
		return make_default(type());
	}

	template <typename char_t>
	static constexpr inline auto name = str_from<char_t>("<>");
};

_ARGLESS_CORE_END

#endif

