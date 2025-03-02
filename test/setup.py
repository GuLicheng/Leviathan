import os
from setuptools import setup
from setuptools import Extension

# your compiler args
cpp_args = ['/std:c++latest', '/O2']   

# your pybind11 path
includes = [
    "E:\\Anaconda3\\anaconda3\\envs\\segmentation\\Lib\\site-packages\\pybind11\\include",
    "D:\\Library\\Leviathan",
]

extension_name = "cpp2py"
sources = ["cpp2py.cpp"]

ext_modules = [
    Extension(
        extension_name,
        sources=sources,
        include_dirs=includes,
        language='c++',
        extra_compile_args=cpp_args,
    ),
]

setup(
    name=extension_name,
    version='0.0.1',
    ext_modules=ext_modules,
)
# python setup.py build_ext -i | stubgen -m cpp 
# stubgen: pip install mypy
# pybind11: pip install pybind11