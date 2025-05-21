from pyext.cpp2py import avl_tree

if __name__ == "__main__":

    t = avl_tree()

    t[1] = 2

    print(1 in t)
    print(2 in t)

    print(t.get(1))
    print(t.get(0))

    {}.setdefault()