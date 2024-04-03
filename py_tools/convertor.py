from abc import abstractmethod
from typing import Callable, Iterable
from tqdm import tqdm
import argparse

class Convertor:

    def __init__(self, dest: str = None, fmt: str = None) -> None:
        self.dest = dest
        self.fmt = fmt

    @abstractmethod
    def __call__(self, filename: str) -> None:
        pass

class FileConvertor:

    def __init__(self, files: Iterable, convertor: Callable) -> None:
        self.files = files
        self.convertor = convertor

    def __call__(self):

        for file in tqdm(self.files):
            self.convertor(file)

def parse(args):
    pass

if __name__ == "__main__":

    parser = argparse.ArgumentParser()
    subparsers = parser.add_subparsers()

    # Music
    music_parser = subparsers.add_parser(name="music")
    music_parser.add_argument("-s", "--src", type=str, required=True)
    music_parser.add_argument("-d", "--dest", type=str, required=True)
    music_parser.add_argument("-f", "--formats", type=str, required=True, nargs='+')

    # Image
    image_parser = subparsers.add_parser(name="image")
    image_parser.add_argument("-s", "--src", type=str, required=True)
    image_parser.add_argument("-d", "--dest", type=str, required=True)
    image_parser.add_argument("-f", "--formats", type=str, required=True, nargs='+')

    args = parser.parse_args(["music", "-s", "HelloWorld", "-f", "mp3", "ogg", "-d", "Other"])

    print(vars(args))