#pragma once

#include "named_tuple.hpp"

#include <optional>
#include <memory>

namespace leviathan
{
    template <basic_fixed_string... Names>
    struct builder
    {
        /**
         * @brief This class is just for adaptor multi-template parameters.
        */
        template <typename... Ts>
        struct builder2
        {
            using tuple_type = named_tuple<field<Names, Ts>...>; 

            struct builder_impl
            {
                builder_impl() = default;

                tuple_type m_t;

                template <basic_fixed_string Name, typename... Args>
                builder_impl&& build_member(Args&&... args) &&
                {
                    m_t.template get_with<Name>().emplace((Args&&) args...);
                    return std::move(*this);
                }

                template <size_t I, typename... Args>
                builder_impl&& build_member(Args&&... args) &&
                {
                    std::get<I>(m_t.get_tuple()).emplace((Args&&) args...);
                    return std::move(*this);
                }

                /**
                 * @brief A deleter may be added here.
                */
                template <typename Target>
                std::unique_ptr<Target> build() && 
                {
                    auto target = std::apply([](auto&&... op) {
                        if (!(op.has_value() && ...))
                            throw std::bad_optional_access();
                        return new Target(std::move(*op)...);
                    }, m_t.get_tuple()); 
                    return std::unique_ptr<Target>(target);
                }
            };
        };

        template <typename... Ts>
        using with = typename builder2<std::optional<Ts>...>::builder_impl;
    };


} // namespace leviathan







