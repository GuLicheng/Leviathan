from pyext.cpp2py import avl_tree
from collections import Counter
import json

if __name__ == "__main__":

    t = avl_tree()

    t[1] = 2

    print(1 in t)
    print(2 in t)

    print(t.get(1))
    print(t.get(0))

    obj = {
        "name": "Alice",
        "age": 30,
        "is_student": False,
        "grades": [85, 90, 78],
        "address": {
            "street": "123 Main St",    
            "city": "Wonderland",
            "zip": "12345"
        }
    }


    json_str = json.dumps(obj, indent=4)
    print(json_str)
