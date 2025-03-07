import xml.etree.cElementTree as ET
import toml

def main():
    xml = ET.parse("a.xml")
    xml.write("b.xml")

    toml.dump({});

if __name__ == "__main__":

    main()
