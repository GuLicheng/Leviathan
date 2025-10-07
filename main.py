import toml, os, json
from PIL import Image

def read_toml_file(root):

    dest = open('output.toml', 'w', encoding='utf-8')

    for file in os.listdir(root):
        if file.endswith('.toml'):
            file_path = os.path.join(root, file)
            with open(file_path, 'r', encoding='utf-8') as f:
                toml_data = toml.load(f)
                toml.dump(toml_data, dest)


def main():
    Image.open("lena.bmp").save("lena.png")

    toml.dump(json.load(open("salary.json", 'r', encoding='utf-8')), open("salary.toml", 'w', encoding='utf-8'))

if __name__ == "__main__":
    
    main()