from filesystem import PathHandler
from convertor import Convertor
from pydub import AudioSegment
import argparse
import os

class MusicWriter(Convertor):

    def __init__(self, dest: str, fmt: str) -> None:
        super().__init__(dest, fmt)
        self.handler = PathHandler(self.fmt, self.dest)

    def __call__(self, filename: str) -> None:
        
        music = self.handler(filename)

        if not os.path.exists(music):
            audio = AudioSegment.from_file(filename, format=self.handler.extension(filename))
            audio.export(music, format=self.fmt)

class MP3MusicWriter(MusicWriter):

    def __init__(self, dest: str) -> None:
        super().__init__(dest, "mp3")

TYPE_DICTIONARY = {
    "mp3": MP3MusicWriter
}

def get_parser(args):

    return list(map(lambda x: TYPE_DICTIONARY[x](args.dest), args.formats))

if __name__ == "__main__":
    
    music = r"E:\QQMusicDownload\春涧 - 浅影阿.ogg"
    MP3MusicWriter(".")(music)
