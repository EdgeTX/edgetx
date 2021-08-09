#!/usr/bin/env python3

# certifi==2021.5.30
# charset-normalizer==2.0.4
# idna==3.2
# Pillow==8.3.1
# requests==2.26.0
# urllib3==1.26.6

import requests
import os
import shutil
import subprocess
import platform
import tarfile
from PIL import Image

# if False, then sips will be used instead of ImageMagick
useMagick = True

logo_filename = "edgetx-logo.png"

PROJECT_ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), ".."))
images_dir = os.path.join(PROJECT_ROOT, "companion", "src", "images")
win_icons_dir = os.path.join(PROJECT_ROOT, "companion", "src", "images", "winicons")
linux_icons_dir = os.path.join(PROJECT_ROOT, "companion", "src", "images", "linuxicons")
mac_icons_dir = os.path.join(PROJECT_ROOT, "companion", "src", "images", "macicons")
start_dir = os.getcwd()


def cleanup():
    os.chdir(start_dir)
    os.remove(logo_filename)

    # Remove temporary mac icons folder
    if os.path.exists(mac_icons_dir):
        shutil.rmtree(mac_icons_dir)

    if os.path.exists("icns.tar.gz"):
        os.remove("icns.tar.gz")

    if os.path.exists("icnsify"):
        os.remove("icnsify")
    quit()


def downloadFile(url: str, outFile: str):
    try:
        r = requests.get(url, allow_redirects=True)
    except requests.exceptions.RequestException as e:  # This is the correct syntax
        print("Unable to download!")
        raise SystemExit(e)
    open(outFile, 'wb').write(r.content)


# Startup checks
if not os.path.exists(images_dir) or not os.path.exists(win_icons_dir) or not os.path.exists(linux_icons_dir):
    print("Couldn't find a required directory!")
    print("Images => " + images_dir)
    print("Windows => " + win_icons_dir)
    print("Linux => " + linux_icons_dir)
    quit()

# Download and save logo if we don't have it already
if not os.path.exists(logo_filename):
    print("Downloading logo...")
    downloadFile('https://raw.githubusercontent.com/EdgeTX/edgetx.github.io/master/images/edgetx-v2.png', logo_filename)

print("Generate 96x96 icon.png... ")
os.chdir(images_dir)
if os.path.exists(images_dir + os.sep + 'icon.png'):
    os.remove(images_dir + os.sep + 'icon.png')
img = Image.open(start_dir + os.sep + logo_filename).resize((96, 96))
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

alternative_icns_tools = '''\
icns (Go) @ https://github.com/JackMordaunt/icns
make-icns (NodeJS) @ https://www.npmjs.com/package/make-icns
libicns @ https://icns.sourceforge.io\
'''

print("Generate Mac icon set...")
if platform.system() == "Linux":
    icns_linux_gz = os.path.join(start_dir, "icns.tar.gz")
    print("Downloading icns (Go) for Linux amd64...")
    downloadFile('https://github.com/JackMordaunt/icns/releases/download/v2.1.2/icns_2.1.2_Linux_x86_64.tar.gz',
                 icns_linux_gz)

    print("Extract icnsify...")
    tar = tarfile.open(icns_linux_gz, "r:gz")
    for member in tar.getmembers():
        if "icnsify" in member.name:
            tar.extract(member, start_dir)
    tar.close()

    if not os.path.exists(os.path.join(start_dir, "icnsify")):
        print("Something went wrong setting up icns! Some alternatives are")
        print(alternative_icns_tools)
    else:
        print("Creating MacOS icon set...")
        subprocess.call(
            [
                os.path.join(start_dir, "icnsify"),
                "-i",
                os.path.join(start_dir, logo_filename),
                "-o",
                os.path.join(images_dir, "iconmac.icns")
            ]
        )

    cleanup()
elif platform.system == "Windows":
    print("Windows is not supported yet. Some options are:")
    print(alternative_icns_tools)
    cleanup()
elif platform.system == "Darwin":
    if useMagick:
        if shutil.which("magick") is None or shutil.which("iconutil") is None:
            print("Required tool ImageMagick or Mac built in iconutil missing")
            print("useMagick flag at top of file can be disabled to use sips instead,")
            print("but it apparently has poorer image quality than ImageMagick.")
            print()
            print("Alternatives for icon generation are:")
            print(alternative_icns_tools)
            cleanup()

    # Following was based on MIT licensed work from retifrav
    # https://github.com/retifrav/python-scripts/blob/master/generate-iconset/generate-iconset.py

    class IconParameters():
        width = 0
        scale = 1

        def __init__(self, width, scale):
            self.width = width
            self.scale = scale

        def getIconName(self):
            if self.scale != 1:
                return f"icon_{self.width}x{self.width}{ext}"
            else:
                return f"icon_{self.width//2}x{self.width//2}@2x{ext}"

    ListOfIconParameters = [
        IconParameters(16, 1),
        IconParameters(16, 2),
        IconParameters(32, 1),
        IconParameters(32, 2),
        IconParameters(64, 1),
        IconParameters(64, 2),
        IconParameters(128, 1),
        IconParameters(128, 2),
        IconParameters(256, 1),
        IconParameters(256, 2),
        IconParameters(512, 1),
        IconParameters(512, 2),
        IconParameters(1024, 1),
        IconParameters(1024, 2)
    ]

    # generate iconset
    for ip in ListOfIconParameters:
        if useMagick:
            subprocess.call(
                [
                    "magick",
                    "convert",
                    logo_filename,
                    "-resize",
                    str(ip.width),
                    mac_icons_dir / ip.getIconName()
                ]
            )
        else:
            subprocess.call(
                [
                    "sips",
                    "-z",
                    str(ip.width),
                    str(ip.width),
                    logo_filename,
                    "--out",
                    mac_icons_dir / ip.getIconName()
                ]
            )

    # convert iconset to icns file
    subprocess.call(
        [
            "iconutil",
            "-c",
            "icns",
            mac_icons_dir,
            "-o",
            images_dir / "iconmac.icns"
        ]
    )

    cleanup()
