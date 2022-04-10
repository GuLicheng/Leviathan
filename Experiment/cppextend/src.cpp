#include <pybind11/pybind11.h>


namespace py = pybind11;

class Int
{
    int x;

public:

    Int(int x) : x{x} { }
    int value() const { return x; }
};

PYBIND11_MODULE(cpp, m)
{
    m.doc() = "Test For Cpp Python";

    py::class_<Int>(m, "Int")
        .def(py::init<int>(), py::arg("x"))
        .def("value", &Int::value);
        
}
