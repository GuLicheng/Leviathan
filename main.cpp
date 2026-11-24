#include <cstddef>
#include <cstdio>
#include <utility>
#include <cassert>
#include <functional>

// --------------------------
// 侵入式配对堆 Hook
// 三指针布局：child / sibling / parent
// 对齐 PostgreSQL 工业级实现，支持任意节点删除与降优先级
// --------------------------
struct pairing_heap_hook
{
    pairing_heap_hook* child{nullptr};
    pairing_heap_hook* sibling{nullptr};
    pairing_heap_hook* parent{nullptr};
};

// --------------------------
// 地址投影：从 hook 指针反向获取宿主业务节点
// --------------------------
template<typename Node, pairing_heap_hook Node::*HookMember>
Node* get_node(pairing_heap_hook* hook) noexcept
{
    if (!hook) return nullptr;
    return reinterpret_cast<Node*>(
        reinterpret_cast<char*>(hook) - offsetof(Node, HookMember)
    );
}

// --------------------------
// 配对堆策略类（Policy）
// 小顶堆语义，比较器与键提取器外部注入
// --------------------------
template<typename Node, pairing_heap_hook Node::*HookMember, typename KeyProj, typename Compare>
struct pairing_heap_policy
{
    Compare comp;
    KeyProj proj;

    pairing_heap_policy(Compare c, KeyProj p)
        : comp(std::move(c)), proj(std::move(p))
    {}

    // 核心原语：合并两个合法配对堆，返回新堆根
    pairing_heap_hook* meld(pairing_heap_hook* a, pairing_heap_hook* b)
    {
        if (!a) return b;
        if (!b) return a;

        Node* na = get_node<Node, HookMember>(a);
        Node* nb = get_node<Node, HookMember>(b);

        // 保证 a 是优先级更高（更小）的根
        if (comp(proj(nb), proj(na)))
        {
            std::swap(a, b);
        }

        // b 作为 a 的第一个孩子插入子链表头部
        b->parent = a;
        b->sibling = a->child;
        if (a->child)
        {
            a->child->parent = b;
        }
        a->child = b;
        return a;
    }

    // 插入单个节点
    pairing_heap_hook* push(pairing_heap_hook* root, pairing_heap_hook* node)
    {
        if (!node) return root;
        node->child = nullptr;
        node->sibling = nullptr;
        node->parent = nullptr;
        return meld(root, node);
    }

    // 获取堆顶节点
    pairing_heap_hook* top(pairing_heap_hook* root)
    {
        return root;
    }

    // 弹出堆顶（最小元素），返回新堆根
    pairing_heap_hook* pop_min(pairing_heap_hook* root)
    {
        if (!root) return nullptr;
        pairing_heap_hook* child_list = root->child;

        // 清空原根的所有指针
        root->child = nullptr;
        root->sibling = nullptr;
        root->parent = nullptr;

        if (!child_list)
        {
            return nullptr;
        }

        // 第一轮：从左向右两两配对，结果头插形成反向链表
        pairing_heap_hook* pair_head = nullptr;
        pairing_heap_hook* cur = child_list;
        while (cur && cur->sibling)
        {
            pairing_heap_hook* n1 = cur;
            pairing_heap_hook* n2 = cur->sibling;
            cur = n2->sibling;

            n1->sibling = nullptr;
            n2->sibling = nullptr;
            pairing_heap_hook* m = meld(n1, n2);
            m->sibling = pair_head;
            pair_head = m;
        }
        // 处理剩余单个节点
        if (cur)
        {
            cur->sibling = pair_head;
            pair_head = cur;
        }

        // 第二轮：从右向左依次合并所有配对后的堆
        pairing_heap_hook* new_root = nullptr;
        cur = pair_head;
        while (cur)
        {
            pairing_heap_hook* next = cur->sibling;
            cur->sibling = nullptr;
            new_root = meld(new_root, cur);
            cur = next;
        }
        return new_root;
    }

    // 降低节点优先级（小顶堆），调用方需先修改节点键值
    pairing_heap_hook* decrease_key(pairing_heap_hook* root, pairing_heap_hook* node)
    {
        if (!root || !node) return root;
        if (root == node) return root;

        // 从父节点的子链表中断开当前节点
        if (node->parent)
        {
            if (node->parent->child == node)
            {
                node->parent->child = node->sibling;
            }
            else
            {
                pairing_heap_hook* prev = node->parent->child;
                while (prev && prev->sibling != node)
                {
                    prev = prev->sibling;
                }
                if (prev)
                {
                    prev->sibling = node->sibling;
                }
            }
        }
        // 更新后继兄弟的父指针
        if (node->sibling)
        {
            node->sibling->parent = node->parent;
        }

        node->parent = nullptr;
        node->sibling = nullptr;
        // 带着子树重新合并回堆
        return meld(root, node);
    }

    // 删除任意节点
    pairing_heap_hook* erase(pairing_heap_hook* root, pairing_heap_hook* node)
    {
        if (!root || !node) return root;
        if (root == node)
        {
            return pop_min(root);
        }

        // 从父链表中断开当前节点
        if (node->parent)
        {
            if (node->parent->child == node)
            {
                node->parent->child = node->sibling;
            }
            else
            {
                pairing_heap_hook* prev = node->parent->child;
                while (prev && prev->sibling != node)
                {
                    prev = prev->sibling;
                }
                if (prev)
                {
                    prev->sibling = node->sibling;
                }
            }
        }
        if (node->sibling)
        {
            node->sibling->parent = node->parent;
        }

        // 取出被删节点的子树
        pairing_heap_hook* sub_tree = node->child;
        // 清空被删节点的所有指针
        node->child = nullptr;
        node->sibling = nullptr;
        node->parent = nullptr;

        // 子树合并回主堆
        return meld(root, sub_tree);
    }

    // 调试校验：递归检查堆序合法性（小顶堆）
    bool is_valid(pairing_heap_hook* root)
    {
        if (!root) return true;
        Node* self = get_node<Node, HookMember>(root);

        for (pairing_heap_hook* c = root->child; c != nullptr; c = c->sibling)
        {
            Node* child_node = get_node<Node, HookMember>(c);
            // 子节点优先级不得高于父节点
            if (comp(proj(child_node), proj(self)))
            {
                return false;
            }
            if (c->parent != root)
            {
                return false;
            }
            if (!is_valid(c))
            {
                return false;
            }
        }
        return true;
    }
};

// --------------------------
// 业务节点示例
// --------------------------
struct Task
{
    uint64_t prio;
    pairing_heap_hook heap_hook;
};

// --------------------------
// 测试用例
// --------------------------
int main()
{
    auto proj = [](Task* t) -> uint64_t { return t->prio; };
    auto comp = std::less<uint64_t>{};

    pairing_heap_policy<Task, &Task::heap_hook, decltype(proj), decltype(comp)> policy(comp, proj);
    pairing_heap_hook* root = nullptr;

    Task a{10};
    Task b{5};
    Task c{20};
    Task d{1};
    Task e{15};

    // 测试 push + 堆序校验
    root = policy.push(root, &a.heap_hook);
    assert(policy.is_valid(root));
    root = policy.push(root, &b.heap_hook);
    assert(policy.is_valid(root));
    root = policy.push(root, &c.heap_hook);
    assert(policy.is_valid(root));
    root = policy.push(root, &d.heap_hook);
    assert(policy.is_valid(root));
    root = policy.push(root, &e.heap_hook);
    assert(policy.is_valid(root));

    // 测试 top：最小值应为 1
    assert(get_node<Task, &Task::heap_hook>(policy.top(root))->prio == 1);

    // 测试 pop_min
    root = policy.pop_min(root);
    assert(policy.is_valid(root));
    assert(get_node<Task, &Task::heap_hook>(policy.top(root))->prio == 5);

    // 测试 decrease_key：将 20 降为 2，应成为新堆顶
    c.prio = 2;
    root = policy.decrease_key(root, &c.heap_hook);
    assert(policy.is_valid(root));
    assert(get_node<Task, &Task::heap_hook>(policy.top(root))->prio == 2);

    // 测试 erase：删除优先级为5的节点
    root = policy.erase(root, &b.heap_hook);
    assert(policy.is_valid(root));
    assert(get_node<Task, &Task::heap_hook>(policy.top(root))->prio == 2);

    // 依次弹出全部元素，校验升序
    uint64_t last = 0;
    while (root)
    {
        uint64_t cur_prio = get_node<Task, &Task::heap_hook>(policy.top(root))->prio;
        assert(cur_prio >= last);
        last = cur_prio;
        root = policy.pop_min(root);
        assert(policy.is_valid(root));
    }

    printf("All test passed!\n");
    return 0;
}
