#pragma once

#include <utility>
#include <functional>

namespace leviathan::detail
{
	template <typename T>
	inline constexpr bool pass_by_value_v = std::is_trivially_copyable_v<T>
 	 									 && std::is_trivially_destructible_v<T>
										 && sizeof(T) <= sizeof(void*);

	template <typename Comp, typename Proj>
	constexpr auto make_comp_proj(Comp& comp, Proj& proj)
	{
		if constexpr (pass_by_value_v<Comp> && pass_by_value_v<Proj>)
		{
			return [=]<typename L, typename R>(L&& lhs, R&& rhs) -> bool {
				return std::invoke(comp, 
					std::invoke(proj, (L&&)lhs),
					std::invoke(proj, (R&&)rhs)
				);
			};
		}
		else	
		{
			return [&]<typename L, typename R>(L&& lhs, R&& rhs) -> bool {
				return std::invoke(comp, 
					std::invoke(proj, (L&&)lhs),
					std::invoke(proj, (R&&)rhs)
				);
			};
		}
	}
}

