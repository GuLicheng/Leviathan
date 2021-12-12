#pragma once

#include <lv_cpp/meta/template_info.hpp>

#include <map>
#include <memory>
#include <functional>
#include <any>
#include <utility>
#include <algorithm>
#include <fstream>
#include <iostream>
#include <ranges>

// helper meta for deducing type
template <typename T>
struct referece_traits : std::type_identity<T> { };

template <typename T>
struct referece_traits<std::reference_wrapper<T>> : std::type_identity<T&> { };

// the return instance registered by calling `register_instance` should not be deleted
template <typename T>
class conditional_deleter : public std::default_delete<T>
{
    using base = std::default_delete<T>;
    bool m_deleted;

public: 
    constexpr conditional_deleter(bool is_deleted = true) : base{ }, m_deleted{ is_deleted } { }
    
    void operator()(T *ptr) const
    {
        if (m_deleted)
            base::operator()(ptr);
    }
};


class ioc_container
{
    struct info
    {
        enum class type { Ctor, Instance };
        std::string_view m_base_name;
        std::string_view m_derived_name;
        std::any m_ctor_or_instance; // ctor for derived class or a derived class instance
        type m_id; //   
    };

    std::vector<info> m_container;

public:

    ioc_container() = default;
    ioc_container(const ioc_container&) = delete;
    ioc_container& operator=(ioc_container&&) = delete;

    template <typename BaseClass, typename DerivedClass, typename... Args>
    void register_type()
    {
        static_assert(std::is_abstract_v<BaseClass>, "BaseClass should be interface");
        static_assert(!std::is_abstract_v<DerivedClass>, "DerivedClass cannot be interface");
        static_assert(std::is_base_of_v<BaseClass, DerivedClass>);
        static_assert(std::is_constructible_v<DerivedClass, Args...>);
        static_assert(std::is_same_v<BaseClass, std::remove_cvref_t<BaseClass>>);
        static_assert(std::is_same_v<DerivedClass, std::remove_cvref_t<DerivedClass>>);

        constexpr std::string_view base_name = TypeInfo(BaseClass);
        constexpr std::string_view derived_name = TypeInfo(DerivedClass);
        
        std::function<BaseClass*(Args...)> fn = [](Args... args) { return new DerivedClass(args...); };

        auto iter = std::ranges::find_if(m_container, [](const info& i)
        {
            return i.m_base_name == base_name && i.m_derived_name == derived_name;
        });
        if (iter == m_container.end()) 
        {
            m_container.emplace_back(info {
                .m_base_name = base_name,
                .m_derived_name = derived_name,
                .m_ctor_or_instance = std::move(fn),
                .m_id = info::type::Ctor
            });    
        }
        else
        {
            // cover 
            iter->m_ctor_or_instance = std::move(fn),
            iter->m_id = info::type::Ctor;
        }
    }

    template <typename ClassType = void, typename InstanceType, typename RealClassType = std::conditional_t<std::is_void_v<ClassType>, InstanceType, ClassType>>
    void register_instance(InstanceType* instance)
    {
        static_assert(!std::is_abstract_v<InstanceType>, "Instance type cannot be interface");

        constexpr std::string_view base_name = TypeInfo(RealClassType);
        constexpr std::string_view derived_name = TypeInfo(InstanceType);

        auto iter = std::ranges::find_if(m_container, [](const info& i)
        {
            return i.m_base_name == base_name && i.m_derived_name == derived_name;
        });
        if (iter == m_container.end())
            m_container.emplace_back(info {
                .m_base_name = base_name,
                .m_derived_name = derived_name,
                .m_ctor_or_instance = static_cast<RealClassType*>(instance),
                .m_id = info::type::Instance
            }); 
        else
            // cover 
            iter->m_ctor_or_instance = static_cast<RealClassType*>(instance); 
            iter->m_id = info::type::Instance;
    }


    template <typename ClassType, typename... Args>
    std::unique_ptr<ClassType, conditional_deleter<ClassType>> reslove(Args... args)
    {
        static_assert(std::is_class_v<ClassType>);
        // auto iter = std::ranges::find_if(m_container.rbegin(), m_container.rend(), [](const info& i) 
        // clang error
        auto iter = std::ranges::find_if(m_container | std::views::reverse, [](const info& i) 
        {
            constexpr auto cls_name = TypeInfo(ClassType); 
            return i.m_base_name == cls_name;
        });

        if (iter.base() == m_container.begin()) 
            return nullptr;

        if (iter->m_id == info::type::Ctor)
        {
            using fn_type = std::function<ClassType*(typename referece_traits<Args>::type...)>;
            auto& fn = std::any_cast<fn_type&>(iter->m_ctor_or_instance);
            auto* p = fn(std::move(args)...);
            return std::unique_ptr<ClassType, conditional_deleter<ClassType>>(p);
        }
        else
        {
            auto* instance = std::any_cast<ClassType*>(iter->m_ctor_or_instance);
            conditional_deleter<ClassType> deleter{ false };
            return std::unique_ptr<ClassType, conditional_deleter<ClassType>>(instance, deleter);
        }

    }

};