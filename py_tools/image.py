from PIL import Image
from convertor import Convertor
from filesystem import PathHandler
import os

class ImageConvertor(Convertor):

    def __init__(self, dest: str, fmt: str) -> None:
        super().__init__(dest, fmt)
        self.handler = PathHandler(self.fmt, self.dest)
        
    def __call__(self, filename: str) -> None:
        image = self.handler(filename)
        if not os.path.exists(image):
            Image.open(filename).save(image)

class ReduceImage(Convertor):

    def __init__(self, dest: str, quality: int = 85) -> None:
        super().__init__(dest, None)
        self.quality = quality
        self.handler = PathHandler(self.fmt, self.dest)

    def __call__(self, filename: str) -> None:
        image = self.handler(filename)         
        if not os.path.exists(image):
            Image.open(filename).save(image, quality=self.quality)

class JPEGConvertor(ImageConvertor):

    def __init__(self, dest: str) -> None:
        super().__init__(dest, "jpeg")

class PNGConvertor(ImageConvertor):

    def __init__(self, dest: str) -> None:
        super().__init__(dest, "png")

class BMPConvertor(ImageConvertor):

    def __init__(self, dest: str) -> None:
        super().__init__(dest, "bmp")

IMAGES_TYPES = {
    "jpeg": JPEGConvertor,
    "bmp": BMPConvertor,
    "png": PNGConvertor,
}

if __name__ == "__main__":

    from argparse import ArgumentParser

    parser = ArgumentParser()

    parser.add_argument("--src", type=str, required=True)
    parser.add_argument("--dest", type=str, required=True)
    parser.add_argument("--format", type=str, required=True)

    args = parser.parse_args([
        "--src", r"D:\Library\Leviathan\py_tools\IMG_7571(20240402-181856).JPG", 
        "--dest", "./Result", 
        "--format", "png",
    ])

    # print(vars(args))
    # exit()

    if not os.path.exists(args.dest):
        os.mkdir(args.dest)

    IMAGES_TYPES[args.format](args.dest)(args.src)


