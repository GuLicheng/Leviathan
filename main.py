from functools import reduce
import json
import matplotlib.pyplot as plt

def merge_entries(*dicts):

    def merge_two_dicts(dict1, dict2):
        for k, v in dict2.items():
            dict1[k] = dict1.setdefault(k, 0) + v
        return dict1
        
    return reduce(merge_two_dicts, dicts)

def life_insurance(cost_per_year, years, cash_values):
    cost = cost_per_year * years

    retval = []

    for year, cash_value in enumerate(cash_values):
        year += 1
        cur_value = cash_value - cost
        print(f"year = {year} and value = {cur_value / cost / year * 100:.2f}%")
        retval.append(cur_value / cost / year * 100)
    
    return retval

CASH_VALUES = [
    42089,
    119869,
    219857,
    342850,
    489667,
    501673,
    513970,
    526566,
    539470,
    552693,
    566245,
    580139,
    594388,
    609007,
    624009,
    639411,
    655231,
    671488,
    688201,
    705392,
    723027,
    741102,
    759630,
    778620,
    798085,
    818037,
    838487,
    859448,
    880934,
    902956,
    948666,
    972381,
    996689,
    1021604,
    1047142,
    1073317,
    1100147,
    1127647,
    1155833,
    1184724,
    1214336,
]

class Plot:

    def __init__(self, xlabel = "unknown", ylabel = "unknown"):
        self.xlabel = xlabel
        self.ylabel = ylabel
        self.scaleX = None
        self.horizon = []

    def set_scaleX(self, scales):
        self.scaleX = scales

    def add_horizon(self, value):
        self.horizons.append(value)

    def add_points(self, points):
        self.points.append(points)

    def show(self):
        left, right = min(self.scaleX), max(self.scaleX)
        scaleX = [left, right]
        for horizon in self.horizons:
            plt.plot(scaleX, [horizon] * 2, "o-", color="b", label="line")
        plt.legend(loc="best")
        plt.show()


if __name__ == "__main__":

    retval = life_insurance(100000, 5, CASH_VALUES) 
    retval = retval[5:]
    length = len(retval)

    plt.plot(range(5, 5 + len(retval)), retval, "o-", color="red", label="cash_value")
    plt.plot([0, len(retval) + 5], [1.35] * 2, "o-", color="cyan", label="1.35")
    plt.plot([0, len(retval) + 5], [1.45] * 2, "o-", color="blue", label="1.45")
    plt.plot([0, len(retval) + 5], [1.9] * 2, "o-", color="orange", label="1.9")
    plt.legend(loc="best")
    plt.show()

    pass