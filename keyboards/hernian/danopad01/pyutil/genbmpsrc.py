# bmpファイルを読み込んで、モノクロ画像のc言語配列のソースファイルを出力する

from PIL import Image

def main() -> None:
    # filenameSrc = '2in9_1.bmp'
    filenameSrc = 'cross.bmp'
    filenameDst = 'mybitmap.c'
    im = Image.open(filenameSrc)
    w, h = im.size
    with open(filenameDst, 'w') as fs:
        fs.write('static const uint8_t Image128x296[] =\n');
        fs.write('{\n');
        for y in range(h):
            src_line = '  '
            pix8 = 0
            for x in range(w):
                pix = im.getpixel((x, y))
                pix8 = pix8 << 1
                if pix != 0:
                    pix8 |= 0x01
                if x % 8 == 7:
                    comma = ',' if x < 127 or y < 295 else ''
                    src_line += f'0x{pix8:02x}{comma} '
                    pix8 = 0
            fs.write(f'{src_line}\n')
        fs.write('};\n')

if __name__ == '__main__':
    main()
