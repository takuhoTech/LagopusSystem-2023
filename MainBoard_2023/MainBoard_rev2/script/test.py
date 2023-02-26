import cv2

img = cv2.imread("lagopus.png")
img = cv2.cvtColor(img, cv2.COLOR_BGR2GRAY)

ret, img_mono = cv2.threshold(img, 150, 255, cv2.THRESH_BINARY)

img_ada = cv2.adaptiveThreshold(
    img, 255, cv2.ADAPTIVE_THRESH_GAUSSIAN_C, cv2.THRESH_BINARY, 49, 10)

img_result = cv2.bitwise_and(img_mono, img_ada)

cv2.imwrite("mono.png", img_mono)
cv2.imwrite("ada.png", img_ada)
cv2.imwrite("img.png", img_result)
# cv2.imshow("img", th_img)
cv2.waitKey(0)
cv2.destroyAllWindows()
