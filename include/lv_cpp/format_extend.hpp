#ifndef __FORMAT_EXTEND_HPP__
#define __FORMAT_EXTEND_HPP__


#include <lv_cpp/tuples/tuple_base.hpp>

#include <iostream>
#include <tuple>
#include <vector>
#include <list>
#include <set>
#include <array>
#include <variant>
#include <format>


#define TINY_BUFFER_SIZE 16

template <typename Container>
concept ContainerChecker = requires (const Container &c)
{
	// not support std::generator
	std::ranges::begin(c);
	std::ranges::end(c);
};
// tuple - like
template <typename Tuple>
concept TupleChecker = !ContainerChecker<Tuple> && leviathan::meta::tuple_like<Tuple>;

template <ContainerChecker Ranges, typename CharT>
struct std::formatter<Ranges, CharT>
{
	// accept a context and return it's iterator
	// std::basic_format_context<BackInsertIteartor, CharType>
	constexpr auto format(const Ranges& rg, auto& format_context)
	{
		auto iter = std::format_to(format_context.out(), "{}", '[');
		auto begin = std::ranges::begin(rg);
		auto end = std::ranges::end(rg);
		std::basic_string_view<CharT> fmt(m_fmt, m_fmt + m_buffer_len);
		for (auto vec_iter = begin; vec_iter != end; ++vec_iter)
		{
			if (vec_iter != begin)
			{
				iter = ',', iter = ' ';
			}
			iter = std::format_to(format_context.out(), fmt, *vec_iter);
		}
		iter = ']';
		return iter;
	}
	// Accept a context and return it's iteartor
	// std::basic_format_parse_context<OutIter, CharT>
	constexpr auto parse(auto& context)
	{
		m_fmt[m_buffer_len++] = '{';
		auto iter = context.begin();
		if (iter == context.end() || *iter == '}')
		{
			m_fmt[m_buffer_len++] = '}';
			return iter;
		}
		m_fmt[m_buffer_len++] = ':';
		for (; iter != context.end() && *iter != '}'; ++iter)
		{
			m_fmt[m_buffer_len++] = *iter;
		}
		m_fmt[m_buffer_len++] = '}';
		return iter;
	}
private:
	CharT m_fmt[TINY_BUFFER_SIZE];  // tiny buffer to store args
	size_t m_buffer_len = 0;
};

template <TupleChecker Tuple, typename CharT>
struct std::formatter<Tuple, CharT> 
{
	constexpr static auto size = std::tuple_size_v<Tuple>;

	constexpr auto format(const Tuple& t, auto& format_context)
	{
		auto __print_tuple = [&]<size_t... Idx>(std::index_sequence<Idx...>)
		{
			auto iter = std::format_to(format_context.out(), "{}", '(');

			auto write = [&]<size_t Index>()
			{
				if constexpr (Index != 0)
					iter = ',', iter = ' ';
				const auto index = Index < m_len ? Index : m_len;
				std::basic_string_view<CharT> fmt{ m_fmts[index], m_fmts[index] + m_buffers_len[index] };
				iter = std::format_to(format_context.out(), fmt, std::get<Index>(t));
			};

			(write.operator() < Idx > (), ...);

			iter = ')';
			return iter;
		};
		return __print_tuple(std::make_index_sequence<size>());
	}

	constexpr auto parse(auto& context)
	{
		auto iter = context.begin();
		auto end = context.end();
		m_fmts[0][m_buffers_len[m_len]++] = '{';
		if (iter == end || *iter == '}')
		{
			m_fmts[0][m_buffers_len[m_len]++] = '}';
			return iter;
		}
		m_fmts[0][m_buffers_len[m_len]++] = ':';
		for (; iter != end && *iter != '}'; ++iter)
		{
			if (*iter == '|')
			{
				m_fmts[m_len][m_buffers_len[m_len]++] = '}';
				m_len++;
				m_fmts[m_len][m_buffers_len[m_len]++] = '{';
				m_fmts[m_len][m_buffers_len[m_len]++] = ':';
			}
			else 
			{
				m_fmts[m_len][m_buffers_len[m_len]++] = *iter;
			}
		}
		m_fmts[m_len][m_buffers_len[m_len]++] = '}';
		return iter;
	}

	CharT m_fmts[size][TINY_BUFFER_SIZE];
	size_t m_buffers_len[size] = {};
	size_t m_len = 0;  // last format context

};

#undef TINY_BUFFER_SIZE

#endif