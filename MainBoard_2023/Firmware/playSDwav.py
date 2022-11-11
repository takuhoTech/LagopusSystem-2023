'''
Raspberry Pi Pico, décodeur I2S UDA1334A et lecteur
de cartes SD.
Les fichiers .wav présent sur la carte SD sont lus
tour à tour.
Pour plus d'infos, consultez le blog:
https://electroniqueamateur.blogspot.com/2022/08/jouer-de-la-musique-fichiers-wav-avec.html
'''

import os
import time
from machine import Pin, SPI
from wavplayer import WavPlayer # https://github.com/miketeachman/micropython-i2s-examples
from sdcard import SDCard # https://github.com/micropython/micropython/blob/master/drivers/sdcard/sdcard.py

# lecteur de carte SD
cs = Pin(13, Pin.OUT)
spi = SPI(1, baudrate=1_000_000,  polarity=0,
    phase=0, bits=8, firstbit= SPI.MSB,
    sck=Pin(14), mosi=Pin(15), miso=Pin(12),)
sd = SDCard(spi, cs)
sd.init_spi(25_000_000) 
os.mount(sd, "/sd")

# module décodeur I2S
wp = WavPlayer(id=0, sck_pin=Pin(16),ws_pin=Pin(17),
    sd_pin=Pin(18),ibuf=40000,)

# on joue chaque fichier .wav présent sur la carte SD
for filename in (os.listdir("/sd")):
    print("En train de jouer: ", filename)
    #wp.play(filename, loop=False)
    wp.play("1.wav", loop=False)
    while wp.isplaying() == True:
        pass

os.umount("/sd") 