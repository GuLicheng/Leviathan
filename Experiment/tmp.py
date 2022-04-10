import cpp


ls = [1, 2, 3, 4, 5]

def plus_one(x):
    x = x + 1

cpp.for_each(ls, plus_one)
print(ls)
