#pragma once

#include "mutex.hpp"

#include <pthread.h>

#include <system_error>
#include <utility>
#include <chrono>
#include <type_traits>
#include <tuple>
#include <functional> 

template <typename MutexType>
class LockGuard
{
public:
    using mutex_type = MutexType;

    explicit LockGuard(mutex_type& m) : m_device(m)
    { this->m_device.lock(); }

    LockGuard(mutex_type& m, AdoptLock) noexcept : m_device(m)
    { } // calling thread owns mutex

    ~LockGuard() 
    { this->m_device.unlock(); }

    NOCOPYABLE(LockGuard)

private:
    mutex_type& m_device;
};

template <typename MutexType>
class UniqueLock
{
public:
    using mutex_type = MutexType;

    UniqueLock() noexcept 
        : m_device(nullptr), m_owns(false)
    { }

    explicit UniqueLock(mutex_type& m)
        : m_device(std::addressof(m)), m_owns(false)
    {
        lock();
        m_owns = true;
    }

    UniqueLock(mutex_type& m, DeferLock) noexcept
        : m_device(std::addressof(m)), m_owns(false)
    { }

    UniqueLock(mutex_type& m, TryToLock)
        : m_device(std::addressof(m)), m_owns(m_device->try_lock())
    { }

    UniqueLock(mutex_type& m, AdoptLock) noexcept
        : m_device(std::addressof(m)), m_owns(true)
    {
        // calling thread owns mutex
        // This lock shoule unlock when destoried
    }

    template <typename ClockType, typename DurationType>
    UniqueLock(mutex_type& m, const std::chrono::time_point<ClockType, DurationType>& atime)
        : m_device(std::addressof(m)), m_owns(m_device->try_lock_until(atime))
    { }

    template <typename RepType, typename PeriodType>
    UniqueLock(mutex_type& m, const std::chrono::duration<RepType, PeriodType>& rtime)
        : m_device(std::addressof(m)), m_owns(m_device->try_lock_for(rtime))
    { }

    ~UniqueLock()
    {
        if (this->m_owns)
            unlock();
    }

    NOCOPYABLE(UniqueLock)

    UniqueLock(UniqueLock&& rhs) noexcept
        : m_device(rhs.m_device), m_owns(rhs.m_owns)
    {
        rhs.m_device = nullptr;
        rhs.m_owns = false;
    }

    UniqueLock& operator=(UniqueLock&& rhs) noexcept
    {
        if (this->m_owns)
            unlock();
        UniqueLock(std::move(rhs)).swap(*this);
        rhs.m_device = nullptr;
        rhs.m_owns = false;
        return *this;
    }

    void lock()
    {
        check_state();
        this->m_device->lock();
        this->m_owns = true;
    }

    bool try_lock()
    {
        check_state();
        this->m_owns = this->m_device->try_lock();
        return this->m_owns;
    }
            
    template <typename ClockType, typename DurationType>
    bool try_lock_until(const std::chrono::time_point<ClockType, DurationType>& atime)
    {
        check_state();
        this->m_owns = this->m_device->try_lock_until(atime);
        return this->m_owns;
    }

    template <typename RepType, typename PeriodType>
    bool try_lock_for(const std::chrono::duration<RepType, PeriodType>& rtime)
    {
        check_state();
        this->m_owns = this->m_device->try_lock_for(rtime);
        return this->m_owns;
    }

    void unlock()
    {
        // we cannot unlock if we don't own resource
        if (!this->m_owns)
            throw std::system_error(std::make_error_code(std::errc::operation_not_permitted));
        else if (this->m_device)
        {
            this->m_device->unlock();
            this->m_owns = false;
        }
    }

    void swap(UniqueLock& rhs) noexcept
    {
        std::swap(this->m_device, rhs.m_device);
        std::swap(this->m_owns, rhs.m_owns);
    }

    mutex_type* release() noexcept
    {
        mutex_type* ret = this->m_device;
        this->m_device = nullptr;
        this->m_owns = false;
        return ret;
    }

    bool owns_lock() const noexcept
    { return this->m_owns; }

    explicit operator bool() const noexcept
    { return this->owns_lock(); }

    mutex_type* mutex() const noexcept
    { return this->m_device; }

private:

    void check_state() 
    {
        if (!this->m_device)
            throw std::system_error(std::make_error_code(std::errc::operation_not_permitted));
        if (this->m_owns)
            throw std::system_error(std::make_error_code(std::errc::resource_deadlock_would_occur));
    }

    mutex_type* m_device;
    bool m_owns;
};



template <typename MutexType>
class SharedLock
{
public:

    using mutex_type = MutexType;

    SharedLock() noexcept : m_mutex(nullptr), m_owns(false) { }

    explicit
    SharedLock(mutex_type& m)
        : m_mutex(std::addressof(m)), m_owns(true)
    { m.lock_shared(); }

    SharedLock(mutex_type& m, DeferLock) noexcept
        : m_mutex(std::addressof(m)), m_owns(false) { }

    SharedLock(mutex_type& m, TryToLock)
        : m_mutex(std::addressof(m)), m_owns(m.try_lock_shared()) { }

    SharedLock(mutex_type& m, AdoptLock)
        : m_mutex(std::addressof(m)), m_owns(true) { }

    template<typename ClockType, typename DurationType>
    SharedLock(mutex_type& m, const std::chrono::time_point<ClockType, DurationType>& atime)
        : m_mutex(std::addressof(m)),
    m_owns(m.try_lock_shared_until(atime)) { }

    template<typename _Rep, typename _Period>
    SharedLock(mutex_type& m, const std::chrono::duration<_Rep, _Period>& rtime)
        : m_mutex(std::addressof(m)),
    m_owns(m.try_lock_shared_for(rtime)) { }
 
    ~SharedLock()
    {
        if (this->m_owns)
            this->m_mutex->unlock_shared();
    }

    NOCOPYABLE(SharedLock)

    SharedLock(SharedLock &&rhs) noexcept : SharedLock()
    { swap(rhs); }

    SharedLock& operator=(SharedLock &&rhs) noexcept
    {
        SharedLock(std::move(rhs)).swap(*this);
        return *this;
    }

    void lock()
    {
        check_state();
        this->m_mutex->lock_shared();
        this->m_owns = true;
    }

    bool try_lock()
    {
        check_state();
        return this->m_owns = this->m_mutex->try_lock_shared();
    }

    template <typename RepType, typename PeriodType>
    bool try_lock_for(const std::chrono::duration<RepType, PeriodType>& rtime)
    {
        check_state();
        return this->m_owns = this->m_mutex->try_lock_for(rtime);
    }

    template <typename ClockType, typename DurationType>
    bool try_lock_until(const std::chrono::time_point<ClockType, DurationType> &atime)
    {
        check_state();
        return this->m_owns = this->m_mutex->try_lock_until(atime);
    }

    void unlock()
    {
        if (!this->m_owns)
            throw std::system_error(std::make_error_code(std::errc::resource_deadlock_would_occur));
        this->m_mutex->unlock_shared();
        this->m_owns = false;
    }

    void swap(SharedLock& rhs) noexcept
    {
        std::swap(this->m_owns, rhs.m_owns);
        std::swap(this->m_mutex, rhs.m_mutex);
    }

    bool owns_lock() const noexcept { return this->m_owns; }

    explicit operator bool() const noexcept { return this->m_owns; }

    mutex_type* mutex() const noexcept { return this->m_mutex; }

    mutex_type* release() noexcept
    {
        this->m_owns = false;
        return std::exchange(this->m_mutex, nullptr);
    }

private:

    void check_state() 
    {
        if (!this->m_device)
            throw std::system_error(std::make_error_code(std::errc::operation_not_permitted));
        if (this->m_owns)
            throw std::system_error(std::make_error_code(std::errc::resource_deadlock_would_occur));
    }

    mutex_type* m_mutex;
    bool m_owns;
};

template <template <typename...> typename Class, typename Instance>
struct IsInstance : std::false_type { };

template <template <typename...> typename Class, typename... Args>
struct IsInstance<Class, Class<Args...>> : std::true_type { };

template <template <typename...> typename Class, typename Instance>
constexpr auto IsInstanceV = IsInstance<Class, Instance>::value;

static_assert(IsInstanceV<UniqueLock, UniqueLock<Mutex>>);
static_assert(IsInstanceV<SharedLock, SharedLock<SharedMutex>>);

template <typename Instance, template <typename...> typename... Classes>
struct InstanceOf : std::disjunction<IsInstance<Classes, Instance>...> { };

template <typename LockType>
concept MyLockType = InstanceOf<LockType, UniqueLock, SharedLock>::value;

static_assert(MyLockType<UniqueLock<Mutex>>);
static_assert(MyLockType<SharedLock<SharedMutex>>);
static_assert(!MyLockType<int>);

namespace std
{
    template <::MyLockType LockType>
    inline void swap(LockType& lhs, LockType& rhs) noexcept
    { lhs.swap(rhs); }
}

template <typename Lock1, typename... Locks>
static void TryLockImpl(int& idx, Lock1& lock1, Locks&... locks)
{
    idx ++;
	if constexpr (sizeof...(Locks) > 0)
	{
		UniqueLock<Lock1> lock(lock1, try_to_lock);
		if (lock.owns_lock())
		{
			TryLockImpl(idx, locks...);
			if (idx == -1)
				lock.release();
		}
	}
	else
	{
		UniqueLock<Lock1> lock(lock1, try_to_lock);
		if (lock.owns_lock())
		{
			lock.release();
			idx = -1;
		}
	}
}

template<typename Lock1, typename Lock2, typename... Lock3>
int TryLock(Lock1& l1, Lock2& l2, Lock3&... l3)
{
    int idx = 0;
    TryLockImpl(idx, l1, l2, l3...);
    return idx;
}

template <typename Lock1, typename Lock2, typename... Locks>
void Lock(Lock1& l1, Lock2& l2, Locks&... ls)
{
    // TODO : ... 
    while (1)
    {
        int idx = 0;
        UniqueLock<Lock1> first{l1};
        TryLockImpl(idx, l2, ls...);
        if (idx == -1)
        {
            first.release();
            return;
        }
    }
}

template <typename... MutexTypes>
class ScopedLock
{
public:
    explicit ScopedLock(MutexTypes&... m) : m_mutexes{ m... } 
    { Lock(m...); }

    explicit ScopedLock(AdoptLock, MutexTypes&... m) : m_mutexes{ m... } { }

    ~ScopedLock()
    { std::apply([](auto&... m) { (m.unlock(), ...); }, this->m_mutexes); }

    NOCOPYABLE(ScopedLock)

private:
    std::tuple<MutexTypes&...> m_mutexes;
};

template <typename MutexType>
class ScopedLock<MutexType>  // same as LockGuard
{
public:
    using mutex_type = MutexType;

    NOCOPYABLE(ScopedLock)
    
    ScopedLock(mutex_type& m) : m_mutex{m}
    { this->m_mutex.lock(); }

    ~ScopedLock() 
    { this->m_mutex.unlock(); }

    ScopedLock(AdoptLock, mutex_type& m) : m_mutex{m}
    { }  // calling thread owns mutex

private:
    mutex_type m_mutex;
};

template <>
class ScopedLock<>
{
public:
    DEFAULT_CONSTRACTOR(ScopedLock)

    explicit ScopedLock(AdoptLock) noexcept { }
};

class OnceFlag
{
public:
    using native_type = pthread_once_t;
    
    constexpr OnceFlag() noexcept = default;
    
    NOCOPYABLE(OnceFlag)

    template <typename Callable, typename... Args>
    friend void CallOnce(OnceFlag&, Callable&&, Args&&...);

private:
    native_type m_once = PTHREAD_ONCE_INIT;
};

// template< class Function, class... Args > 
// explicit thread( Function&& f, Args&&... args ) {
//     auto wrapper = new auto([f, args...]{ return f(args...); });
//     using pointertype = decltype(wrapper);
//     using realtype = std::remove_pointer_t<pointertype>;
//     using entry = void (*)(void*);
//     entry invoke_wrapper = [](void* p) { std::unique_ptr<realtype> pf (static_cast<pointertype>(p)); (*pf)(); };
//     thread_create(invoke_wrapper, wrapper);
// }

template <typename Callable, typename... Args>
void CallOnce(OnceFlag& once_flag, Callable&& callable, Args&&... args)
{
    // FIXME:
    thread_local std::function<void()> cstyle_func = [&]() 
    {
        std::invoke(std::forward<Callable>(callable), std::forward<Args>(args)...);
    };
    void (*call_once_warpper)() = []() { cstyle_func(); };
    int e = pthread_once(&once_flag.m_once, call_once_warpper);
    if (e)
        throw std::system_error(std::make_error_code(static_cast<std::errc>(e)));
}

