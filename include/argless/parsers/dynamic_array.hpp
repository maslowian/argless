#pragma once
#include "../parser.hpp"

#if defined(ARGLESS_STDH_VECTOR) || defined(_GLIBCXX_VECTOR) || defined(_LIBCPP_VECTOR) || defined(_VECTOR_) || \
	defined(ARGLESS_STDH_LIST) || defined(_GLIBCXX_LIST) || defined(_LIBCPP_LIST) || defined(_LIST_) || \
	defined(ARGLESS_STDH_FORWARD_LIST) || defined(_GLIBCXX_FORWARD_LIST) || defined(_LIBCPP_FORWARD_LIST) || defined(_FORWARD_LIST_) || \
	defined(ARGLESS_STDH_DEQUE) || defined(_GLIBCXX_DEQUE) || defined(_LIBCPP_DEQUE) || defined(_DEQUE_) || \
	defined(ARGLESS_STDH_STACK) || defined(_GLIBCXX_STACK) || defined(_LIBCPP_STACK) || defined(_STACK_) || \
	defined(ARGLESS_STDH_QUEUE) || defined(_GLIBCXX_QUEUE) || defined(_LIBCPP_QUEUE) || defined(_QUEUE_)

#if defined(ARGLESS_STDH_VECTOR) || defined(_GLIBCXX_VECTOR) || defined(_LIBCPP_VECTOR) || defined(_VECTOR_)
#include <vector>
#endif

#if	defined(ARGLESS_STDH_LIST) || defined(_GLIBCXX_LIST) || defined(_LIBCPP_LIST) || defined(_LIST_)
#include <list>
#endif

#if	defined(ARGLESS_STDH_FORWARD_LIST) || defined(_GLIBCXX_FORWARD_LIST) || defined(_LIBCPP_FORWARD_LIST) || defined(_FORWARD_LIST_)
#include <forward_list>
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

#if	defined(ARGLESS_STDH_FORWARD_LIST) || defined(_GLIBCXX_FORWARD_LIST) || defined(_LIBCPP_FORWARD_LIST) || defined(_FORWARD_LIST_)
template <typename t_t, typename t_st>
struct parser<std::forward_list<t_t, t_st>>
{
	using type = std::forward_list<t_t, t_st>; 

	template <typename char_t>
	static inline parse_result<type, char_t> parse(args<char_t>& args)
	{
		type value;
		auto it = value.before_begin();

		while (true)
		{
			auto result = parser<t_t>::template parse<char_t>(args);
			if (!result.is_valid_no_default())
				break;

			it = value.insert_after(it, std::move(result).get());
		}

		if (value.empty())
			return make_default(std::move(value));
		else
			return value;
	}

	template <typename char_t>
	static constexpr inline auto name = array_wrap_name<type_name<t_t, char_t>()>() + str_from<char_t, "[]">();
};
#endif

_ARGLESS_CORE_END

#endif
