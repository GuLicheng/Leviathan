#ifndef __STATIC_LOPPER_HPP__
#define __STATIC_LOPPER_HPP__

#include <lv_cpp/io/console.hpp>

#include <tuple>
#include <type_traits>
#include <functional>

namespace leviathan::tuple
{

    /*
    * for Left < Right, it will perform as for (int i = Left, i < Right; i += Step), so you should pass <0, size>
    * for Left >= Right, it will perform as for (int i = Left; i >= Right; i += Step), so you should pass <size-1, 0>
    */
    template <int Left, int Right, int Step = std::conditional_t<(Left < Right), std::integral_constant<int, 1>, std::integral_constant<int, -1>>::value, 
        typename Comparetor = std::conditional_t<(Step > 0), std::less<void>, std::greater_equal<void>>>
    struct static_looper
    {    
        static_assert(Step != 0);
        /*
            for (int i = init; i < size; ++i);
            for (int i = size-1; i >=0; --i);
        */

        inline constexpr static Comparetor comparator = {};

        template <typename Tuple1, typename Tuple2, typename BinaryOp1, typename BinaryOp2, typename Init>
        constexpr static decltype(auto) inner_product(const Tuple1& t1, const Tuple2& t2, BinaryOp1 op1, BinaryOp2 op2, Init init)
        {
            if constexpr (comparator(Left,Right))
            {
                auto a_op1_b = op1(std::get<Left>(t1), std::get<Left>(t2));
                init = op2(std::move(a_op1_b), std::move(init));
                return static_looper<Left + Step, Right, Step, Comparetor>::inner_product
                    (t1, t2, std::move(op1), std::move(op2), std::move(init));
            }
            else
            {
                return init;
            }
        }

        template <typename Tuple, typename BinaryOp, typename Init>
        constexpr static decltype(auto) reduce(const Tuple& t, BinaryOp op, Init init)
        {
            if constexpr (comparator(Left, Right))
            {
                init = op(std::get<Left>(t), std::move(init));
                return static_looper<Left + Step, Right, Step, Comparetor>::reduce(t, std::move(op), std::move(init));
            }
            else
            {
                return init;
            }
        }

        template <typename Tuple1, typename Tuple2, typename BinaryOp>
        constexpr static ssize_t mismatch(const Tuple1& t1, const Tuple2& t2, BinaryOp op)
        {
            if constexpr(comparator(Left, Right))
            {
                if (op(std::get<Left>(t1), std::get<Left>(t2)))
                {
                    return static_looper<Left + Step, Right, Step, Comparetor>::mismatch(t1, t2, std::move(op));
                }
                else 
                {
                    return static_cast<ssize_t>(Left);
                }
            }
            else
            {
                return static_cast<ssize_t>(Left);
            }
        }


        template <template <typename...> typename Tuple, typename... Ts, typename Operation>
        constexpr static void dynamic_set(Tuple<Ts...>& t, Operation op, int idx)
        {
            if constexpr (comparator(Left, Right))
            {
                if constexpr (std::invocable<decltype(op), decltype(std::get<Left>(t))>)
                {
                    if (Left == idx)
                        op(std::get<Left>(t));
                    else
                        static_looper<Left + Step, Right, Step, Comparetor>::
                            dynamic_set(t, std::move(op), idx);
                }
                else
                {
                    static_looper<Left + Step, Right, Step, Comparetor>::
                        dynamic_set(t, std::move(op), idx);
                }
            }
            else
            {
                // this routine will be exec if idx > sizeof...(Tuple)
                return ;
            }
        }

    
    };

} // namespace tuple

#endif
