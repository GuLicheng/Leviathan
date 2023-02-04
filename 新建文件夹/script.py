import matplotlib.pyplot as plt


def select_mIoU(log_path):

    keys = "mIoU="

    mIoUs = []

    lines = open(log_path, "r").read().splitlines()
    for line in lines:
        if keys in line:
            idx = line.index(keys)
            line = line[idx + len(keys):]
            line = line[:6]
            mIoUs.append(float(line))

    return mIoUs


r1 = select_mIoU("a.log")
r2 = select_mIoU("b.log")

x_values = range(len(r1))
# x_values = [0, 1.5, 2, 3]
# y_values = [4, 2, 1, 3]
# plt.style.use('ggplot')

plt.plot(x_values, r1, label="GMP", color="red")

plt.plot(x_values, r2, label="GAP", color="blue")

plt.title("GAP vs GMP")
plt.xlabel("Epoch")

plt.ylabel("mIoU")
plt.legend()
plt.grid()
# 设置每个坐标轴的取值范围
plt.axis([0, 30, 0.5, 0.75])

plt.show()
