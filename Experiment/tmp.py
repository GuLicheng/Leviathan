import os
import cv2 as cv
import numpy as np
from os.path import join

def imread(path: str):

    img = cv.imread(path)
    if path.endswith(".jpg"):
        return cv.resize(img, dsize=(224, 224), interpolation=cv.INTER_LINEAR)
    else:
        return cv.resize(img, dsize=(224, 224), interpolation=cv.INTER_NEAREST)

imagenet = r"F:\Paper\imagenet_pred"
dino = r"F:\Paper\dino_pred"
images = r"D:\VOCtrainval_11-May-2012\VOCdevkit\VOC2012\JPEGImages"
gts = r"D:\VOCtrainval_11-May-2012\VOCdevkit\VOC2012\SegmentationClass"

def visual():
    # cv.namedWindow("Window", 0)
    # cv.resizeWindow("Window", 300, 300)

    for name in os.listdir(dino):

        img = imread(join(images, name.replace(".png", ".jpg")))
        gt = imread(join(gts, name))
        image_net_pred = imread(join(imagenet, name))
        dino_pred = imread(join(dino, name))



        h_images1 = np.hstack([img, gt])
        h_images2 = np.hstack([image_net_pred, dino_pred])

        v_image = np.vstack([h_images1, h_images2])

        print(name)
        cv.imshow("Window   ", v_image)

        cv.waitKey(0)

    cv.destroyAllWindows()

selected = [
    "2007_000464",
    "2007_000636",
    "2007_001289",
    "2007_001761",
    "2007_002094",
    "2007_003051",
    "2007_003194",
    "2007_004969",
    "2007_005294",
    "2007_009897",
    "2008_000107",
    "2008_003110",
    "2007_001774",
]

def copy_selected_sample():

    for name in selected:

        os.system(f"cp {os.path.join(images, name)}.jpg ./images/{name}_img.jpg")
        os.system(f"cp {os.path.join(dino, name)}.png ./images/{name}_dino.png")
        os.system(f"cp {os.path.join(imagenet, name)}.png ./images/{name}_imagenet.png")
        os.system(f"cp {os.path.join(gts, name)}.png ./images/{name}_GT.png")

if __name__ == "__main__":

    copy_selected_sample()
    # visual()



"""
2007_000464.png
2007_000636.png
2007_001289.png
2007_001761.png
2007_002094.png
2007_003051.png
2007_003194.png
2007_004969.png
2007_005294.png
2007_009897.png
2008_000107.png
2008_003110.png
"""












