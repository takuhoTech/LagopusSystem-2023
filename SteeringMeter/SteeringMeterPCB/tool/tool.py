from svglib.svglib import svg2rlg
from reportlab.graphics import renderPDF, renderPM
import sys
import os
from PIL import Image
import numpy as np
import cv2

def pil2cv(image):
    ''' PIL型 -> OpenCV型 '''
    new_image = np.array(image, dtype=np.uint8)
    if new_image.ndim == 2:  # モノクロ
        pass
    elif new_image.shape[2] == 3:  # カラー
        new_image = cv2.cvtColor(new_image, cv2.COLOR_RGB2BGR)
    elif new_image.shape[2] == 4:  # 透過
        new_image = cv2.cvtColor(new_image, cv2.COLOR_RGBA2BGRA)
    return new_image

def crop(img): #引数は画像の相対パス
    # Grayscale に変換
    gray = cv2.cvtColor(img, cv2.COLOR_BGR2GRAY)
    # cv2.imshow('gray', gray)

    # 色空間を二値化
    img2 = cv2.threshold(gray, 127, 255, cv2.THRESH_BINARY)[1]
    # cv2.imshow('img2', img2)

    img2 = cv2.bitwise_not(img2)

    # 輪郭を抽出
    contours = cv2.findContours(img2, cv2.RETR_TREE, cv2.CHAIN_APPROX_SIMPLE)[0]

    # 輪郭の座標をリストに代入していく
    x1 = [] #x座標の最小値
    y1 = [] #y座標の最小値
    x2 = [] #x座標の最大値
    y2 = [] #y座標の最大値
    for i in range(1, len(contours)):# i = 1 は画像全体の外枠になるのでカウントに入れない
        ret = cv2.boundingRect(contours[i])
        x1.append(ret[0])
        y1.append(ret[1])
        x2.append(ret[0] + ret[2])
        y2.append(ret[1] + ret[3])

    # 輪郭の一番外枠を切り抜き
    x1_min = min(x1)
    y1_min = min(y1)
    x2_max = max(x2)
    y2_max = max(y2)
    cv2.rectangle(img, (x1_min, y1_min), (x2_max, y2_max), (0, 255, 0), 3)

    crop_img = img2[y1_min:y2_max, x1_min:x2_max]
    # cv2.imshow('crop_img', crop_img)

    crop_img = cv2.bitwise_not(crop_img)

    return crop_img

args = sys.argv
filename = args[1]
filename_without_ext = os.path.splitext(os.path.basename(filename))[0]

#drawing = svg2rlg(filename)
#renderPDF.drawToFile(drawing, filename_without_ext + ".pdf")
#drawing = svg2rlg(filename)
#renderPM.drawToFile(drawing, filename_without_ext + ".png", fmt="PNG",dpi=2048)
drawing = svg2rlg(filename)
img = renderPM.drawToPIL(drawing, dpi=2048, bg=0xffffff, configPIL=None)

img = pil2cv(img)
cv2.imwrite("raw.png",img)

img = crop(img)
cv2.imwrite("in.png",img)

kernel = np.ones((17, 17), np.uint8)
dilation = cv2.erode(img, kernel, iterations=1)
out = cv2.bitwise_not(dilation)
cv2.imwrite('./out.png', out)

img_diff = cv2.absdiff(dilation, img)
img_diff = cv2.bitwise_not(img_diff)
cv2.imwrite('diff.png',img_diff)
