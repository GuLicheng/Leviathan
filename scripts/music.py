from tqdm import tqdm
from filesystem import PathHandler
from convertor import Convertor
from utils import bind_front, pipeline
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
            try:
                audio = AudioSegment.from_file(filename, format=self.handler.extension(filename))
                audio.export(music, format=self.fmt)
            except Exception:
                print(filename)

class MP3MusicWriter(MusicWriter):

    def __init__(self, dest: str) -> None:
        super().__init__(dest, "mp3")

TYPE_DICTIONARY = {
    ".mp3": MP3MusicWriter
}

def convert_music(source: str, target: str = ".", fmt: str = ".mp3"):
    """
        Convert music to target format.

        Args:
            source: Source filename.
            target: Target directory the result will be placed.
            fmt: Target format of music.

        >>> convert_music("QQMusicDownload/music_name.ogg", "target_directory", ".mp3")
    """
    print(f"Converting {source}...")
    TYPE_DICTIONARY[fmt](dest=target)(source)

def convert_musics(source_directory: str, target_directory: str, ext: str, fmt: str):

    musics = pipeline(
        source_directory, os.listdir, 
        bind_front(filter, lambda x: x.endswith(ext)),
        bind_front(map, lambda x: os.path.join(source_directory, x)),
        list
    )

    for music in tqdm(musics):
        convert_music(music, target_directory, fmt)

if __name__ == "__main__":
    
    # Convert .mgg to .ogg
    # https://demo.unlock-music.dev/
    # Github: https://git.unlock-music.dev/um/web


    parser = argparse.ArgumentParser()

    parser.add_argument("--source", type=str, 
                        default=r"E:\QQMusicDownload", help="source files directory")

    parser.add_argument("--target", type=str, 
                        default=r"E:\QQMusicDownload\MP3Songs", help="target files directory")
    
    parser.add_argument("--source_fmt", type=str, help="source file format", default=".ogg")
    parser.add_argument("--target_fmt", type=str, help="target file format", default=".mp3")

    args = parser.parse_args()

    convert_musics(args.source, args.target, args.source_fmt, args.target_fmt)

