#!/usr/bin/env python3

# certifi==2021.5.30
# charset-normalizer==2.0.4
# idna==3.2
# Pillow==8.3.1
# requests==2.26.0
# urllib3==1.26.6

import requests
import os
from PIL import Image

logo_filename = "edgetx-logo.png"

PROJECT_ROOT = os.path.join(os.path.dirname(__file__), "..")
images_dir = os.path.abspath(os.path.join(PROJECT_ROOT, "companion", "src", "images"))
win_icons_dir = os.path.abspath(os.path.join(PROJECT_ROOT, "companion", "src", "images", "winicons"))
linux_icons_dir = os.path.abspath(os.path.join(PROJECT_ROOT, "companion", "src", "images", "linuxicons"))
start_dir = os.getcwd()

if not os.path.exists(images_dir) or not os.path.exists(win_icons_dir) or not os.path.exists(linux_icons_dir):
    print("Couldn't find a required directory!")
    print("Images => " + images_dir)
    print("Windows => " + win_icons_dir)
    print("Linux => " + linux_icons_dir)
    quit()

# Download and save logo if we don't have it already
if not os.path.exists(logo_filename):
    print("Downloading logo...")
    url = 'https://raw.githubusercontent.com/EdgeTX/edgetx.github.io/master/images/edgetx-v2.png'
    try:
        r = requests.get(url, allow_redirects=True)
    except requests.exceptions.RequestException as e:  # This is the correct syntax
        print("Unable to download!")
        raise SystemExit(e)

    open(logo_filename, 'wb').write(r.content)

print("Generate 96x96 icon.png... ")
os.chdir(images_dir)
if os.path.exists(images_dir + os.sep + 'icon.png'):
    os.remove(images_dir + os.sep + 'icon.png')
img = Image.open(start_dir + os.sep + logo_filename).resize((96,96))
img.save(images_dir + os.sep + 'icon.png')

print("Generate Linux Icons... ", end="")
os.chdir(linux_icons_dir)
linux_resolutions = [16, 22, 24, 32, 48, 128, 256, 512]
for size in linux_resolutions:
    new_image_folder = os.path.join(linux_icons_dir, str(size) + 'x' + str(size))
    if os.path.exists(new_image_folder + os.sep + 'companion.png'):
        os.remove(new_image_folder + os.sep + 'companion.png')
    elif not os.path.exists(new_image_folder):
        os.mkdir(new_image_folder)

    print(str(size) + " ", end="")
    img = Image.open(start_dir + os.sep + logo_filename).resize((size, size))
    img.save(new_image_folder + os.sep + 'companion.png')


print("\nGenerate Windows Icons... ", end="")
os.chdir(win_icons_dir)
windows_resolutions = [16, 20, 24, 30, 32, 36, 40, 48, 60, 64, 72, 80, 96, 128, 256]
for size in windows_resolutions:
    if os.path.exists(win_icons_dir + os.sep + 'edgetx_' + str(size) + '.ico'):
        os.remove(win_icons_dir + os.sep + 'edgetx_' + str(size) + '.ico')
    img = Image.open(start_dir + os.sep + logo_filename).resize((size, size))
    print(str(size) + " ", end="")
    img.save('edgetx_' + str(size) + '.ico')

print("\nWindows All in One...")
# Since Pillow only supports [(16, 16), (24, 24), (32, 32), (48, 48), (64, 64), (128, 128), (256, 256)]
# setting any other size will use the nearest valid setting instead, don't bother with the full list
# icon_sizes = [(16,16), (20,20), (24,24), (30,30), (32, 32), (36,36), (40,40), (48, 48), (60,60), (64,64), (72,72), (80,80), (96,96), (128,128), (256,256)]
icon_sizes = [(16, 16), (24, 24), (32, 32), (48, 48),
              (64, 64), (128, 128), (256, 256)]

img = Image.open(start_dir + os.sep + logo_filename)
if os.path.exists('edgetx.ico'):
    os.remove('edgetx.ico')
img.save('edgetx.ico', sizes=icon_sizes)

os.chdir(start_dir)

# Cleanup
os.remove(logo_filename)
