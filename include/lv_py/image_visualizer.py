from typing import Any
import cv2 as cv
import numpy as np

from PIL import Image
import torch

class ImageManager:

    def __init__(self) -> None:
        self.images = []

    def resize(self, img: np.ndarray, image_size: int):
        mode = cv.INTER_NEAREST if len(img.shape) == 2 else cv.INTER_LINEAR  
        return cv.resize(img, dsize=(image_size, image_size), interpolation=mode)

    def normalize(self, img: np.ndarray, eps: float = 1e-5):
        min_val, max_val = img.min(), img.max()
        img = (img - min_val) / (max_val - min_val + eps)
        return img

    def to3channel(self, img: np.ndarray):
        if len(img.shape) == 3:
            assert img.shape[2] in [1, 3]
            return img
        img = (self.normalize(img) * 255).astype(np.uint8)
        img = cv.applyColorMap(img, cv.COLORMAP_JET)
        return img

    def hstack(self, **kwargs):

        image_size = kwargs.get("image_size", 480)
        channel = kwargs.get("channel", 3)
        assert isinstance(image_size, int) and channel in [3, 4]

        images = [self.to3channel(self.resize(image, image_size)) for image in self.images]
        image = np.hstack(images)
        return image

    
    def read_image_from_file(self, filename: str, mode: int = cv.IMREAD_ANYCOLOR):
        self.images.append(cv.imread(filename, mode))

    
    def read_image_from_PIL(self, PIL_image: Image):
        self.images.append(self.convert_rgb2bgr(np.array(PIL_image)))

    def read_mask_from_tensor(self, tensor: torch.Tensor):
        assert len(tensor.shape) == 2
        self.images.append(tensor.numpy())

    def read_images(self, *args):
        for arg in args: self.read_image_from(arg)

    def read_image_from(self, src: Any):
        if isinstance(src, torch.Tensor):
            return self.read_mask_from_tensor(src)
        elif isinstance(src, Image.Image):
            return self.read_image_from_PIL(src)
        elif isinstance(src, str):
            return self.read_image_from_file(src)
        else:
            raise TypeError("Unsupported Type")
    
    def convert_rgb2bgr(self, img: np.ndarray):
        return cv.cvtColor(img, cv.COLOR_RGB2BGR)

if __name__ == "__main__":
    img1 = r"D:\Library\Leviathan\Experiment\images\t01d70f03f64fdb7d37.jpg"
    img2 = r"D:\Library\Leviathan\Experiment\images\t015613cc04cfad2a49.jpg"
    img3 = r"D:\Library\Leviathan\Experiment\images\2007_000129.png"
    img4 = r"D:\Library\Leviathan\Experiment\images\ILSVRC2012_test_00000415.png"

    img2 = Image.open(img2).convert("RGB")

    image_manager = ImageManager()
    image_manager.read_images(img1, img2, img3, img4)
    img = image_manager.hstack()
    cv.imshow("window", img)
    cv.waitKey(0)

