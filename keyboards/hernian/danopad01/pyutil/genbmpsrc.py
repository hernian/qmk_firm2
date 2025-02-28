# bmpファイルを読み込んで、モノクロ画像のc言語配列のソースファイルを出力する
import sys
import os
from PIL import Image


def main() -> None:
    filename_in = sys.argv[1]
    dir_out = sys.argv[2]
    print(f'input: {filename_in}\n')

    if dir_out != '' and dir_out != '.':
        os.makedirs(dir_out, exist_ok=True)
    basename = os.path.splitext(os.path.basename(filename_in))[0]
    filename_out_c = os.path.join(dir_out, f'img_{basename}.c')
    filename_out_h = os.path.join(dir_out, f'img_{basename}.h')

    # filenameSrc = '2in9_1.bmp'
    im = Image.open(filename_in)
    h, w = im.size
    count_byte_line = (h + 7) // 8
    count_byte_image = w * count_byte_line
    with open(filename_out_c, 'w') as fs:
        fs.write(f'#include "img_{basename}.h"\n')
        fs.write(f'const uint8_t img_{basename}[{count_byte_image}] =\n');
        fs.write('{\n');
        for y in range(h):
            src_line = '  '
            pix8 = 0
            for x in range(w):
                pix = sum(im.getpixel((y, w - x - 1)))
                #print(pix)
                pix8 = pix8 << 1
                if pix != 0:
                    pix8 |= 0x01
                if x % 8 == 7:
                    if y < 296:
                        print(f'{pix8:02x} ', end='')
                    comma = ',' if x < 127 or y < 295 else ''
                    src_line += f'0x{pix8:02x}{comma} '
                    pix8 = 0
            if y < 296:
                print('')
            fs.write(f'{src_line}\n')
        fs.write('};\n')
    with open(filename_out_h, 'w') as fs:
        fs.write('#pragma once\n')
        fs.write('#include <stdint.h>\n')
        fs.write(f'const uint8_t img_{basename}[{count_byte_image}];\n')

if __name__ == '__main__':
    main()
