# -*- coding: utf-8 -*-
import json, toml
from typing import Iterator

SOCIAL_SECURITY_AND_TAX = [
    "住房公积金个人缴费",
    "基本养老保险个人缴费",
    "基本医疗保险个人缴费",
    "失业保险个人缴费",
    "应扣个人所得税",
    "企业年金个人缴费",
]

TAIL = [
    "应发工资", "应扣工资", "实发工资"
]

type Number = float | int

class Salary:

    def __init__(self) -> None:
        self.data: dict[str, list[dict[str, Number]]] = None

    def read(self, source):
        with open(source, "r", encoding="utf-8") as file:
            self.data = json.load(file)
            toml.dump(self.data, open("salary.toml", "w", encoding="utf-8"))
        return self
    
    def __str__(self):
        return str(self.data)    
    
    def chunk(self) -> Iterator[dict[str, Number]]:
        for details in self.data.values():
            for detail in details:
                yield detail 

    def total(self):
        result = {}
        for details in list(self.chunk()):
            for name, number in details.items():
                if name not in result:
                    result[name] = 0.
                result[name] += number

        # Adjust ordered

        print('(')
        for name, number in result.items():
            if name not in TAIL:
                print(f"  {name}:{number:.2f}")
        for name in TAIL:
            print(f"  {name}:{result[name]:.2f}")
        print(')')

if __name__ == "__main__":

    s = Salary().read("salary.json")

    # s.total()
