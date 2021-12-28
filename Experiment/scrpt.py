import matplotlib.pyplot as plt
import matplotlib
import argparse
import configparser
import os


if __name__ == "__main__":

    os.system("g++ -std=c++20 test.cpp -O3 -o a | ./a a.txt")

    matplotlib.rcParams['font.sans-serif'] = ['SimHei']
    matplotlib.rcParams['axes.unicode_minus'] = False

    parser = argparse.ArgumentParser()
    parser.add_argument("--key", type=str, default="Random Int")
    args = parser.parse_args()

    ini = configparser.ConfigParser()
    ini.read("a.txt", encoding="utf-8")
    distrbutions = ini.sections()
    dictionary = {}
    for section in distrbutions:
        dictionary[section] = {}
        for option in ini.options(section):
            dictionary[section][option] = ini.get(section, option)

    price = [float(x) for x in dictionary[args.key].values()]
    X = dictionary[args.key].keys()
    plt.barh(range(len(price)), price, height=0.7, color='steelblue', alpha=0.8)  
    plt.yticks(range(len(X)), X)
    plt.xlim(0, 300)
    plt.xlabel("Time: ms")
    plt.title("1e6 numbers")
    for x, y in enumerate(price):
        plt.text(y + 0.2, x - 0.1, '%s' % y)
    plt.show()
