from utils import *


class Item:

    def __init__(self, line: str) -> None:
        
        assert len(line) > 0, f"Empty line is not allowed"

        name_and_pro = line.split('=')

        self.name = name_and_pro[0]
        self.pros = name_and_pro[1].split(',')

    def show_name(self):
        self.pros[-1] = 1

    def __str__(self) -> str:
        return f"{self.name}={','.join(map(str, self.pros))}"


def show_item_name(config, text_writer):

    combine(open(config).read().splitlines(), bind_front(filter, lambda s: len(s) > 0))

    items = [Item(line) for line in open(config).read().splitlines()]

    for item in items:
        item.show_name()
        text_writer.write(f"{item}\n")

if __name__ == "__main__":

    show_item_name(r"D:\WolServer\传奇世界\Data\config\100_传奇世界_落木\bestitem.ini", open("bestitem.txt", "w"))    
