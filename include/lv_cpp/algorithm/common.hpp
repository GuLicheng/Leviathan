#pragma once

#include <utility>
#include <functional>

namespace leviathan::detail
{
	template <typename Comp, typename Proj>
	constexpr auto make_comp_proj(Comp& comp, Proj& proj)
	{
		return [&]<typename L, typename R>(L&& lhs, R&& rhs) -> bool {
			return std::invoke(comp, 
				std::invoke(proj, (L&&)lhs),
				std::invoke(proj, (R&&)rhs)
			);
		};
	}

}






