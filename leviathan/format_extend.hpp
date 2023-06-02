#pragma once

#include <iostream>
#include <ranges>
#include <tuple>
#include <format>

template <typename Container>
concept constable_range = requires (const Container &c)
{
	// not support std::generator
	std::ranges::begin(c);
	std::ranges::end(c);
};


template <constable_range Ranges, typename CharT>
struct std::formatter<Ranges, CharT>
{
	// accept a context and return it's iterator
	// std::basic_format_context<BackInsertIterator, CharType>
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
			iter = std::vformat_to(format_context.out(), fmt, std::make_format_args(*vec_iter));
		}
		iter = ']';
		return iter;
	}

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
	CharT m_fmt[16];  // tiny buffer to store args
	size_t m_buffer_len = 0;
};



