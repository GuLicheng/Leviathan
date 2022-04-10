import os
from setuptools import setup
from setuptools import Extension

# your complier args
cpp_args = ['/std:c++latest', '/O2']   

# your pybind11 path
includes = [
    "E:\\Anaconda3\\anaconda3\\envs\\segmentation\\Lib\\site-packages\\pybind11\\include",
]

sources_root = "cppextend"
extension_name = "cpp"
sources = [f"{sources_root}/{file}" for file in filter(lambda x: x.endswith(".cpp"), os.listdir(sources_root))]

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