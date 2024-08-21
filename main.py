import toml, json


if __name__ == "__main__":

    root = toml.load("a.toml")

    print(root)

    print(toml.dumps(root))

# {"supertable": {"subtable": {}, "subarray": [{}]}, "superarray": [{"subtable": {}, "subarray": [{}, {}]}]}
# {'supertable': {'subtable': {}, 'subarray': [{}]}, 'superarray': [{'subtable': {}, 'subarray': [{}, {}]}]}