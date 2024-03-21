import toml
import argparse

# argparse.ArgumentParser().add_argument(help=)


root = toml.load("a.toml")
print(root)