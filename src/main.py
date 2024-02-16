import os
import PIL
import subprocess

from help_functions import img_to_text
from help_functions import entropy
from help_functions import files_equal_coefficient
from help_functions import compression_ratio

INPUT_DIR = os.path.abspath(os.path.join(os.path.dirname(__file__), '../input'))
OUTPUT_DIR = os.path.abspath(os.path.join(os.path.dirname(__file__), '../output'))
BIN_DIR = os.path.abspath(os.path.join(os.path.dirname(__file__), '../bin'))

# =====
# read images to .txt files to input foler
for filename in os.listdir(os.path.join(INPUT_DIR, "img")):
    input_path = os.path.join(INPUT_DIR, "img\\" + filename)
    output_path = os.path.join(INPUT_DIR, f"{filename.split('.')[0]}.txt")
    img_to_text(input_path, output_path)

# =====
# make encoded .txt files using RLE encoder
for filename in os.listdir(INPUT_DIR):
    if not filename.endswith('.txt'):
        continue

    input_path = os.path.join(INPUT_DIR, filename)
    new_filename = filename.split('.')[0] + '_encoded.txt'
    output_path = os.path.join(OUTPUT_DIR, f"encoded\\{new_filename}")
    path_to_exe = os.path.join(BIN_DIR, 'RLE_encoding.exe')
    subprocess.run([path_to_exe, input_path, output_path])

# =====
# make decoded .txt files using RLE decoder
for filename in os.listdir(os.path.join(OUTPUT_DIR, "encoded")):
    input_path = os.path.join(OUTPUT_DIR, f"encoded\\{filename}")
    new_filename = '_'.join(filename.split('_')[:-1]) + '_decoded.txt'
    output_path = os.path.join(OUTPUT_DIR, f"decoded\\{new_filename}")
    path_to_exe = os.path.join(BIN_DIR, 'RLE_decoding.exe')
    subprocess.run([path_to_exe, input_path, output_path])

# =====
# make results.txt
file = open(os.path.join(OUTPUT_DIR, "results.txt"), 'w', encoding='utf-8')
file.write("filename entropy_ratio decoding_ratio compression_ratio start_size[kb] encoded_size[kb]")

for filename in os.listdir(INPUT_DIR):
    if not filename.endswith('.txt'):
        continue

    file.write('\n')
    path_to_original = os.path.join(INPUT_DIR, filename)
    path_to_encoded = os.path.join(OUTPUT_DIR, f"encoded\\{filename.split('.')[0]}_encoded.txt")
    path_to_decoded = os.path.join(OUTPUT_DIR, f"decoded\\{filename.split('.')[0]}_decoded.txt")

    file.write(filename + ' ' + \
               str(entropy(path_to_original)) + ' ' + \
               str(files_equal_coefficient(path_to_original, path_to_decoded)) + ' ' + \
               str(compression_ratio(path_to_original, path_to_encoded)) + ' ' +\
               str(os.path.getsize(path_to_original) / 1024) + ' ' + \
               str(os.path.getsize(path_to_encoded) / 1024))
file.close()

# =====
# make graphics
