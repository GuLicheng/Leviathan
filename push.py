import os   

if __name__ == "__main__":

    instructions = [
        'git add .',
        'git commit -m "^_^"',
        'git push'
    ]

    for instruction in instructions:
        os.system(instruction)