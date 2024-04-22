waters = [
    [570, 24, 56.50],
    [348, 24, 47.90],
    [4500, 2, 48.90],
    [1000, 15, 61.90],
    [1500, 12, 62.90],
]


if __name__ == "__main__":

    for v, count, cost in waters:
        print(v * count / cost)