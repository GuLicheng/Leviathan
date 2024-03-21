import os   
import argparse

if __name__ == "__main__":

    # argparse.ArgumentParser()

    instructions = [
        'git add .',
        'git commit -m "^_^"',
        'git push'
    ]

    for instruction in instructions:
        os.system(instruction)



        