import pandas as pd

def main():
    df = pd.read_csv('salary.txt', sep='\t', encoding='utf-8')
    df.to_excel('salary.xlsx', index=False, header=True)


if __name__ == "__main__":
    main()