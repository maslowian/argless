#pragma once
#include "core.hpp"

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
