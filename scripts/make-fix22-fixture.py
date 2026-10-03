#!/usr/bin/env python3
"""Create a precise diagnostic atlas; retain the original V14 test PNG separately.

No downloaded fonts or image editing. The 5x7 glyphs and panel dimensions are
defined here so the test label size and colors are reproducible.
"""
from pathlib import Path
from PIL import Image, ImageDraw

glyphs = {
    'A': ['01110','10001','10001','11111','10001','10001','10001'],
    'B': ['11110','10001','10001','11110','10001','10001','11110'],
    'D': ['11110','10001','10001','10001','10001','10001','11110'],
    'E': ['11111','10000','10000','11110','10000','10000','11111'],
    'F': ['11111','10000','10000','11110','10000','10000','10000'],
    'L': ['10000','10000','10000','10000','10000','10000','11111'],
    'M': ['10001','11011','10101','10101','10001','10001','10001'],
    'N': ['10001','11001','10101','10011','10001','10001','10001'],
    'O': ['01110','10001','10001','10001','10001','10001','01110'],
    'R': ['11110','10001','10001','11110','10100','10010','10001'],
    'S': ['01111','10000','10000','01110','00001','00001','11110'],
    'T': ['11111','00100','00100','00100','00100','00100','00100'],
    'V': ['10001','10001','10001','10001','10001','01010','00100'],
}

def label(word, scale):
    mask = Image.new('L', ((6*len(word)-1)*scale, 7*scale), 0)
    draw = ImageDraw.Draw(mask)
    for n, char in enumerate(word):
        for y, row in enumerate(glyphs[char]):
            for x, bit in enumerate(row):
                if bit == '1':
                    x0, y0 = (6*n+x)*scale, y*scale
                    draw.rectangle((x0,y0,x0+scale-1,y0+scale-1),fill=255)
    return mask

def place(image, word, scale, center, rotate=False):
    mask = label(word, scale)
    if rotate:
        mask = mask.transpose(Image.Transpose.ROTATE_90)
    x, y = center[0]-mask.width//2, center[1]-mask.height//2
    # A black box behind white glyphs is visible regardless of panel lighting.
    ImageDraw.Draw(image).rectangle((x-8,y-8,x+mask.width+7,y+mask.height+7),fill=(8,12,16,255))
    image.paste((255,255,255,255),(x,y),mask)

def fixture():
    image = Image.new('RGBA',(1100,588))
    draw = ImageDraw.Draw(image)
    for box,color in [((0,0,519,587),(39,93,170,255)),
                      ((520,0,579,587),(244,165,36,255)),
                      ((580,0,1099,587),(20,148,90,255))]:
        draw.rectangle(box,fill=color)
        draw.rectangle((box[0]+5,5,box[2]-5,582),outline=(255,255,255,255),width=4)
    place(image,'VERSO',12,(260,294))
    place(image,'LOMBADA',5,(550,294),True)
    place(image,'FRENTE',12,(840,294))
    return image

if __name__ == '__main__':
    path = Path(__file__).resolve().parent.parent/'pkgfiles/USRDIR/FULL_COVER_FIX22_LABELS.png'
    fixture().save(path,format='PNG',optimize=False,compress_level=9)
    print(f'{path.name}: 1100x588, BACK 0:520 | SPINE 520:580 | FRONT 580:1100')
