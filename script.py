import toml


root = toml.load("a.toml")
print(root["lines"])