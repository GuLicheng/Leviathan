from functools import reduce
from operator import add
import argparse

DIGITS = "0123456789"

def odd(x: int):
    quotient, remainder = divmod(x * 2, 10)
    return quotient + remainder

def even(x: int):
    return x

def Luhn(numbers: str):
    numbers = list(map(int, numbers))
    def closure(x):
        idx, digit = x
        return odd(digit) if idx % 2 == 0 else even(digit)    
    result = reduce(add, map(closure, enumerate(reversed(numbers)))) % 10
    return 0 if result == 0 else 10 - result

def fill_dash(numbers: str):

    if '-' in numbers:
        for ch in DIGITS:
            yield numbers.replace('-', ch)
    else:
        yield numbers

def fill_star(numbers: str):
    if '*' in numbers:
        for ch in DIGITS:
            idx = numbers.index('*')
            for number in fill_star(f"{numbers[:idx]}{ch}{numbers[idx + 1:]}"):
                yield number
    else:
        yield numbers

def generate_account(numbers: str, expected: int):
    for s1 in fill_star(numbers):
        for s2 in fill_dash(s1):
            if Luhn(s2) == expected:
                print(f"{s2}{expected}")


if __name__ == "__main__":  

    parser = argparse.ArgumentParser()

    parser.add_argument("--account", type=str, default="6214881630**888")
    parser.add_argument("--parity", type=int, default=8)

    args = parser.parse_args()

    generate_account(args.account, args.parity)


