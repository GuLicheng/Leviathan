
#include "test_container_allocator.hpp"

#include <memory>
#include <utility>
#include <iostream>
#include <assert.h>


template <typename T, typename Allocator = std::allocator<T>>
class LinkList {
public:
    struct ListNodeBase {
        ListNodeBase* next = nullptr;
    };

    struct ListNode : ListNodeBase {
        T value;
    };

    struct ListImpl {
        size_t size = 0;
        ListNodeBase header;
    };

    using alloc_traits = std::allocator_traits<Allocator>;
    using node_alloc_traits = typename std::allocator_traits<Allocator>::template rebind_traits<ListNode>;
    using node_alloc_type = typename std::allocator_traits<Allocator>::template rebind_alloc<ListNode>;
    constexpr static auto NoException = typename alloc_traits::is_always_equal();

    struct Iterator {

        ListNode* node;

        auto operator->() const { return &(node->value); }
    };

public:

    LinkList() = default;

    LinkList(const Allocator& a) : alloc(a) { }

    // ----------------------------- const& -----------------------------
    LinkList(const LinkList& other, const Allocator& a) : alloc(a) {
        assign_from_another_list(other);
    }

    LinkList(const LinkList& other) 
        : LinkList(other, node_alloc_traits::select_on_container_copy_construction(other.alloc)) 
    { }

    LinkList& operator=(const LinkList& other) {
        constexpr auto POCCA = typename alloc_traits::propagate_on_container_copy_assignment();
        if (this != std::addressof(other)) {
            clear();
            if constexpr (POCCA)
                alloc = other.alloc;
            assign_from_another_list(other);
        }
        return *this;
    }

    // ------------------------------- && -------------------------------
    LinkList(LinkList&& other, const Allocator& a) noexcept(NoException) : alloc(a) {
        if (other.alloc == a) {
            impl = std::exchange(other.impl, ListImpl());
        } else {
            move_from_another_list(std::move(other));
            // Each object after moving should not be used again, so 
            // this step is not necessary in C++. 
            // Rust may report an error if we use a moved object.
            other.clear(); 
        }
    }

    LinkList(LinkList&& other) noexcept(NoException) 
        : impl(std::exchange(other.impl, ListImpl())), alloc(std::move(other.alloc)) {
        assert(other.is_empty_list());
    }

    LinkList& operator=(LinkList&& other) noexcept(NoException) {
        constexpr auto POCMA = typename alloc_traits::propagate_on_container_move_assignment();
        if (this != std::addressof(other)) {
            clear();
            if constexpr (POCMA) {
                // Same as LinkList(LinkList&& other)
                alloc = std::move(other.alloc);
                impl = std::exchange(other.impl, ListImpl());
            } else {
                // Same as LinkList(LinkList&& other, const Allocator& a)
                if (alloc == other.alloc) {
                    impl = std::exchange(other.impl, ListImpl());
                } else {
                    move_from_another_list(other);
                    other.clear(); 
                }
            }
        }
        assert(other.is_empty_list() && "We always making other empty.");
        return *this;
    }

    ~LinkList() {
        // std::cout << "~LinkList Called\n";
        clear();
        assert(is_empty_list());
    }



    // ------------------------------- swap -----------------------------
    void swap(LinkList& other) noexcept(NoException) {
        std::swap(impl, other.impl);
        if constexpr (typename alloc_traits::propagate_on_container_swap())
            std::swap(alloc, other.alloc);
    }

    // -------------------------- other functions ------------------------

    Iterator begin() { 
        return Iterator(static_cast<ListNode*>(impl.header.next)); 
    }

    Allocator get_allocator() { return alloc; }

    size_t size() const { return impl.size; }

    ListNodeBase* first_node() { return impl.header.next; }

    // Check whether the list is empty, not only size is 0 but also header.next is null pointer.
    bool is_empty_list() const { return impl.size == 0 && impl.header.next == nullptr; }

    template <typename... Args>
    std::pair<Iterator, bool> insert(Args&&... args) {
        auto node = make_node((Args&&) args...);
        node->next = impl.header.next;
        impl.header.next = node;
        impl.size++;
        return { Iterator(node), true };
    }

    template <typename... Args>
    std::pair<Iterator, bool> emplace(Args&&... args) {
        auto node = make_node((Args&&) args...);
        node->next = impl.header.next;
        impl.header.next = node;
        impl.size++;
        return { Iterator(node), true };
    }

    void clear() {
        auto p = impl.header.next;
        while (p) {
            auto q = p->next;
            drop_node(static_cast<ListNode*>(p));
            p = q;
        }
        impl = ListImpl();
    }

    void show() const {
        for (auto p = impl.header.next; p; p = p->next) {
            std::cout << static_cast<const ListNode*>(p)->value << ' ';
        }
        std::cout << '\n';
    }

// private:

    void assign_from_another_list(const LinkList& other) {
        assert(impl.size == 0 && !impl.header.next && "only for empty list.");
        auto cur = &impl.header;
        for (auto p = other.impl.header.next; p; p = p->next) {
            auto node = make_node(static_cast<ListNode*>(p)->value);
            cur->next = static_cast<ListNodeBase*>(node);
            cur = node;
        }
        impl.size = other.size();
    }

    void move_from_another_list(LinkList& other) {
        assert(impl.size == 0 && !impl.header.next && "only for empty list.");
        auto cur = &impl.header;
        for (auto p = other.impl.header.next; p; p = p->next) {
            auto node = make_node(std::move(static_cast<ListNode*>(p)->value));
            cur->next = static_cast<ListNodeBase*>(node);
            cur = node;
        }
        impl.size = other.size();
    }

    template <typename... Args>
    ListNode* make_node(Args&&... args) {
        node_alloc_type node_alloc(alloc);
        // C-style: malloc + init
        auto node = node_alloc_traits::allocate(node_alloc, 1);
        node->next = nullptr;
        // T may overload operator&, so we use std::addressof to access the address of value.
        node_alloc_traits::construct(node_alloc, std::addressof(node->value), (Args&&) args...);
        return node;
    }

    void drop_node(ListNode* node) {
        node_alloc_type node_alloc(alloc);
        node_alloc_traits::destroy(node_alloc, std::addressof(node->value));
        node_alloc_traits::deallocate(node_alloc, node, 1);
    }

    ListImpl impl;
    [[no_unique_address]] Allocator alloc;
};

// template <typename... Ts>
// using ContainerType = LinkList<Ts...>;

// TEST_CASE("test_propagate_on_container_copy_assignment_on_all_with_same_allocator")
// {
//     test_propagate_on_container_copy_assignment_on_all_with_same_allocator<ContainerType>();
// }

// TEST_CASE("test_propagate_on_container_copy_assignment_on_nocopy_with_same_allocator")
// {
//     test_propagate_on_container_copy_assignment_on_nocopy_with_same_allocator<ContainerType>();
// }

// TEST_CASE("test_propagate_on_container_copy_assignment_on_all_with_different_allocator")
// {
//     test_propagate_on_container_copy_assignment_on_all_with_different_allocator<ContainerType>();
// }

// TEST_CASE("test_propagate_on_container_copy_assignment_on_nocopy_with_different_allocator")
// {
//     test_propagate_on_container_copy_assignment_on_nocopy_with_different_allocator<ContainerType>();
// }

// TEST_CASE("test_propagate_on_container_move_assignment_on_all_with_same_allocator")
// {
//     test_propagate_on_container_move_assignment_on_all_with_same_allocator<ContainerType>();
// }

// TEST_CASE("test_propagate_on_container_move_assignment_on_nomove_with_same_allocator")
// {
//     test_propagate_on_container_move_assignment_on_nomove_with_same_allocator<ContainerType>();
// }

// TEST_CASE("test_propagate_on_container_move_assignment_on_all_with_different_allocator")
// {
//     test_propagate_on_container_move_assignment_on_all_with_different_allocator<ContainerType>();
// }

// TEST_CASE("test_propagate_on_container_move_assignment_on_nomove_with_different_allocator")
// {
//     test_propagate_on_container_move_assignment_on_nomove_with_different_allocator<ContainerType>();
// }

// TEST_CASE("test_propagate_on_container_swap")
// {
//     test_propagate_on_container_swap<ContainerType>();
// }

using TrackedT = tracked<int>;

CreatePropagateTestingWithAllocator(LinkList, checked_allocator, TrackedT)
