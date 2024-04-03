from typing import List, Iterator, Tuple
import os

class PathHandler:
    """
        A helper class for replacing the filename and extension.
    """
    
    BACKSLASH = '\\'
    SLASH = '/'
    DOT = '.'

    def __init__(self, ext: str = None, dest: str = None) -> None:
        self.ext = ext
        self.dest = dest

    def __call__(self, filename: str) -> str:
        filename = PathHandler.replace_backslash_with_slash(filename)
        roots, name, ext = PathHandler.split_name(filename)

        def exchange(x, y):
            return x if x is not None else y

        return os.path.join(exchange(self.dest, roots), name) + f".{exchange(self.ext, ext)}"

    @staticmethod
    def split_name(filename: str) -> Tuple[str, str, str]:
        filename = PathHandler.replace_backslash_with_slash(filename)
        
        idx1 = filename.rfind(PathHandler.SLASH)
        idx2 = filename.rfind(PathHandler.DOT)

        return filename[:idx1], filename[idx1 + 1:idx2], filename[idx2 + 1:]

    @staticmethod
    def replace_backslash_with_slash(filename: str) -> str:
        return filename.replace(PathHandler.BACKSLASH, PathHandler.SLASH)
           
    @staticmethod
    def extension(filename: str) -> str:
        idx = filename.rfind(PathHandler.DOT)
        return filename[idx + 1:]

@DeprecationWarning
class Files:

    def __init__(self, root: str, *, exts: List[str] = None) -> None:
        self.root = root

        self.files = []

        self.read(self.root)

        self.files = map(PathHandler.replace_backslash_with_slash, self.files)

        if exts is not None:

            def closure(name: str):
                for ext in exts:
                    if name.endswith(ext):
                        return True
                return False

            self.files = filter(closure, self.files)

        self.files = list(self.files)

    def __iter__(self):

        class FilesIterator(Iterator):

            def __init__(self, files, idx) -> None:
                self.files = files
                self.idx = idx

            def __next__(self):
                
                if self.idx == len(self.files):
                    raise StopIteration()
            
                current = self.files[self.idx]
                self.idx += 1

                return current
            
            def __iter__(self):
                return self
            
        return FilesIterator(self.files, 0)

    def __repr__(self) -> str:
        return repr(self.files)

    def read(self, root: str):

        for file in os.listdir(root):

            if os.path.isdir(file):
                self.read(os.path.join(root, file))
            else:
                self.files.append(os.path.join(root, file))

if __name__ == "__main__":

    current = r"D:\Library\Leviathan\py_tools\filesystem.py"

    print(PathHandler.replace_backslash_with_slash(current))
    print(PathHandler.split_name(current))
    print(list(Files(r"D:\Library\Leviathan\py_tools")))