import os, sys
from distutils.core import setup, Extension
from distutils import sysconfig

cpp_args = ['/std:c++latest']
includes = [
    "E:\\Anaconda3\\anaconda3\\envs\\segmentation\\Lib\\site-packages\\pybind11\\include"
]

ext_modules = [
    Extension(
    'lvimage',
        ['src.cpp'],
        include_dirs=includes,
    language='c++',
    extra_compile_args = cpp_args,
    ),
]

setup(
    name='lvimage',
    version='0.0.1',
    ext_modules=ext_modules,
)

# python setup.py build_ext -i

