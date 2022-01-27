import argparse

"""
通常，argparse 模块会认为 -f 和 --bar 等旗标是指明 可选的 参数，它们总是可以在命令行中被忽略。 
要让一个选项成为 必需的，则可以将 True 作为 required= 关键字参数传给 add_argument():


add_argument() 的``const`` 参数用于保存不从命令行中读取但被各种 ArgumentParser 
动作需求的常数值add_argument() 的``const`` 参数用于保存不从命令行中读取但被各种 ArgumentParser 动作需求的常数值

"""

parser = argparse.ArgumentParser()

parser.add_argument("--v", "-v", "---v", type=str, default="0.0.0")

args = parser.parse_args()

print(vars(args))


