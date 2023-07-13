import cv2
import numpy as np


img = cv2.imread('./raw.png', 0)
kernel = np.ones((185, 185), np.uint8)
dilation = cv2.erode(img, kernel, iterations=1)

cv2.imwrite('./lasercut.png', dilation)

img_diff = cv2.absdiff(dilation, img)
img_diff = cv2.bitwise_not(img_diff)
cv2.imwrite('sabun.png',img_diff)