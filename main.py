import xml.etree.cElementTree as ET
import toml, time, datetime

def main():
    xml = ET.parse("a.xml")
    xml.write("b.xml")


if __name__ == "__main__":

    datetime.datetime.now()

    main()
