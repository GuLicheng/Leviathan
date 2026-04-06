import pandas as pd
import click, argparse

def main():
    df = pd.read_csv('salary.txt', sep='\t', encoding='utf-8')
    df.to_excel('salary.xlsx', index=False, header=True)
    parser = argparse.ArgumentParser(description='Convert salary.txt to salary.xlsx')
    parser.add_argument('--input', '-i', default='salary.txt', help='Input text file')

if __name__ == "__main__":
    main()