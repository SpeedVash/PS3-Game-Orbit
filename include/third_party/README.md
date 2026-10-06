# stb_image_write

v1.16, upstream commit `2c980bb59875b0d32144a71867fbdebb2f77cd20`. Source: https://github.com/nothings/stb/blob/2c980bb59875b0d32144a71867fbdebb2f77cd20/stb_image_write.h

Local fix: the JPEG bit accumulator and its internal parameters use `unsigned int`; Huffman values are cast to unsigned before shifting. This defines 32-bit wraparound and removes the signed-left-shift undefined behavior found by UBSan. The encoded JPEG format and public API are unchanged.

License: `assets/licenses/STB_LICENSE.txt` (MIT or public domain).
