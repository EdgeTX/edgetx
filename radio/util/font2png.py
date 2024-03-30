#!/usr/bin/env python
# -*- coding: utf-8 -*-

import sys, os
import glob

from PIL import Image, ImageDraw, ImageFont

chars_en = u""" !"#$%&'()*+,-./0123456789:;<=>?°ABCDEFGHIJKLMNOPQRSTUVWXYZ[\]^_`abcdefghijklmnopqrstuvwxyz~|≥"""
chars_fr = u"""éèàîç"""
chars_da = u"""åæøÅÆØ"""
chars_de = u"""ÄäÖöÜüß"""
chars_cz = u"""ěščřžýáíéňóůúďťĚŠČŘŽÝÁÍÉŇÓÚŮĎŤ"""
chars_es = u"""ÑñÁáÉéÍíÓóÚú"""
chars_fi = u"""åäöÅÄÖ"""
chars_it = u"""àù"""
chars_pl = u"""ąćęłńóśżźĄĆĘŁŃÓŚŻŹ"""
chars_pt = u"""ÁáÂâÃãÀàÇçÉéÊêÍíÓóÔôÕõÚú"""
chars_ru = u"""АБВГДЕЖЗИЙКЛМНОПРСТУФХЦЧШЩЪЫЬЭЮЯабвгдежзийклмнопрстуфхцчшщъыьэюя"""
chars_ua = u"""БГҐДЄЖЗИЇЙЛПУФЦЧШЩЬЮЯбвгґдєжзиїйклмнптфцчшщьюя"""

COUNT_EXTRA_CHARS = 21

chars_extra = u"".join([chr(1+i) for i in range(COUNT_EXTRA_CHARS)])
chars = chars_en + chars_extra + chars_fr + chars_da + chars_de + chars_cz + chars_es + chars_fi + chars_it + chars_pl + chars_pt + chars_ru + chars_ua


def createFontBitmap(filename, fontname, fontsize, fontoffset, foreground, background, coordsfile=True):


    coords = []
    font_filename = 'fonts/' + fontname + '.ttf'

    print("Font filename: %s" % font_filename)
    font = ImageFont.truetype(font_filename, fontsize)

    extraImage = None
    extraFilename = 'fonts/extra_%dpx.png' % fontsize

    try:
        extraImage = Image.open(extraFilename)
        extraImage = extraImage.convert('RGB');

    except IOError:
        print("Missing extra symbol filename %s" % extraFilename)

    def getCharWidth(c):
        if c in chars_extra:
            if extraImage:
                # Extra characters 16px
                #if fontsize == 16:
                if ord(c) <= 9:
                    return 13
                else:
                    return 15
            else:
                return 0
        elif c == " ":
            return 4
        else:
            w, h = font.getsize(c)
            return w + 1

    def getFontWidth():
        width = 0
        for c in chars:
            width += getCharWidth(c)
        return width

    width = getFontWidth()
    image = Image.new('RGB', (width, fontsize + 4), background)
    draw = ImageDraw.Draw(image)

    width = 0
    for c in chars:
        coords.append(width)

        if c in chars_extra:
            if extraImage and c == chars_extra[0]:
                image.paste(extraImage.copy(),(width,1))
        elif c == " ":
            pass
        elif c == "j" and fontsize == 32:
            w, h = font.getsize(c)
            draw.text((width + 2, fontoffset), c, fill=foreground, font=font)
        elif c == "j" and fontsize == 64:
            aw, h = font.getsize(c)
            draw.text((width + 3, fontoffset), c, fill=foreground, font=font)
        else:
            w, h = font.getsize(c)
            draw.text((width + 1, fontoffset), c, fill=foreground, font=font)

        width += getCharWidth(c)

    coords.append(width)

    image.save(filename + ".png")
    if coordsfile:
        with open(filename + ".specs", "w") as f:
            f.write(",".join(str(tmp) for tmp in coords))


# Main
if __name__ == "__main__":
    createFontBitmap(sys.argv[4], sys.argv[1], int(sys.argv[2]), int(sys.argv[3]), (0,0,0), (255,255,255))
