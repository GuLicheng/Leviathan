import xml.etree.cElementTree as ET

def main():
    xml = ET.parse("a.xml")
    xml.write("b.xml")


if __name__ == "__main__":

    main()
