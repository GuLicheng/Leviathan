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


if __name__ == "__main__":

    imagenet = r"F:\Paper\imagenet_pred"
    dino = r"F:\Paper\dino_pred"
    images = r"D:\VOCtrainval_11-May-2012\VOCdevkit\VOC2012\JPEGImages"
    gts = r"D:\VOCtrainval_11-May-2012\VOCdevkit\VOC2012\SegmentationClass"

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
















