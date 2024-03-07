import matplotlib.pyplot as plt
import os
import sys

RESULTS_FILE = sys.argv[1]
OUTPUT_DIR = os.path.dirname(RESULTS_FILE)
GRAPHICS_DIR = os.path.abspath(os.path.join(OUTPUT_DIR, 'graphics'))
os.makedirs(GRAPHICS_DIR, exist_ok=True)

file = open(RESULTS_FILE, 'r')
lines = file.readlines()

col_names = lines[0].split(); lines.pop(0)
FILENAMES = [line.split()[0].split('.')[0] for line in lines]
ENTROPY_RATIOS = [float(line.split()[1]) for line in lines]
START_SIZES = [float(line.split()[2]) for line in lines]
ENCODED_SIZES = [float(line.split()[3]) for line in lines]
COMPRESSION_RATIOS = [float(line.split()[4]) for line in lines]
DECODING_RATIOS = [float(line.split()[5]) for line in lines]

# entropy ratios
plt.figure(figsize=(10, 5))
plt.bar(FILENAMES, ENTROPY_RATIOS, color='red')
plt.title('Entropy ratios')
plt.savefig(os.path.join(GRAPHICS_DIR, 'entropy_ratios.png'))
plt.show()

# start and encoded file sizes
fig, axes = plt.subplots(2, 1, figsize=(10, 10))
axes[0].bar(FILENAMES, START_SIZES, color=['red']*len(lines))
axes[0].set_title('START file sizes')
axes[0].set_ylabel('Kilobytes')
axes[1].bar(FILENAMES, ENCODED_SIZES, color=['#75E859']*len(lines))
axes[1].set_title('ENCODED file sizes')
axes[1].set_ylabel('Kilobytes')
plt.tight_layout()
plt.savefig(os.path.join(GRAPHICS_DIR, 'start_and_encoded_sizes.png'))
plt.show()
plt.close(fig)

# start vs encoded file sizes
plt.figure(figsize=(10, 5))
plt.bar(FILENAMES, START_SIZES, color='red', label='Start size')
plt.bar(FILENAMES, ENCODED_SIZES, color='#75E859', label='Encoded size')
plt.title('START vs ENCODED file sizes')
plt.ylabel('Kilobytes')
plt.legend()
plt.savefig(os.path.join(GRAPHICS_DIR, 'start_vs_encoded_sizes.png'))
plt.show()

