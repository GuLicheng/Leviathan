import argparse

if __name__ == "__main__":

    parser = argparse.ArgumentParser()

    parser.add_argument("version", default=1, const=True, nargs='?')

    args = parser.parse_args()

    print(vars(args))
