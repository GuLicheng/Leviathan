import toml, os

def read_toml_file(root):

    dest = open('output.toml', 'w', encoding='utf-8')

    for file in os.listdir(root):
        if file.endswith('.toml'):
            file_path = os.path.join(root, file)
            with open(file_path, 'r', encoding='utf-8') as f:
                toml_data = toml.load(f)
                toml.dump(toml_data, dest)


def main():
    file_path = 'test.toml'
    toml_data = read_toml_file(file_path)
    print(toml_data)

if __name__ == "__main__":
    
    main()