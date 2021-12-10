#include <iostream>
#include <map>
#include <memory>
#include <functional>
#include <string>
#include <any>

using namespace std;

class IocContainer
{
public:

	//注册一个key对应的类型
	template<class T, class Depend, typename ...Args>
		void registerType(string key)
	{
		std::function<T*(Args...)> func = [] (Args...args){return new T(new Depend(args...)); };
		registerType(key, func);
	}

	//根据唯一标志查询对应的构造器 创建对象
	template<class T, typename...Args>
	T* resolve(string key, Args...args)
	{
		if (m_map.find(key) == m_map.end())
		{
			return nullptr;
		}
		std::any func = m_map[key];

		//转换为合适的类型
		// std::function<T*(Args...)> f = func.cast<std::function<T*(Args...)>>();
		std::function<T*(Args...)> f = std::any_cast<std::function<T*(Args...)>>(func);
		return f(args...);
	}

	template<class T, typename...Args>
	std::shared_ptr<T> resolveShared(string key, Args...args)
	{
		T* t = resolve<T>(key, args...);
		return std::shared_ptr<T>(t);
	}

private:
	void registerType(string key, std::any func)
	{
		if (m_map.find(key) != m_map.end())
		{
			throw std::invalid_argument("this key has exist");
		}
		m_map.emplace(key, func);
	}

private:
	std::map<string, std::any> m_map;
};

struct Base
{
	virtual void func() {}
	virtual ~Base() {}
};

struct Derived : Base
{
	Derived(int a, double d) :m_a(a), m_d(d) {}
	virtual void func() {
		cout << "derived :"<<m_a + m_d << endl;
	}
private:
	int m_a;
	double m_d;
};

struct A
{
	A(Base* ptr) : m_ptr(ptr) {}

	virtual void func() {
		m_ptr->func();
	}

private:
	Base* m_ptr;
};


