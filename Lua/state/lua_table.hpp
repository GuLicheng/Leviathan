#pragma once

#include "../go2cpp.hpp"

#include <functional>
#include <map>
#include <unordered_map>
#include <vector>
#include <algorithm>
#include <iostream>


namespace lua
{

    template <typename ScriptObject>
    struct LuaTableHash
    {
        std::size_t operator()(const ScriptObject& x) const
        {
            return x.hash_code();
        }
    };

    template <typename ScriptObject>
    class LuaTableT
    {
        std::vector<ScriptObject> arr;
        std::unordered_map<ScriptObject, ScriptObject, LuaTableHash<ScriptObject>> map;

        Expected<int64> try_convert_to_integer(const ScriptObject& val)
        {
            if (val.template is<int64>())
                return val.template as<int64>();
            if (val.template is<float64>())
                return float_to_integer(val.template as<float64>());
            return UnExpected("Not int or float.");
        }

        void shrink_array()
        {
            auto pos = std::find_if_not(this->arr.rbegin(), this->arr.rend(), [](const ScriptObject& val) {
                return val.template is<std::nullptr_t>();
            }).base();

            std::cout << "Pos = " << *pos << '\n';

            this->arr.erase(pos, this->arr.end());
        
            // auto rest = arr | std::views::reverse | std::views::drop_while([](const ScriptObject& val) { return x.template is<std::nullptr_t>(); });
            // arr.erase(rest.begin().base(), arr.end());

        }

        void expand_array()
        {
            for (auto idx = std::ssize(this->arr) + 1; 1; ++idx)
            {
                if (auto iter = this->map.find(idx); iter != this->map.end())
                {
                    this->map.erase(iter);
                    this->arr.emplace_back(idx);
                }
                else
                {
                    break;
                }
            }
        }

    public:

        LuaTableT(int nArr, int nRec)
        {
            if (nArr > 0)
                this->arr.resize(nArr);
            if (nRec > 0)
                this->map.reserve(nRec);
        }

        ScriptObject& get(const ScriptObject& key)
        {

            auto idx = this->try_convert_to_integer(key);
            if (idx && *idx >= 1 && *idx <= std::ssize(this->arr))
            {
                return this->arr[*idx - 1];
            }
            else
            {
                return this->map[key];
            }
        }

        const ScriptObject& get(const ScriptObject& key) const
        {
            return const_cast<LuaTableT&>(*this).get(key);
        }

        void put(ScriptObject key, ScriptObject val)
        {

            if (key.template is<std::nullptr_t>())
                panic("table index is nil.");
            if (key.template is<float64>() && std::isnan(key.template as<float64>()))
                panic("table index is NaN");

            auto idx = this->try_convert_to_integer(key);
            if (idx && *idx >= 1)
            {
                auto arr_len = std::ssize(this->arr);
                std::cout << "length = " << arr_len << '\n';
                std::cout << "idx = " << *idx << '\n';
                if (*idx <= arr_len)
                {
                    this->arr[*idx - 1] = std::move(val);
                    if (*idx == arr_len && this->arr[*idx - 1].template is<std::nullptr_t>())
                        this->shrink_array();
                    return;
                }
                if (*idx == arr_len + 1)
                {
                    this->map.erase(key);
                    if (!val.template is<std::nullptr_t>())
                    {
                        this->arr.emplace_back(std::move(val));
                        this->expand_array();
                    }
                    return;
                }
            }

            if (!val.template is<std::nullptr_t>())
            {
                this->map[key] = std::move(val);
            }
            else
            {
                this->map.erase(key);
            }
        }

        int len() const 
        {
            return this->arr.size();
        }

        friend std::ostream& operator<<(std::ostream& os, const LuaTableT& table)
        {
            os << "{";

            for (int i = 0; i < table.arr.size(); ++i)
            {
                std::cout << "(" << i << ", " << table.arr[i] << ")";
            }

            std::cout << "\n-------------------\n";

            for (const auto& [k, v] : table.map)
            {
                std::cout << "(" << k << ", " << v << ")";
            }
            return os << "}";
        }

    };
}

