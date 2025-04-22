from datetime import datetime

if __name__ == "__main__":
    dt2 = datetime(2025, 4, 20)
    dt1 = datetime.now()
    
    delta = dt2 - dt1

    print(delta)
