#include <lv_cpp/ioc_container.hpp>


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
    virtual AbstractProduct_t* CreateProduct() = 0;
    virtual ~AbstractFactory() {}
};

// 具体模板工厂类
// 模板参数：AbstractProduct_t 产品抽象类，ConcreteProduct_t 产品具体类
template <class AbstractProduct_t, class ConcreteProduct_t>
class ConcreteFactory : public AbstractFactory<AbstractProduct_t>
{
public:
    AbstractProduct_t* CreateProduct()
    {
        return new ConcreteProduct_t();
    }
};


void BiHuTest()
{
    // 构造耐克鞋的工厂对象
    ConcreteFactory<Shoes, NiKeShoes> nikeFactory;
    // 创建耐克鞋对象
    Shoes* pNiKeShoes = nikeFactory.CreateProduct();
    // 打印耐克鞋广告语
    pNiKeShoes->Show();

    // 构造优衣库衣服的工厂对象
    ConcreteFactory<Clothe, UniqloClothe> uniqloFactory;
    // 创建优衣库衣服对象
    Clothe* pUniqloClothe = uniqloFactory.CreateProduct();
    // 打印优衣库广告语
    pUniqloClothe->Show();

    // 释放资源
    delete pNiKeShoes;
    pNiKeShoes = NULL;

    delete pUniqloClothe;
    pUniqloClothe = NULL;
}

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


    // Singleton
    ioc.register_instance<Shoes>(new NiKeShoes());
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
    {
        m_os << info;
    }

    std::ostream& m_os;
};

struct FileStream : IStream
{
    FileStream(const std::string& filepath) : m_fs{ } { m_fs.open(filepath, std::ios_base::binary | std::ios_base::out | std::ios_base::app); }
    ~FileStream() { std::cout << "FileStream Destory\n"; }

    void Report(std::string_view info = "FileStream") override
    {
        m_fs << info;
    }

    std::ofstream m_fs;
};

void test()
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
    //system("chcp 65001");
#endif
    BiHuTest();
    test();
    DivingLine();
    test_ioc();

     

    std::cout << "OK\n";
}