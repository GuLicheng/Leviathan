import argparse

# parser = argparse.ArgumentParser()
# parser.add_argument("--lr", type=float)
# parser.add_argument("epoch", type=str)
# parser.add_argument("weight", type=str)

# args = parser.parse_args()
# print(vars(args))

from PIL import Image

png = Image.open("./2007_000039.png")
png.save("./PC.bmp", format="bmp")

