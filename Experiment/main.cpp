

// any -> void*
// tuple -> struct
// variant -> union

#include <memory>
#include <typeinfo>
#include <iostream>
#include <assert.h>

struct StorageBase
{
    virtual ~StorageBase() = default;
    virtual std::unique_ptr<StorageBase> clone() const = 0;
    virtual const void* address() = 0;
    virtual const std::type_info& id() = 0;
};

template <typename T>
struct Storage : public StorageBase
{
    T value;

    explicit Storage(const T& t) : value(t) { }
    
    std::unique_ptr<StorageBase> clone() const override
    {
        return std::make_unique<Storage>(value);
    }

    const void* address() override { return &value; }
    
    const std::type_info& id() override
    {
        return typeid(T);
    }

};

struct Any
{
    std::unique_ptr<StorageBase> p = nullptr;

    template <typename T>
    explicit Any(const T& t) : p(std::make_unique<Storage<T>>(t)) { }

    Any(const Any& rhs) : p(rhs.p->clone()) { }
    Any(Any&&) noexcept = default;
    ~Any() = default;

    template <typename T>
    T cast() 
    {
        if (p->id() == typeid(T))
            return *(T*)p->address();
        throw std::bad_typeid();
    }

};


int main()
{
    Any a{ 1 };
    int i = a.cast<int>();
    assert(i == 1);
}
