from typing import Iterable
import itertools

def chunk(rg: Iterable, size: int):
    ls = []
    for idx, val in enumerate(rg):
        if idx > 0 and idx % size == 0:
            yield list(ls)
            ls.clear()
        ls.append(val)
    if ls:
        yield ls



if __name__ == "__main__":

    print(chunk([1, 2, 3, 4, 5], 2))