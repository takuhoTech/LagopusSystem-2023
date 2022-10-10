from PIL import Image, ImageOps, ImageFont, ImageDraw
import os

year = "2023"

dir = os.path.dirname(__file__) + '/'

img = Image.new("1", (300, 300))
draw = ImageDraw.Draw(img)
font1 = ImageFont.truetype(dir + "kronika.ttf",  120)
font2 = ImageFont.truetype(dir + "kronika.ttf",  100)
draw.text((15, 10), "ABU" , fill ="white", font=font1)
draw.text((20, 140), year, fill ="white", font=font2)
draw.line([(20,10),(280,10),(290,20),(290,280),(280,290),(20,290),(10,280),(10,20),(20,10)], fill = "white", joint="curve", width = 7)

img.show()
img.save(dir+"abu.jpg")