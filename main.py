import toml, json


if __name__ == "__main__":

    root = toml.load("a.toml")

    # print(root)

    print(toml.dumps(root))
