import sys
import os
import time
import argparse
import matplotlib.pyplot as plt
import matplotlib.patches as patches
from matplotlib.collections import PatchCollection

# Constants for colors
FCELL_COLOR = "#FF0000"
MCELL_COLOR = "#00FF00"
PR_COLOR    = "#0000FF"


def parse_arguments():
    """Parse command-line arguments."""
    parser = argparse.ArgumentParser (
        description = "A script that can draw the die layout.",
        usage="%(prog)s -i [Input_Name] -m [Output_Name] / [Optional] -t [Title] [-p] [-f] [-m]",
    )
    parser.add_argument("-i", "--input",  required=True, type=str, help="Input file name")
    parser.add_argument("-o", "--output", required=True, type=str, help="Output file name")
    parser.add_argument("-t", "--title",  required=False, type=str, help="Title for the plot")
    parser.add_argument("-P", "--placement-rows-file", required=False, type=str, help="Show placement rows with file name")
    parser.add_argument("-p", "--hide-placement-rows", action="store_false", help="Hide placement rows")
    parser.add_argument("-f", "--hide-fcell", action="store_false", help="Hide fixed cells")
    parser.add_argument("-m", "--hide-mcell", action="store_false", help="Hide movable cells")

    return parser.parse_args()

def setup_plot(die_lb_x, die_lb_y, die_ur_x, die_ur_y):
    """Set up the matplotlib figure and axis."""
    die_width  = die_ur_x - die_lb_x
    die_height = die_ur_y - die_lb_y
    size = 2.3e-5 * max(die_width, die_height) + 10
    print(f"figsize=({size:.2f}, {size:.2f})")

    fig, ax = plt.subplots(figsize=(size, size))
    ax.set_xlim(die_lb_x, die_ur_x)
    ax.set_ylim(die_lb_y, die_ur_y)
    ax.set_aspect('equal')  # Equal aspect ratio
    ax.set_facecolor('#EEEEEE')
    return fig, ax

def parse_input_lg(file_name):
    """Parse the input file to extract block information."""
    if not os.path.isfile(file_name):
        raise FileNotFoundError(f"Input file {file_name} does not exist.")

    with open(file_name, 'r') as f:
        lines = f.read().strip().split("\n")

    # Parse die dimensions
    die_dimensions = lines[2].split(" ")
    die_lb_x, die_lb_y, die_ur_x, die_ur_y = map(float, die_dimensions[1:5])

    # Get all cell lines until 'PlacementRows'
    cell_lines = []
    placement_rows = []
    temp_placement_rows = []

    for line in lines[3:]:
        if line.split()[0] == "PlacementRows":
            temp_placement_rows.append(line.split()[1:])
        else:
            cell_lines.append(line)

    # Parse cells using list comprehension
    cells = [(words[0], *map(float, words[1:5]), words[5])
             for line in cell_lines
             for words in [line.split()]]

    # Separate fixed and movable cells
    fCell = [(name, x, y, w, h) for name, x, y, w, h, fix in cells if fix == "FIX"]
    mCell = [(name, x, y, w, h) for name, x, y, w, h, fix in cells if fix != "FIX"]

    # Placement rows format: convert(x, y, site_width, site_height, num_of_sites) to (name, x, y, site_width*num_of_sites, site_height)
    for i, words in enumerate(temp_placement_rows):
        name, x, y, w, h, num_of_sites = f"PR_{i}", *map(float, words)
        placement_rows.append((name, x, y, w*num_of_sites, h))


    # placement_rows = [tuple(map(int, words[:5])) for words in placement_rows]

    return die_lb_x, die_lb_y, die_ur_x, die_ur_y, fCell, mCell, placement_rows

def parse_placement_row_file(file_name):
    """Parse the placement rows file to extract placement rows information."""
    with open(file_name, 'r') as f:
        lines = f.read().strip().split("\n")

    placement_rows = []
    for i, line in enumerate(lines):
        words = line.split()
        x, y, w, h = map(int, words)
        placement_rows.append((f"PR_{i}", x, y, w, h))

    return placement_rows

def draw_blocks(ax, blocks, **kwargs):
    """Draw multiple blocks using PatchCollection."""

    color = kwargs.get('color', '#000000')
    alpha = kwargs.get('alpha', 1.0)
    linewidth = kwargs.get('linewidth', 1.0)

    # Create patches for each block
    rects = []
    for cellName, x, y, w, h in blocks:
        rect = patches.Rectangle(
            (x, y), w, h,
            edgecolor=color,
            facecolor="#FFFFFF",
            linewidth=linewidth,
            alpha=alpha
        )
        rects.append(rect)

    # Create a PatchCollection for blocks and add it to the axes
    block_collection = PatchCollection(rects, match_original=True)
    ax.add_collection(block_collection)

    # Check for overlaps
    # Sweep line algorithm for overlap detection
    events = []
    for i, (name, x, y, w, h) in enumerate(blocks):
        events.append((x, 'start', i, y, y + h))
        events.append((x + w, 'end', i, y, y + h))

    events.sort()  # Sort by x-coordinate

    active = []  # Blocks intersecting with sweep line
    for x, event_type, block_idx, y1, y2 in events:
        if event_type == 'start':
            # Check for overlaps with all active blocks
            for other_idx, other_y1, other_y2 in active:
                # Check y-overlap
                if not (y2 <= other_y1 or y1 >= other_y2):
                    block1 = blocks[block_idx]
                    block2 = blocks[other_idx]

                    # Calculate overlap rectangle
                    x_left = x
                    y_bottom = max(y1, other_y1)
                    x_right = min(block1[1] + block1[3], block2[1] + block2[3])
                    y_top = min(y2, other_y2)

                    overlap_rect = patches.Rectangle(
                        (x_left, y_bottom),
                        x_right - x_left,
                        y_top - y_bottom,
                        facecolor='red',
                        alpha=0.5
                    )
                    ax.add_patch(overlap_rect)

            active.append((block_idx, y1, y2))
        else:
            # Remove block from active set
            active = [(idx, y1, y2) for idx, y1, y2 in active if idx != block_idx]

def main():
    start_time = time.time()

    args = parse_arguments()

    input_file          = args.input
    image_file          = args.output
    image_title         = args.title
    placement_rows_file = args.placement_rows_file

    # Parse the input file
    print("Parsing input file...")
    die_lb_x, die_lb_y, die_ur_x, die_ur_y, fCell, mCell, placement_rows = parse_input_lg(input_file)

    # Set up the figure and axis with adjusted dimensions
    fig, ax = setup_plot(die_lb_x, die_lb_y, die_ur_x, die_ur_y)

    # Draw blocks
    print("Drawing blocks...")
    if args.hide_mcell:
        draw_blocks(ax, mCell, color=MCELL_COLOR)
    if args.hide_fcell:
        draw_blocks(ax, fCell, color=FCELL_COLOR)
    if placement_rows_file:
        placement_rows = parse_placement_row_file(placement_rows_file)
        draw_blocks(ax, placement_rows, color=PR_COLOR, alpha=0.5, linewidth=0.3)
    elif args.hide_placement_rows:
        draw_blocks(ax, placement_rows, color=PR_COLOR, alpha=0.5, linewidth=0.3)



    # Save the figure with the specified output name
    plt.title(image_title)
    plt.savefig(image_file, dpi=300)
    plt.close()  # Close the plot to free up memory
    print(f"Plot saved as {image_file}")

    end_time = time.time()
    print(f"Total execution time: {end_time - start_time:.2f} seconds")

if __name__ == "__main__":
    main()
