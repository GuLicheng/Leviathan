import os

def generate_all_header(path: str):
    r = os.listdir(path)
    r = filter(lambda x: x.endswith(".hpp") and x != "all.hpp", r)
    return "#pragma once\n\n" + "\n".join([f'#include "{x}"' for x in r])

if __name__ == "__main__":

    generated = generate_all_header("leviathan/extc++")
    print(generated)