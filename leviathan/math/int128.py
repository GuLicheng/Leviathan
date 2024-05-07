import random
from operator import *

LOWER_ONLY = 0
UPPER_ONLY = 1
LOWER_UPPER = 2

N = 1000 
UINT64_MAX = 2 ** 64 - 1 # 18,446,744,073,709,551,615

div = lambda x, y: x // y

BinaryOp = [
    (add,  '+'),
    (sub,  '-'),
    (mul,  '*'),
    (div,  '/'),
    (mod,  '%'),
    (xor,  '^'),
    (and_, '&'),
    (or_,  '|'),
]

def combine(hi, lo):
    return (hi << 64) | lo

def generate_random_unsigned128():
    hi, lo = 0, 0
    if random.random() > 0.5:
        hi = random.randint(0, UINT64_MAX)
    if random.random() > 0.5:
        lo = random.randint(0, UINT64_MAX)
    return combine(hi, lo)

class BinaryOperation:

    def __init__(self, op) -> None:
        self.op = op
        self.lhs = generate_random_unsigned128()
        self.rhs = generate_random_unsigned128()

    def __call__(self):
        pass


def test():

    op = BinaryOp[random.randint(0, len(BinaryOp) - 1)]
    x1, x2 = generate_random_unsigned128()
    y1, y2 = generate_random_unsigned128()
    x = combine(x1, x2)
    y = combine(y1, y2)
    print(f"x={x}, y={y} and x {op[1]} y = {op[0](x, y)}")

if __name__ == "__main__":

    test()
    test()
    test()