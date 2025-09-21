import toml, os
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

if __name__ == "__main__":
    
    main()