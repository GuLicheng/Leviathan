import dis

class Foo:

    def __lt__(self, rhs):
        print("__le__")

    def __eq__(self, rhs):
        print("__eq__")

    def __le__(self, rhs):
        print("__le__")

def func1():


    foo1, foo2 = Foo(), Foo()

    foo1 < foo2
    foo1 == foo2


    foo1 > foo2 # call __le__
    foo1 <= foo2
    foo1 != foo2
    foo1 >= foo2

    print(1 == None)
    print(1 != None)
    print(1 == 1.0)
    

def func2(c):
    a = c + 1.0
    # print(None + 1)
    # print(None + 1.0)
    return a

if __name__ == "__main__":
    func2(1)
    # dis.dis(func2)