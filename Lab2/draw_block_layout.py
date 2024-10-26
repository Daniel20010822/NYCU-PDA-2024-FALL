import sys
import numpy as np
import matplotlib.pyplot as plt
import matplotlib.patches as patches
import math
import time

def draw_block(ax, x, y, width, height, block_id):
    ax.add_patch(
        patches.Rectangle(
            (x, y),
            width,
            height,
            fill=True,
            edgecolor="#000000",
            facecolor="#FFCCCC",
            alpha=1.0
        )
    )
    # Add the block_id text at the center of the block
    # ax.text(
    #     x + width / 2, 
    #     y + height / 2, 
    #     str(block_id),
    #     color='black', 
    #     fontsize=10, 
    #     ha='center', 
    #     va='center'
    # )

def draw_terminal(ax, x, y, terminal_id):
    # Draw the terminal as a small point
    ax.plot(x, y, marker='o', color='red', markersize=5)
    # Label the terminal
    ax.text(
        x + 5, 
        y + 5, 
        terminal_id,
        color='black', 
        fontsize=8, 
        ha='left', 
        va='bottom'
    )

# Input and output file names
txt_name = sys.argv[1]
png_name = sys.argv[2]

# Read and parse the input file
with open(txt_name, 'r') as fread:
    f = fread.read().split("\n")

original_width, original_height = map(int, f[0].split(" "))
total_block_number = int(f[1])

# Initialize variables to find the adjusted outline dimensions
max_x, max_y = original_width, original_height

# Parse the blocks and find the maximum dimensions
blocks = []
line_idx = 2
for i in range(line_idx, line_idx + total_block_number):
    ss = f[i].split(" ")
    block_id = ss[0]  # Keep block_id as a string
    lb_x = int(ss[1])
    lb_y = int(ss[2])
    width = int(ss[3])
    height = int(ss[4])
    
    blocks.append((block_id, lb_x, lb_y, width, height))
    max_x = max(max_x, lb_x + width)
    max_y = max(max_y, lb_y + height)

line_idx += total_block_number

# Parse the terminal information
total_terminal_number = int(f[line_idx])
terminals = []
line_idx += 1
for i in range(line_idx, line_idx + total_terminal_number):
    ss = f[i].split(" ")
    terminal_id = ss[0]  # Terminal name
    x = int(ss[1])  # Terminal x-coordinate
    y = int(ss[2])  # Terminal y-coordinate
    terminals.append((terminal_id, x, y))
    max_x = max(max_x, x)
    max_y = max(max_y, y)

# Set up the figure and axis with adjusted dimensions
fig, ax = plt.subplots(figsize=(8, 8))
# ax.set_xlim(0, max_x)
# ax.set_ylim(0, max_y)
ax.set_xlim(0, 12000)
ax.set_ylim(0, 12000)

# Ensure the x and y axes have the same intervals (equal aspect ratio)
ax.set_aspect('equal')

# Draw the original outline with a dashed line
ax.add_patch(
    patches.Rectangle(
        (0, 0),
        original_width,
        original_height,
        fill=False,
        edgecolor="blue",
        linestyle="--",
        linewidth=1.5
    )
)

# Draw each block
for block_id, lb_x, lb_y, width, height in blocks:
    draw_block(ax, lb_x, lb_y, width, height, block_id)

# Draw each terminal
for terminal_id, x, y in terminals:
    draw_terminal(ax, x, y, terminal_id)

# Save the figure with the specified output name
plt.savefig(png_name)
