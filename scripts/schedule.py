from collections import Counter
from typing import Callable, Iterable, List
from calendar import Calendar
import random, itertools, datetime

def sample(agency_workdays: int, employee_workdays: int):
    return random.sample(range(0, agency_workdays), employee_workdays)

def flatten(sequence: Iterable):
    for x in sequence:
        if isinstance(x, Iterable):
            for y in flatten(x):
                yield y
        else:
            yield x

# class ScheduleEmployee:

#     def __init__(self):
#         self.dictionary = dict()
    
#     def __getitem__(self, key):
#         return self.dictionary[key]

#     def __setitem__(self, key, value):
#         self.dictionary[key] = value
    
#     def __str__(self):
#         return str(self.dictionary)

#     def count_workday(self):
#         def closure(x):
#             return x[0], len(x[1])
#         return dict(map(closure, self.dictionary.items()))

class Employee:

    def __init__(self, id: str, name: str, show_name: bool = False):
        self.id = id
        self.name = name
        self.show_name = show_name

    def __str__(self):
        return f"{self.name}" if self.show_name else f"{self.id}"
    
Employees = [
    Employee("37002128", "YZY"), 
    Employee("62516351", "LX"), 
    Employee("71572631", "JHH"), 
    Employee("53037551", "WQ"), 
    Employee("15829913", "PQX"), 
    Employee("29333731", "YWR"), 
    Employee("76101209", "ZQ"),
]

class ScheduleDay:

    def __init__(self, employees: List[Employee] = None):
        self.employees: List[Employee] = employees or [] 

    def add_employee(self, employee: Employee):
        self.employees.append(employee)

    def sort_and_fill(self) -> List[Employee]:
        result = []
        for employee in Employees:
            if employee not in self.employees:
                result.append(None)
            else:
                result.append(employee)
        return ScheduleDay(result)

    def visualize(self) -> str:
        def str_or_none(x):
            return "--------" if x is None else str(x) 
        return str(list(map(str_or_none, self.employees)))
    
    def __str__(self):
        return self.sort_and_fill().visualize()

    def __len__(self):
        return len(self.employees)

class ScheduleMonth:

    def __init__(self, employees: List[Employee], employee_workdays: int, agency_workdays: int):
        self.employees = employees
        self.employee_workdays = employee_workdays
        self.agency_workdays = agency_workdays

    def arrange(self):
        result = [ScheduleDay() for _ in range(0, self.agency_workdays)]
        for employee in self.employees:
            for day in sample(self.agency_workdays, self.employee_workdays):
                result[day].add_employee(employee)
        return result
    
    def valid(self) -> bool:
        x = self.arrange()
        x = map(len, x)
        counter = Counter(x)
        print(counter)
        return True

    def __str__(self):
        return str(list(map(str, flatten(self.arrange()))))

if __name__ == "__main__":

    selector = ScheduleMonth(Employees, 20, 25)
    for x in flatten(selector.arrange()):
        print(x)
    selector.valid()

