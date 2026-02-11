#pragma once
#include "argless.hpp"
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
