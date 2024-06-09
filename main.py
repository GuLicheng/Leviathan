import random
from datetime import datetime
from calendar import Calendar
from typing import List

class AgencyWorkDays:

    def __init__(self, year, month) -> None:
        self.year = year
        self.month = month
        self.calendar = Calendar()
        self.dates = list(self)

    def __iter__(self):
        return filter(
            lambda x: x.month == self.month,
            self.calendar.itermonthdates(self.year, self.month)
        )
    
    def __str__(self):
        return str(self.dates)

    @staticmethod
    def name(weekend):
        names = ("MONDAY", "TUESDAY", "WEDNESDAY", "THURSDAY", "FRIDAY", "SATURDAY", "SUNDAY")
        return names[weekend]

class Employee:

    def __init__(self, id: str, name: str, show_name: bool = False) -> None:
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

class EmployeeWorkday:

    def __init__(self, employee: Employee, workdays: List[int]) -> None:
        self.employee = employee
        self.workdays = workdays

class ScheduleOneDay:

    def __init__(self) -> None:
        self.employees = []

    def add_employee(self, employee: Employee):
        self.employees.append(employee)

    def count(self):
        return len(self.employees)

    def __str__(self):
        result = []
        for idx, employee in enumerate(Employees):
            if employee in self.employees:
                result.append(str(employee))
            else:
                result.append("--------")   
        return str(list(map(str, result)))
            
class Schedule:

    YEAR = 2024
    MONTH = 6

    def __init__(self, schedule_list: List[ScheduleOneDay]):
        self.schedule_list = schedule_list

    def valid(self):
        for day in self.schedule_list:
            if day.count() < 5:
                return False
        return True

    def __repr__(self):
        result, idx = "", 0
        for d in AgencyWorkDays(self.YEAR, self.MONTH):
            result += f"{d.year:4}/{d.month:02}/{d.day:02} "
            if d.weekday() == 6:
                result += f"{'Sunday':9}\n"
            else:
                result += f"{AgencyWorkDays.name(d.weekday()):9} {self.schedule_list[idx]}\n"
                idx += 1
        return result

class ScheduleSelector:

    def __init__(self, employees, employee_workdays, agency_workdays):
        self.employees = employees
        self.employee_workdays = employee_workdays
        self.agency_workdays = agency_workdays
        self.schedule = None

    def adjust_order(self, schedule):
        return True

    def __call__(self):
        schedule_list = [ScheduleOneDay() for _ in range(self.agency_workdays)]
        for employee in Employees:
            workdays = random.sample(range(0, self.agency_workdays), self.employee_workdays)
            for workday in workdays:
                schedule_list[workday].add_employee(employee)
        self.schedule = Schedule(schedule_list)
        return self.schedule

    def __str__(self):
        return str(self.schedule)


if __name__ == "__main__":

    while True:
        schedule = ScheduleSelector(len(Employees), 20, 25)()
        if schedule.valid():
            print(schedule)
            break
        else: 
            continue


