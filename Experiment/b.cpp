
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
    conditional_deleter(bool is_deleted = true) : base{ }, m_deleted{ is_deleted } { }
    
    void operator()(T *ptr) const
    {
        if (m_deleted)
            base::operator()(ptr);
    }
};


class ioc_container
{
    // Pair<BaseClassName, DerivedName>
    // BaseClassName1 -> <(DerivedName1, any), (DerivedName2, any)> ...
    // BaseClassName2 -> <(DerivedName3, any)>

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

        auto iter = std::ranges::find_if(m_container, [=](const info& i)
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
            *iter = info {
                .m_base_name = base_name,
                .m_derived_name = derived_name,
                .m_ctor_or_instance = std::move(fn),
                .m_id = info::type::Ctor
            };
        }
    }

    template <typename ClassType = void, typename InstanceType, typename RealClassType = std::conditional_t<std::is_void_v<ClassType>, InstanceType, ClassType>>
    void register_instance(InstanceType* instance)
    {
        static_assert(!std::is_abstract_v<InstanceType>, "Instance type cannot be interface");

        constexpr std::string_view base_name = TypeInfo(RealClassType);
        constexpr std::string_view derived_name = TypeInfo(InstanceType);

        auto iter = std::ranges::find_if(m_container, [=](const info& i)
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
            *iter = info {
                .m_base_name = base_name,
                .m_derived_name = derived_name,
                .m_ctor_or_instance = static_cast<RealClassType*>(instance),
                .m_id = info::type::Instance
            };
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

#include <iostream>
// Some code from https://zhuanlan.zhihu.com/p/83537599
class Shoes
{
public:
    virtual void Show() = 0;
    virtual ~Shoes() {}
};

// 耐克鞋子
class NiKeShoes : public Shoes
{
public:
    void Show()
    {
        std::cout << "我是耐克球鞋，我的广告语：Just do it" << std::endl;
    }
};

class AdidasShoes : public Shoes
{
public:
    void Show()
    {
        std::cout << "我是阿迪达斯，我的广告语：不知道" << std::endl;
    }
};

// 基类 衣服
class Clothe
{
public:
    virtual void Show() = 0;
    virtual ~Clothe() {}
};

// 优衣库衣服
class UniqloClothe : public Clothe
{
public:
    void Show()
    {
        std::cout << "我是优衣库衣服，我的广告语：I am Uniqlo" << std::endl;
    }
};

template <class AbstractProduct_t>
class AbstractFactory
{
public:
    virtual AbstractProduct_t *CreateProduct() = 0;
    virtual ~AbstractFactory() {}
};

// 具体模板工厂类
// 模板参数：AbstractProduct_t 产品抽象类，ConcreteProduct_t 产品具体类
template <class AbstractProduct_t, class ConcreteProduct_t>
class ConcreteFactory : public AbstractFactory<AbstractProduct_t>
{
public:
    AbstractProduct_t *CreateProduct()
    {
        return new ConcreteProduct_t();
    }
};

// Use our IOC
void test_ioc()
{
    ioc_container ioc;

    // 构造耐克鞋的工厂对象 
    ioc.register_type<Shoes, NiKeShoes>();
    // 创建耐克鞋对象 
    auto p_nike_shoes = ioc.reslove<Shoes>();
    // 打印耐克鞋广告语
    p_nike_shoes->Show();


    // 构造优衣库衣服的工厂对象
    ioc.register_type<Clothe, UniqloClothe>();
    // 创建优衣库衣服对象
    auto p_uniqlo_clothe = ioc.reslove<Clothe>();
    // 打印优衣库广告语
    p_uniqlo_clothe->Show();

    // register instance
    auto* singleton = new AdidasShoes();
    ioc.register_instance<Shoes>(singleton);
    ioc.reslove<Shoes>()->Show();

}

struct IStream
{
    virtual ~IStream() { }
    virtual void Report(std::string_view = "") = 0;
};

struct ConsoleStream : IStream
{
    ConsoleStream(std::ostream& os) : m_os{ os } { }
    ~ConsoleStream() { std::cout << "ConsoleStream Destory\n"; }

    void Report(std::string_view info = "ConsoleStream") override
    { m_os << info; }

    std::ostream& m_os;
};

struct FileStream : IStream
{
    FileStream(const std::string& filepath) : m_fs{ } { m_fs.open(filepath, std::ios_base::binary | std::ios_base::out | std::ios_base::app); }
    ~FileStream() { std::cout << "FileStream Destory\n"; }

    void Report(std::string_view info = "FileStream") override
    { m_fs << info; }
    
    std::ofstream m_fs;
};

void test___()
{
    ioc_container c;
    c.register_type<IStream, ConsoleStream, std::ostream&>();
    c.reslove<IStream>(std::ref(std::cout))->Report("Console.WriteLn\n");
    c.register_type<IStream, FileStream, std::string>();
    std::string file = "./Hello.txt";
    c.reslove<IStream>(std::move(file))->Report("FileStream.WriteLn(\"This is IOCContainer\")\n");
}

int main()
{
#ifdef _WIN32
    system("chcp 65001");
#endif
    test_ioc();

    static_assert(std::is_abstract_v<Shoes>);
    static_assert(!std::is_abstract_v<NiKeShoes>);
    test___();
}
