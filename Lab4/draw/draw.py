import sys
import os
import argparse
import matplotlib.pyplot as plt
import matplotlib.patches as patches
import random

class RoutingArea:
    def __init__(self, lbx, lby, width, height):
        self.lbx = lbx
        self.lby = lby
        self.width = width
        self.height = height
        self.chips = []

    def store_gcell_dimension(self, x, y):
        self.gcell_dimension = (x, y)

class Chip:
    def __init__(self, lbx, lby, width, height):
        self.lbx = lbx
        self.lby = lby
        self.width = width
        self.height = height
        self.bumps = []

class Bump:
    def __init__(self, idx, lbx, lby):
        self.idx = idx
        self.lbx = lbx
        self.lby = lby

class Via:
    def __init__(self, x, y):
        self.x = x
        self.y = y

class Wire:
    def __init__(self, layer, x1, y1, x2, y2):
        self.layer = layer  # M1 or M2
        self.x1 = x1
        self.y1 = y1
        self.x2 = x2
        self.y2 = y2

class Net:
    def __init__(self, name):
        self.name = name
        self.wires = []
        self.vias = []
        self.color = '#{:06x}'.format(random.randint(0, 0xFFFFFF))
        # List of distinct colors for nets
        DISTINCT_COLORS = [
            'red', 'blue', 'green', 'orange', 'purple', 'yellow', 'cyan', 'magenta', 'brown', 'pink',
            'navy', 'teal', 'maroon', 'olive', 'lime', 'coral', 'indigo', 'violet', 'crimson', 'gold',
            'salmon', 'turquoise', 'plum', 'khaki', 'orchid', 'steelblue', 'tomato', 'sienna', 'peru', 'chocolate',
            'tan', 'forestgreen', 'seagreen', 'royalblue', 'mediumvioletred', 'darkslateblue'
        ]
        # Assign a color from the list, cycling if more nets than colors
        self.color = DISTINCT_COLORS[hash(self.name) % len(DISTINCT_COLORS)]

def parse_gmp_file(file_path):
    with open(file_path, 'r') as file:
        lines = file.readlines()

    routing_area = None
    current_chip = None
    i = 0

    while i < len(lines):
        line = lines[i].strip()
        if not line:
            i += 1
            continue

        if line.startswith('.ra'):
            # Routing area information
            i += 1
            ra_values = list(map(int, lines[i].strip().split()))
            routing_area = RoutingArea(*ra_values)
        elif line.startswith('.g'):
            # Grid cell dimension information
            i += 1
            gcell_dimension = list(map(int, lines[i].strip().split()))
            routing_area.store_gcell_dimension(*gcell_dimension)
        elif line.startswith('.c'):
            # Chip information and position
            if current_chip:
                routing_area.chips.append(current_chip)
            i += 1
            chip_values = list(map(int, lines[i].strip().split()))
            # Adjust chip coordinates by adding routing area's lower-bound coordinates
            chip_values[0] += routing_area.lbx
            chip_values[1] += routing_area.lby
            current_chip = Chip(*chip_values)
        elif line.startswith('.b'):
            # Bump position information
            i += 1
            while i < len(lines) and lines[i].strip() and not lines[i].strip().startswith('.'):
                bump_values = list(map(int, lines[i].strip().split()))
                bump_values[1] += current_chip.lbx
                bump_values[2] += current_chip.lby
                bump = Bump(*bump_values)
                current_chip.bumps.append(bump)
                i += 1
            continue

        i += 1

    if current_chip:
        routing_area.chips.append(current_chip)

    return routing_area

def parse_lg_file(file_path):
    nets = []
    current_net = None
    last_coords = None

    with open(file_path, 'r') as file:
        for line in file:
            line = line.strip()
            if not line:
                continue

            if line == '.end':
                if current_net:
                    nets.append(current_net)
                    current_net = None
                    last_coords = None
            elif line == 'via':
                if last_coords:
                    via = Via(last_coords[0], last_coords[1])
                    current_net.vias.append(via)
            elif line.startswith('M'):
                # Parse wire: M2 x1 y1 x2 y2
                parts = line.split()
                layer = parts[0]
                coords = list(map(int, parts[1:]))
                wire = Wire(layer, *coords)
                current_net.wires.append(wire)
                last_coords = (coords[2], coords[3])  # Save last coordinates for via
            elif not line.startswith('.'):
                current_net = Net(line)

    if current_net:
        nets.append(current_net)

    return nets

def print_routing_area(routing_area):
    print(f"Routing Area: ({routing_area.lbx}, {routing_area.lby}), width: {routing_area.width}, height: {routing_area.height}")
    for i, chip in enumerate(routing_area.chips):
        print(f"Chip {i}: ({chip.lbx}, chip.lby), width: {chip.width}, height: {chip.height}")
        print("Bumps:")
        for bump in chip.bumps:
            print(f"  Bump {bump.idx}: ({bump.lbx}, {bump.lby})")

def setup_plot(routing_area):
    """Set up the matplotlib figure and axis."""
    aspect_ratio = routing_area.width / routing_area.height
    base_size = 10  # Base size for scaling
    if aspect_ratio > 1:
        figsize = (base_size * aspect_ratio, base_size)
    else:
        figsize = (base_size, base_size / aspect_ratio)
    fig, ax = plt.subplots(figsize=figsize)

    # Configure plot boundaries with padding
    ax.set_xlim(routing_area.lbx - 10, routing_area.lbx + routing_area.width + 10)
    ax.set_ylim(routing_area.lby - 10, routing_area.lby + routing_area.height + 10)

    # Create grid with gcell dimensions
    x_spacing, y_spacing = routing_area.gcell_dimension
    x_grid = range(routing_area.lbx, routing_area.lbx + routing_area.width + 1, x_spacing)
    y_grid = range(routing_area.lby, routing_area.lby + routing_area.height + 1, y_spacing)
    ax.set_xticks(x_grid)
    ax.set_yticks(y_grid)
    ax.set_xticklabels([])  # Hide x-axis tick labels
    ax.set_yticklabels([])  # Hide y-axis tick labels
    ax.grid(True, linestyle='-', alpha=0.2)

    # Set plot title
    ax.set_title('Routing Area Layout')

    # Maintain aspect ratio
    ax.set_aspect('equal')

    return fig, ax


def plot_routing_area(ax, routing_area, nets):
    """Plot the routing area layout with chips, bumps, wires, and vias."""

    # Create and add routing area boundary rectangle
    ra_rect = patches.Rectangle(
        (routing_area.lbx, routing_area.lby),
        routing_area.width,
        routing_area.height,
        fill=False,
        color='black',
        linewidth=2,
        label='Routing Area'
    )
    ax.add_patch(ra_rect)

    # Define colors for different chips
    colors = ['lightblue', 'lightgreen']

    # Add chip rectangles and their bumps
    for i, chip in enumerate(routing_area.chips):
        # Add chip rectangle
        chip_rect = patches.Rectangle(
            (chip.lbx, chip.lby),
            chip.width,
            chip.height,
            fill=True,
            alpha=0.3,
            color=colors[i],
            label=f'Chip {i+1}'
        )
        ax.add_patch(chip_rect)

        # Add bump markers and labels
        for bump in chip.bumps:
            bump_circle = patches.Circle(
                (bump.lbx, bump.lby),
                1,
                color='red',
                fill=True
            )
            ax.add_patch(bump_circle)

            ax.text(bump.lbx, bump.lby, str(bump.idx),
                   horizontalalignment='center',
                   verticalalignment='center',
                   fontsize=4)

    # Draw all nets
    for net in nets:
        # Draw all wires in this net with the same color
        for wire in net.wires:
            ax.plot([wire.x1, wire.x2], [wire.y1, wire.y2],
                    color=net.color,
                    linewidth=2,
                    alpha=0.7,
                    label=f'Net {net.name}')

        # Draw vias with the same color as the net
        for via in net.vias:
            via_marker = patches.Circle(
                (via.x, via.y),
                0.5,
                color=net.color,
                fill=True
            )
            ax.add_patch(via_marker)

if __name__ == "__main__":

    file_dir = sys.argv[1]
    testcase = os.path.basename(file_dir)
    gmp_file = os.path.join(file_dir, f"{testcase}.gmp")
    lg_file = os.path.join(file_dir, f"{testcase}.lg")
    output_file = os.path.join("draw", f"{testcase}.png")

    print(f"Reading GMP file: {gmp_file}")
    routing_area = parse_gmp_file(gmp_file)

    print(f"Reading LG file: {lg_file}")
    nets = parse_lg_file(lg_file)

    fig, ax = setup_plot(routing_area)
    plot_routing_area(ax, routing_area, nets)

    fig.savefig(output_file, dpi=300)
    print(f"Routing area layout image saved as \"{output_file}\"")
