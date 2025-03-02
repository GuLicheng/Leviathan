import json, toml

def main():
    a = json.load(open("salary.json", encoding="utf-8"))
    toml.dump(a, open("salary.toml", "w", encoding="utf-8"))

if __name__ == "__main__":

    # main()
    pass
