import cv2 as cv
import numpy as np

size = 500, 500
B = np.zeros(size).reshape(size + (1, ))
G = np.zeros(size).reshape(size + (1, ))
R = np.zeros(size).reshape(size + (1, ))

color = np.concatenate([B, G, R], axis=2)

color[:, :, 0] = 41 / 255
color[:, :, 1] = 50 / 255
color[:, :, 2] = 61 / 255
print(color.shape)
cv.imshow("", color)
cv.waitKey(0)