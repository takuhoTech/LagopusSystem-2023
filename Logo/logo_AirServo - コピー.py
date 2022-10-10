from PIL import Image, ImageOps, ImageFont, ImageDraw
import os

name = "Iris"
detail = "AIRSERVO by"
detail2 = "@takuhoTech"

#


def scale_to_height(img, height):
    width = round(img.width * height / img.height)
    return img.resize((width, height)), width


def paste(img, y, height, list):
    w = [140]
    for im in list:
        im, width = scale_to_height(im, height)
        img.paste(im, (w[-1], y))
        w.append(w[-1] + width+10)


def alpha_binarization(img):
    width = img.size[0]
    height = img.size[1]
    for x in range(width):
        for y in range(height):
            pixel = img.getpixel((x, y))
            if (pixel[3] < 255):
                img.putpixel((x, y), (0, 0, 0, 0))


dir = os.path.dirname(__file__) + '/'

# robo
robo = Image.open(dir + "robo.jpg")
robo = ImageOps.invert(robo)
robo = robo.point(lambda x: 0 if x < 150 else x)
robo, _ = scale_to_height(robo, 150)

# kicad
kicad = Image.open(dir + "kicad.png")

# canfd
canfd = Image.open(dir + "canfd.jpg")
canfd = ImageOps.invert(canfd)

# rmf-nhk
rmf = Image.open(dir + "rmf-nhk.png")

# abu
abu = Image.open(dir + "abu.jpg")

img = Image.new("1", (475, 152))
img.paste(robo, (0, 0))
paste(img, 76, 75, [abu, canfd])
draw = ImageDraw.Draw(img)
font1 = ImageFont.truetype(dir + "impact.ttf",  75)
font2 = ImageFont.truetype(dir + "impact.ttf",  40)
draw.text((140, -5), name, fill="white", font=font1)
draw.text((255, 3), detail, fill="white", font=font2)
draw.text((250, 38), detail2, fill="white", font=font2)

img.show()
img.save(dir + "logo_AirServo.jpg")
