import lvimage.cpp as cpp
import cv2 as cv
import mypy_extensions

def print_all(module_):
    modulelist = dir(module_)
    length = len(modulelist)
    for i in range(0,length,1):
        print(getattr(module_,modulelist[i]))

if __name__ == "__main__":
    print_all(cpp)
    cv.imreadmulti()