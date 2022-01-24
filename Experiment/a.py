import argparse
from cv2 import reduce
import numpy as np
import os
from PIL import Image
import cv2 as cv


PC = cv.imread("./PC.bmp")
PC1 = cv.imread("./PC1.bmp")

assert (PC == PC1).sum() == PC.size
