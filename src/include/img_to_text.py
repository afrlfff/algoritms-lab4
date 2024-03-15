from PIL import Image
import sys

# convert image to the sequence of characters
def img_to_text(input_path, output_path):
    im = Image.open(input_path)
    pixels = im.load()
    width, height = im.size
    with open(output_path, 'w', encoding='utf-8') as file:
        for y in range(height):
            for x in range(width):
                if type(pixels[x, y]) == tuple:
                    r, g, b = pixels[x, y]
                else:
                    r, g, b = pixels[x, y], pixels[x, y], pixels[x, y]

                # add 256 to avoid problematical characters like '\0' etc.
                file.write(chr(r + 256) + chr(g + 256) + chr(b + 256))

arguments = sys.argv
img_to_text(arguments[1], arguments[2])