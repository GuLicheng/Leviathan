对于 BOOST 的 intrusive 来说，里面的 tree 都是异构的容器。
异构容器在进行 lower_bound 操作的时候，往往需要传入一个 Compare 来帮助容器进行比较，比较在这类容器里面，存储的是 Hook/Node* ,但是用户层面传入的是一个 Key(maybe int or std::string),这个时候一个常见的方式就是:

```cpp
class Compare
{
    bool operator()(Node*, int);
    bool operator()(int, Node*);
};
```

通过 Compare 来处理这种内部的差异。我想知道的是，有没有什么好的办法或者说相对优雅的方式来实现呢？

这个问题我也考虑过，依旧存在一个小疑问，就是我们的Projection解决的是Node到Key的映射，但是对于 C++ STL 来说 std::map 存储的是

`std::pair<const Key, Value>`

但是对于异构容器来说，可能结构如下:

```cpp
class MyObject
{
    int key1;
    int key2;
    void* data;

    bool operator<(const MyObject& rhs) const
    {
        return std::make_pair(key1, key2) < std::make_pair(key1, key2);
    }
};
```

就是说我们可能需要多个字段参与比较，如果对于 std::map，我们可以直接写出 `std::map<std::pair<int, int>, void*>`，但是显然对于侵入式容器来说，不太可能提供一个 Projection 来解决这个映射的问题。


按照这套设计，模板参数写成那些比较合适

```cpp
template <std::meta::info MemberHook, typename Compare = std::less<>, bool UniqueKey = true>
class tree;

class MyObject
{
    int key1;
    int key2;
    void* data;

    SomeTreeHook hook;

    bool operator<(const MyObject& rhs) const
    {
        return std::make_pair(key1, key2) < std::make_pair(key1, key2);
    }
};
```

目前来说我们的模板参数声明如上

- MemberHook 可以利用 std::meta::info 计算 offset 来获得对象的起始地址，甚至是对象类型 `parent(MyObject)`，比如传入 `^^MyObject::hook`
- Compare 和 STL 保持一致
- UniqueKey 用来后期快速扩展 multiset/multimap，这个参数先放在这不做过多考虑

需要补充哪些？


但是实际上根本不可能是 `std::identity` ,因为侵入式的容器基本上肯定有其他字段的，比如 `MyObject`。就算真的没有其他字段，里面肯定也包含了 `hook`