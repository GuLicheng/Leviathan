import itertools


if __name__ == "__main__":
    ls = [1, 2, 2, 3]

    for val in itertools.combinations(ls, 2):
        print(val)
