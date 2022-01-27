import cv2 as cv
import numpy as np
import os
import argparse
from PIL import Image
from tqdm import tqdm
import lvimage

DATA_ROOT = "./images"

def concat_file(root):
    return [f"{root}/{file}" for file in os.listdir(root)]

def ndarray_equal(arr1: np.ndarray, arr2: np.ndarray) -> bool:
    assert arr1.shape == arr2.shape, f"{arr1.shape}, {arr2.shape}"
    assert (arr1 - arr2).sum() == 0, f"{arr1}\n============================\n {arr2}"

def compare_bmp(path1, path2):
    arr1 = np.array(Image.open(path1)).flatten()
    arr2 = np.array(lvimage.to_list(path2)).flatten()
    ndarray_equal(arr1, arr2)

def convert_image(src: str, dest: str):
    Image.open(src).save(f"{dest}", format=dest.split('.')[-1])

def check_bmp(args):
    root = args.root1.replace('\\', '/')
    images1 = [f"{root}/{file}" for file in os.listdir(root)]
    root = args.root2.replace('\\', '/')
    images2 = [f"{root}/{file}" for file in os.listdir(root)]
    for image1, image2 in tqdm(zip(images1, images2), total=len(images1)):
        test_pair(image1, image2)

def test_pair(path1, path2):
    img1 = Image.open(path1)
    img2 = Image.open(path2)
    arr1, arr2 = np.array(img1), np.array(img2)
    # print(arr1, "\n============================\n", arr2)
    assert ndarray_equal(arr1, arr2)

def generate_images(path = r"D:\VOCtrainval_11-May-2012\VOCdevkit\VOC2012\JPEGImages"):
    cnt = 0
    root = path.replace("\\", "/")
    for file in tqdm([f"{root}/{file}" for file in os.listdir(root)]):
        Image.open(file).save(f"./voc1/{cnt}.bmp", format="bmp")
        cnt += 1

def main():
    paths = [
        r"D:\VOCtrainval_11-May-2012\VOCdevkit\VOC2012\SegmentationObject",
        r"D:\VOCtrainval_11-May-2012\VOCdevkit\VOC2012\JPEGImages",
        "./images",
        # r"D:\TrainDataset\TrainDataset\DUTS-TR\GT",
    ]
    for path in paths:
        for file in tqdm(concat_file(path)):
            convert_image(file, "a.bmp")
            compare_bmp("a.bmp", "a.bmp")
    print("OK")



if __name__ == "__main__":
    # convert_image("./images/depth.bmp", "./a.ppm")
    main()

