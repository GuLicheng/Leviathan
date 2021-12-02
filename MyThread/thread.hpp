#pragma once

#include <type_traits>  // for std::remove_reference
#include <functional>   // for std::invoke
#include <memory>       // std::unique_ptr
#include <pthread.h>
#include "mutex.hpp"
#include <chrono>

class Thread
{
    pthread_t id;    
public:
    Thread() noexcept = default;
    template <typename Callable, typename... Args>
    explicit Thread(Callable&& f, Args&&... args)
    {
        auto arg = new auto([f=std::forward<Callable>(f), ...args=std::forward<Args>(args)]() mutable
        {
            return std::invoke(std::move(f), std::move(args)...); // STL call && instead
        });
        using handle_type = std::remove_reference_t<decltype(*arg)>;
        void *(*c_func) (void *) = [](void* args)
        {
            std::unique_ptr<handle_type> ptr { static_cast<handle_type*>(args) };
            (void)(*ptr)();
            return (void*)nullptr;
        };
        pthread_create(&this->id, nullptr, c_func, arg);
    }

    ~Thread()
    {
        pthread_join(this->id, nullptr);
    }

};

