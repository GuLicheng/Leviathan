#include "base.hpp"
#include "bmp.hpp"

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/numpy.h>
#include <vector>

namespace py = pybind11;
namespace lv = leviathan::image;

std::vector<uint8_t> to_list(const char* path)
{
    using BMP = lv::bmp<lv::little_endian>;
    BMP b;
    b.read(path);
    auto buf_info = b.buffer();
    return buf_info.m_data; 
}

PYBIND11_MODULE(lvimage, m)
{
    m.doc() = "test module for lvimage";
    m.def("to_list", &to_list, "None");
}
