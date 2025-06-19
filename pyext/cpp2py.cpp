// python setup.py build_ext -i | stubgen -m cpp 
// stubgen: pip install mypy
// pybind11: pip install pybind11

#include <format>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/stl_bind.h>
#include <leviathan/stopwatch.hpp>
#include <leviathan/collections/tree/avl_tree.hpp>
#include <leviathan/algorithm/all.hpp>

namespace py = pybind11;

struct PyObjectHashEqual
{
    static bool operator()(py::object lhs, py::object rhs) 
    {
        return lhs.equal(rhs);
    }

    static py::size_t operator()(py::object obj) 
    {
        return obj.attr("__hash__")().cast<py::size_t>();
    }
};

struct PyObjectLess
{
    static bool operator()(py::object lhs, py::object rhs) 
    {
        return lhs < rhs;
    }
};

using CppAvlTreeMap = cpp::collections::avl_treemap<py::object, py::object, PyObjectLess>;
using CppAvlTreeMapIterator = CppAvlTreeMap::iterator;

// class AVLTreeIterator
// {
// protected:

//     using iterator_category = std::forward_iterator_tag;
//     using value_type = std::pair<handle, handle>;
//     using reference = const value_type; // PR #3263
//     using pointer = arrow_proxy<const value_type>;
// };

// void tim_sort(py::list ls)
// {
//     cpp::ranges::tim_sort(ls.begin(), ls.end(), {}, {});
// }

// void intro_sort(py::list ls)
// {
//     cpp::ranges::intro_sort(ls.begin(), ls.end(), {}, {});
// }

class AVLTree
{
public:

    void clear()
    {
        tree.clear();
    }

    void __setitem__(py::object key, py::object value)
    {
        tree[key] = value;
    }

    bool __contains__(py::object key) const
    {
        return tree.contains(key);
    }

    std::size_t __len__() const
    {
        return tree.size();
    }

    std::string __str__() const
    {
        return "<AVLTree>";
    }

    py::object get(py::object key, py::object default_value) const
    {
        auto it = tree.find(key);

        if (it != tree.end())
        {
            return it->second;
        }
        
        return default_value;
    }

    void setdefault(py::object key, py::object default_value)
    {
        tree.try_emplace(key, default_value);
    }

private:

    CppAvlTreeMap tree;
};

PYBIND11_MODULE(cpp2py, m)
{
    py::class_<AVLTree>(m, "avl_tree")
        .def(py::init<>(), "Create an empty AVL tree")
        .def("__str__", &AVLTree::__str__, "Get string representation of the AVL tree")
        .def("__contains__", &AVLTree::__contains__, py::arg("key"), "Check if the AVL tree contains a key")
        .def("__len__", &AVLTree::__len__, "Get th‘e number of elements in the AVL tree")
        .def("get", &AVLTree::get, py::arg("key"), py::arg("default_value") = py::none(), "Get the value for a key in the AVL tree")
        .def("setdefault", &AVLTree::setdefault, py::arg("key"), py::arg("default_value"), "Set the default value for a key in the AVL tree")
        .def("__setitem__", &AVLTree::__setitem__, py::arg("key"), py::arg("value"), "Set the value for a key in the AVL tree")
        .def("clear", &AVLTree::clear, "Remove all elements from the AVL tree");

    // m.def("tim_sort", &tim_sort);
    // m.def("intro_sort", &intro_sort);
}


