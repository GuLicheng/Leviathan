#include <pybind11/pybind11.h>
#include <Python.h>
#include <set>

#include <lv_cpp/collections/sorted_list.hpp>

#include <memory>

namespace py = pybind11;

using py_sorted_list = sorted_list<py::object, std::less<py::object>, true>;

PYBIND11_MODULE(cpp, m)
{
    m.doc() = "Test For Cpp Python";
    py::class_<py_sorted_list>(m, "py_sorted_list")
        .def(py::init<>())
        .def("insert", &py_sorted_list::insert)
        .def("remove", &py_sorted_list::remove)
        .def("find", &py_sorted_list::find)
        .def("size", &py_sorted_list::size);
}
