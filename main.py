import toml

if __name__ == "__main__":

    root = toml.load("a.toml")
    
    print(root)

    decoder = toml.decoder.TomlDecoder()