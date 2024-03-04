from PIL import Image
import sys

# convert image to the sequence of characters
def img_to_text(input_path, output_path):
    with open(output_path, 'wb') as file:
        im = Image.open(input_path)
        im = im.convert('RGB')
        pixels = im.getdata()

        for pixel in pixels:
            file.write(chr(pixel[0]).encode('utf-8') + \
                    chr(pixel[1]).encode('utf-8') + \
                    chr(pixel[2]).encode('utf-8'))

arguments = sys.argv
img_to_text(arguments[1], arguments[2])