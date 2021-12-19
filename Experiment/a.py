import argparse

argparser = argparse.ArgumentParser()
argparser.add_argument("-Hello", type=str, default="World")
argparser.add_argument("--Hello", type=str, default="World")
argparser.add_argument("--Hello2", type=str, required=True)
args = argparser.parse_args()
print(vars(args))