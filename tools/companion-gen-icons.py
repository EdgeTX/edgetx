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
import pathlib
from PIL import Image

# if False, then sips will be used instead of ImageMagick
useMagick = True

LOGO_FILENAME = "edgetx-logo.png"

PROJECT_ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), ".."))
TOOLS_DIR = os.path.dirname(__file__)
IMAGES_DIR = os.path.join(PROJECT_ROOT, "companion", "src", "images")
WIN_ICONS_DIR = os.path.join(PROJECT_ROOT, "companion", "src", "images", "winicons")
LINUX_ICONS_DIR = os.path.join(PROJECT_ROOT, "companion", "src", "images", "linuxicons")
MAC_ICONS_DIR = os.path.join(PROJECT_ROOT, "companion", "src", "images", "macicons")
START_DIR = os.getcwd()
ABS_LOGO_FILENAME = os.path.abspath(os.path.join(TOOLS_DIR,LOGO_FILENAME))

def cleanup():
    os.chdir(TOOLS_DIR)
    os.remove(ABS_LOGO_FILENAME)

    # Remove temporary mac icons folder
    if os.path.exists(MAC_ICONS_DIR):
        shutil.rmtree(MAC_ICONS_DIR)

    if os.path.exists("icns.tar.gz"):
        os.remove("icns.tar.gz")

    if os.path.exists("icnsify"):
        os.remove("icnsify")

    os.chdir(START_DIR)
    quit()


def downloadFile(url: str, outFile: str):
    try:
        r = requests.get(url, allow_redirects=True)
    except requests.exceptions.RequestException as e:  # This is the correct syntax
        print("Unable to download!")
        raise SystemExit(e)
    open(outFile, 'wb').write(r.content)


# Startup checks
if not os.path.exists(IMAGES_DIR) or not os.path.exists(WIN_ICONS_DIR) or not os.path.exists(LINUX_ICONS_DIR):
    print("Couldn't find a required directory!")
    print("Images => " + IMAGES_DIR)
    print("Windows => " + WIN_ICONS_DIR)
    print("Linux => " + LINUX_ICONS_DIR)
    quit()

# Download and save logo if we don't have it already
if not os.path.exists(LOGO_FILENAME):
    print("Downloading logo...")
    downloadFile('https://edgetx.org/assets/logo.png', LOGO_FILENAME)

print("Generate 96x96 icon.png... ")
os.chdir(IMAGES_DIR)
icon_png = os.path.join(IMAGES_DIR,'icon.png')
if os.path.exists(icon_png):
    os.remove(icon_png)
img = Image.open(ABS_LOGO_FILENAME).resize((96, 96))
img.save(icon_png)

print("Generate Linux Icons... ", end="")
os.chdir(LINUX_ICONS_DIR)

linux_resolutions = [16, 22, 24, 32, 48, 128, 256, 512]
for size in linux_resolutions:
    new_image_folder = os.path.join(LINUX_ICONS_DIR, str(size) + 'x' + str(size))
    companion_png = os.path.join(new_image_folder, 'companion.png')
    if os.path.exists(companion_png):
        os.remove(companion_png)
    elif not os.path.exists(new_image_folder):
        os.mkdir(new_image_folder)

    print(str(size) + " ", end="")
    img = Image.open(ABS_LOGO_FILENAME).resize((size, size))
    img.save(companion_png)

print("\nGenerate Windows Icons... ", end="")
os.chdir(WIN_ICONS_DIR)
windows_resolutions = [16, 20, 24, 30, 32, 36, 40, 48, 60, 64, 72, 80, 96, 128, 256]
for size in windows_resolutions:
    if os.path.exists('edgetx_' + str(size) + '.ico'):
        os.remove('edgetx_' + str(size) + '.ico')
    img = Image.open(ABS_LOGO_FILENAME).resize((size, size))
    print(str(size) + " ", end="")
    img.save('edgetx_' + str(size) + '.ico')

print("\nWindows All in One...")
# Since Pillow only supports [(16, 16), (24, 24), (32, 32), (48, 48), (64, 64), (128, 128), (256, 256)]
# setting any other size will use the nearest valid setting instead, don't bother with the full list
# icon_sizes = [(16,16), (20,20), (24,24), (30,30), (32, 32), (36,36), (40,40), (48, 48), (60,60), (64,64), (72,72), (80,80), (96,96), (128,128), (256,256)]
icon_sizes = [(16, 16), (24, 24), (32, 32), (48, 48),
              (64, 64), (128, 128), (256, 256)]

img = Image.open(ABS_LOGO_FILENAME)
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
    icns_linux_gz = os.path.join(TOOLS_DIR, "icns.tar.gz")
    print("Downloading icns (Go) for Linux amd64...")
    downloadFile('https://github.com/JackMordaunt/icns/releases/download/v2.1.2/icns_2.1.2_Linux_x86_64.tar.gz',
                 icns_linux_gz)

    print("Extract icnsify...")
    tar = tarfile.open(icns_linux_gz, "r:gz")
    for member in tar.getmembers():
        if "icnsify" in member.name:
            tar.extract(member, TOOLS_DIR)
    tar.close()

    if not os.path.exists(os.path.join(TOOLS_DIR, "icnsify")):
        print("Something went wrong setting up icns! Some alternatives are")
        print(alternative_icns_tools)
    else:
        print("Creating MacOS icon set...")
        subprocess.call(
            [
                os.path.join(TOOLS_DIR, "icnsify"),
                "-i",
                ABS_LOGO_FILENAME,
                "-o",
                os.path.join(IMAGES_DIR, "iconmac.icns")
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

    ext = pathlib.Path(ABS_LOGO_FILENAME).suffix

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
                    ABS_LOGO_FILENAME,
                    "-resize",
                    str(ip.width),
                    MAC_ICONS_DIR / ip.getIconName()
                ]
            )
        else:
            subprocess.call(
                [
                    "sips",
                    "-z",
                    str(ip.width),
                    str(ip.width),
                    ABS_LOGO_FILENAME,
                    "--out",
                    MAC_ICONS_DIR / ip.getIconName()
                ]
            )

    # convert iconset to icns file
    subprocess.call(
        [
            "iconutil",
            "-c",
            "icns",
            MAC_ICONS_DIR,
            "-o",
            IMAGES_DIR / "iconmac.icns"
        ]
    )

    cleanup()
