import cv2
import numpy as np


img = cv2.imread('./lasercut.png', 0)
kernel = np.ones((135, 135), np.uint8)
dilation = cv2.erode(img, kernel, iterations=1)

cv2.imwrite('./lasercut_dilation.png', dilation)

img_diff = cv2.absdiff(dilation, img)
img_diff = cv2.bitwise_not(img_diff)
cv2.imwrite('sabun.png',img_diff)