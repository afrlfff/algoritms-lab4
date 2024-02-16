from PIL import Image
import os
import math

# convert image to the sequence of characters
def img_to_text(input_path, output_path):
    file = open(output_path, 'wb') # binary writing mode

    im = Image.open(input_path)
    im = im.convert('RGB')
    pixels = im.getdata()

    for pixel in pixels:
        file.write(chr(pixel[0]).encode('utf-8') + \
                   chr(pixel[1]).encode('utf-8') + \
                   chr(pixel[2]).encode('utf-8'))

    file.close()

# convert sequence of characters to image
""" def text_to_img(input_path, output_path, img_size, show=False):
    file = open(input_path, 'r', encoding='utf-8')
    content = file.read()

    pixels = []
    for i in range(0, len(content) - 2, 3):
        pixels.append((ord(content[i]), ord(content[i + 1]), ord(content[i + 2])))
    
    file.close()

    im = Image.new('RGB', img_size)
    im.putdata(pixels)
    im.save(output_path)
    if show:
        im.show()
 """
# calculate entropy of the text file
def entropy(input_path):
    file = open(input_path, 'rb') # binary reading mode
    content = file.read().decode('utf-8')
    file.close()

    # probabilities of each symbol
    P = {c: 0 for c in set(content)}

    # calculate probabilities
    total_count = len(content)
    for c in content:
        P[c] += 1
    for key, _ in P.items():
        P[key] /= total_count
    
    # calculate entropy
    entropy = 0
    for key, value in P.items():
        entropy += -value * math.log(value, 2)
    
    return entropy

# calculate the coefficient of equivalence of two files.
def files_equal_coefficient(path1, path2):
    file1 = open(path1, 'rb') # binary reading mode
    file2 = open(path2, 'rb') # binary reading mode
    content1 = file1.read()
    content2 = file2.read()
    file1.close()
    file2.close()

    total_count = max(len(content1), len(content2))
    k = 0
    for i in range(min(len(content1), len(content2))):
        if content1[i] == content2[i]:
            k += 1

    return k / total_count

# calculate compression ratio
def compression_ratio(path_to_original, path_to_encodoed):
    return os.path.getsize(path_to_original) / os.path.getsize(path_to_encodoed)