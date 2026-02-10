#pragma once
#include "core.hpp"
#include "str.hpp"

#include "tetter.hpp"

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
	constexpr inline parse_result(const _expected_t<u>& value) : parse_result(_ARGLESS_CORE expected<char_t>(_expected_t<u>::template name<char_t>)) {}

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
inline consteval auto str_name()
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
inline consteval auto str_number() 
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
