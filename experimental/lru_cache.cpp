#include <leviathan/collections/list/intrusive_list.hpp>
#include <leviathan/collections/hook/red_black_hook.hpp>
#include <leviathan/collections/tree/intrusive_tree.hpp>


using cpp::collections::intrusive::list_hook;
using cpp::collections::intrusive::red_black_hook;
using cpp::collections::intrusive::tree;
using cpp::collections::from_hook;

template <typename K, typename V>
struct LRUCacheNode
{
    K key;

    V value;

    [[=cpp::refl::tag("hash")]] 
    red_black_hook treehook;

    [[=cpp::refl::tag("list")]] 
    list_hook listhook;
};

template <typename K, typename V, typename HasherKeyEqual>
class LRUCache
{

    
};

