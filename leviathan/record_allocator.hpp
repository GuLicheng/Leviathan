#pragma once

#include <memory>
#include <stdexcept>
#include <map>
#include <string>
#include <iostream>
#include <algorithm>
#include <leviathan/meta/template_info.hpp>
#include <memory_resource>


struct Counter
{
    std::size_t allocate_size = 0;
    std::size_t deallocate_size = 0;

    std::size_t allocate_cnt = 0;
    std::size_t construct_cnt = 0;
    std::size_t destroy_cnt = 0;
    std::size_t deallocate_cnt = 0;

    bool Check() const
    { return allocate_size == deallocate_size; }

    friend std::ostream& operator<<(std::ostream& os, const Counter& counter)
    {
        #define PrintX(x) (os << #x << ": " << counter. x << '\n')
        PrintX(allocate_size);
        PrintX(deallocate_size);
        PrintX(allocate_cnt);
        PrintX(construct_cnt);
        PrintX(destroy_cnt);
        PrintX(deallocate_cnt);
        #undef PrintX
        return os;
    }
    
};

std::map<std::string_view, Counter> recorder;

void Report()
{
    for (const auto& [name, c] : recorder)
        std::cout << "Name: " << name << "\n" << c << '\n';
}

bool CheckMemoryAlloc()
{
    return std::all_of(recorder.begin(), recorder.end(), [](const auto& r){
        return r.second.Check();
    });
}

template <typename T>
class RecordAllocator : public std::allocator<T>
{
    using Base = std::allocator<T>;
    using Self = RecordAllocator<T>;

    static constexpr std::string_view Name = TypeInfo(Self);

public:
    [[nodiscard]] constexpr T *allocate(std::size_t n)
    {
        recorder[Name].allocate_cnt++;
        recorder[Name].allocate_size += n * sizeof(T);
        return Base::allocate(n);
    }

    template <class U, class... Args>
    void construct(U *p, Args &&...args)
    {
        recorder[Name].construct_cnt++;
        std::construct_at(p, (Args&&) args...);
    }

    template <class U>
    void destroy(U *p)
    {
        recorder[Name].destroy_cnt++;
        std::destroy_at(p);
    }

    constexpr void deallocate(T *p, std::size_t n)
    {
        recorder[Name].deallocate_cnt++;
        recorder[Name].deallocate_size += n * sizeof(T);
        Base::deallocate(p, n);
    }
};

template <typename T>
class RecordPolymorphicAllocator : public std::pmr::polymorphic_allocator<T>
{
    using Base = std::pmr::polymorphic_allocator<T>;
    using Self = RecordPolymorphicAllocator<T>;

    static constexpr std::string_view Name = TypeInfo(Self);

public:
    [[nodiscard]] constexpr T *allocate(std::size_t n)
    {
        recorder[Name].allocate_cnt++;
        recorder[Name].allocate_size += n * sizeof(T);
        return Base::allocate(n);
    }

    template <class U, class... Args>
    void construct(U *p, Args &&...args)
    {
        recorder[Name].construct_cnt++;
        Base::construct(p, (Args&&) args...);
    }

    template <class U>
    void destroy(U *p)
    {
        recorder[Name].destroy_cnt++;
        Base::destroy(p);
    }

    constexpr void deallocate(T *p, std::size_t n)
    {
        recorder[Name].deallocate_cnt++;
        recorder[Name].deallocate_size += n * sizeof(T);
        Base::deallocate(p, n);
    }
};


