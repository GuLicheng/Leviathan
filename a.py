import os
import cv2 as cv
from tqdm import tqdm

PATH = r"F:\DataSet\ade20k\ADEChallengeData2016\images\validation"

images = [f"{PATH}/{file}" for file in os.listdir(PATH)]

h, w = 0, 0

for img in tqdm(images):
    shape = cv.imread(img).shape
    h, w = max(h, shape[0]), max(w, shape[1])

print(h, w)

