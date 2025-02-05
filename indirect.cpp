// https://isocpp.org/files/papers/P3019R11.pdf
#include <memory>

#if 1

template <typename T, typename Allocator = std::allocator<T>>
class indirect
{
public:

    using value_type = T;
    using allocator_type = Allocator;
    using pointer = typename std::allocator_traits<Allocator>::pointer;
    using const_pointer = typename std::allocator_traits<Allocator>::const_pointer;

    explicit constexpr indirect() 
        requires (std::is_default_constructible_v<T> 
               && std::is_copy_constructible_v<T> 
               && std::is_default_constructible_v<Allocator>)
        : indirect(std::allocator_arg, Allocator())
    { }

    explicit constexpr indirect(std::allocator_arg_t, const Allocator& a)
        requires (std::is_default_constructible_v<T> && std::is_copy_constructible_v<T>)
        : m_alloc(a)
    {
        allocate_and_construct_object();
    }

    constexpr indirect(const indirect& other) requires (std::is_copy_constructible_v<V>)
        : indirect(std::allocator_arg, std::allocator_traits<Allocator>::select_on_container_copy_construction(other.m_alloc), other)
    { }

    constexpr indirect(std::allocator_arg_t, const Allocator& a, const indirect& other)
        requires (std::is_copy_constructible_v<T>)
        : m_alloc(a)
    {
        if (!other.valueless())
        {
            allocate_and_construct_object(*other);
        }
    }

    constexpr indirect(indirect&& other) noexcept
        : m_alloc(std::move(other.m_alloc))
    {
        if (!other.valueless())
        {
            allocate_and_construct_object(*std::move(other));
            other.make_self_valueless();
        }
    }

    constexpr indirect(std::allocator_arg_t, const Allocator& a, indirect&& other) 
        noexcept(std::allocator_traits<Allocator>::is_always_equal::value)
        : m_alloc(a)
    {
        if (!other.valueless())
        {
            if (m_alloc == other.m_alloc)
            {
                std::swap(m_p, other.m_p);
            }
            else
            {
                allocate_and_construct_object(*std::move(other));
            }
            other.make_self_valueless();
        }
    }

    template <typename U = T>
    explicit constexpr indirect(U&& u) 
        requires (!std::is_same_v<std::remove_cvref_t<U>, indirect> 
               && !std::is_same_v<std::remove_cvref_t<U>, std::in_place_t>
               && std::is_constructible_v<T, U>
               && std::is_copy_constructible_v<T>
               && std::is_default_constructible_v<Allocator>)
    {
        allocate_and_construct_object((U&&)u);
    }

    template <typename U = T>
    explicit constexpr indirect(std::allocator_arg_t, const Allocator& a, U&& u)
        requires (!std::is_same_v<std::remove_cvref_t<U>, indirect> 
               && !std::is_same_v<std::remove_cvref_t<U>, std::in_place_t>
               && std::is_constructible_v<T, U>
               && std::is_copy_constructible_v<T>)
        : m_alloc(a)
    {
        allocate_and_construct_object((U&&)u);
    }

    template <typename... Us>
    explicit constexpr indirect(std::in_place_t, Us&&... us)
        requires (std::is_constructible_v<T, Us...>
               && std::is_copy_constructible_v<T>
               && std::is_default_constructible_v<Allocator>)
    {
        allocate_and_construct_object((Us&&)us...);
    }

    template <typename... Us>
    explicit constexpr indirect(std::allocator_arg_t, const Allocator& a, std::in_place_t, Us&& ...us)
        requires (std::is_constructible_v<T, Us...>
               && std::is_copy_constructible_v<T>)
        : m_alloc(a)
    {
        allocate_and_construct_object((Us&&)us...);
    }

    constexpr ~indirect()
    {
        make_self_valueless();
    }

    // constexpr indirect& operator=(const indirect& other);
    // constexpr indirect& operator=(indirect&& other);

    // template<class I, class... Us>
    // explicit constexpr indirect(in_place_t, initializer_list<I> ilist,
    // Us&&... us);

    // template<class I, class... Us>
    // explicit constexpr indirect(allocator_arg_t, const Allocator& a,
    // in_place_t, initializer_list<I> ilist,
    // Us&&... us);

    // constexpr const T& operator*() const & noexcept;
    // constexpr T& operator*() & noexcept;
    // constexpr const T&& operator*() const && noexcept;
    // constexpr T&& operator*() && noexcept;

    template <typename Self>
    auto&& operator*(Self&& self) noexcept
    {
        return std::forward_like<Self>(*self.m_p);
    }

    constexpr const_pointer operator->() const noexcept
    {
        return m_p;
    }

    constexpr pointer operator->() noexcept
    {
        return m_p;
    }
    
    constexpr bool valueless_after_move() const noexcept
    {
        return valueless();
    }

    constexpr allocator_type get_allocator() const noexcept
    {
        return m_alloc;
    }

    constexpr void swap(indirect& other);

private:

    template <typename... Args>
    void allocate_and_construct_object(Args&&... args)
    {
        m_p = std::allocator_traits<Allocator>::allocate(m_alloc, 1);
        std::allocator_traits<Allocator>::allocate(m_alloc, m_p, (Args&&)args...);
    }

    constexpr void make_self_valueless()
    {
        std::allocator_traits<Allocator>::destroy(m_alloc, m_p, 1);
        std::allocator_traits<Allocator>::deallocate(m_alloc, m_p);
        m_p = nullptr;
    }

    constexpr bool valueless() const noexcept
    {
        return m_p == nullptr;
    } 

    pointer m_p = nullptr;
    [[no_unique_address]] Allocator m_alloc = Allocator();
};

template <class Value>
indirect(Value) -> indirect<Value>;

template <class Allocator, class Value>
indirect(std::allocator_arg_t, Allocator, Value) -> 
    indirect<Value, typename std::allocator_traits<Allocator>::template rebind_alloc<Value>>;

#endif

int main(int argc, char const *argv[])
{
    
    return 0;
}
