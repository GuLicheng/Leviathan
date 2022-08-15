#pragma once

#include <utility>
#include <functional>

namespace leviathan::detail
{
	template <typename Comp, typename Proj>
	constexpr auto make_comp_proj(Comp& comp, Proj& proj)
	{
		return [&](auto&& lhs, auto&& rhs) -> bool {
			using L = decltype(lhs);
			using R = decltype(rhs);
			return std::invoke(comp, 
				std::invoke(proj, std::forward<L>(lhs)),
				std::invoke(proj, std::forward<R>(rhs))
			);
		};
	}

}






