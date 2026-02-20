#pragma once
#include "core.hpp"
#include "str.hpp"
#include "parser.hpp"

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
