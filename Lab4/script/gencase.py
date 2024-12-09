import os
import random
import math

class Chip:
    def __init__(self, lbx, lby, width, height):
        self.lbx = lbx
        self.lby = lby
        self.width = width
        self.height = height
        self.bumps = []

    def add_bump(self, bump):
        self.bumps.append(bump)

class Bump:
    def __init__(self, idx, x, y):
        self.idx = idx
        self.x = x
        self.y = y

class RoutingArea:
    def __init__(self, lbx, lby, width, height, gcell_dimension):
        self.lbx = lbx
        self.lby = lby
        self.width = width
        self.height = height
        self.gcell_dimension = gcell_dimension
        self.chips = []


    def add_chip(self, chip):
        self.chips.append(chip)

    def is_chip1_overlap(self, chip1, chip2):
        """Check if two chips overlap"""
        return not (chip1.lbx + chip1.width <= chip2.lbx or
                    chip1.lbx >= chip2.lbx + chip2.width or
                    chip1.lby + chip1.height <= chip2.lby or
                    chip1.lby >= chip2.lby + chip2.height)

    def add_bump(self, bump):
        self.bumps.append(bump)

    def write_gmp_file(self, filename='testcase.gmp'):
        with open(filename, 'w') as f:
            f.write(".ra\n")
            f.write(f"{self.lbx} {self.lby} {self.width} {self.height}\n")
            f.write(".g\n")
            f.write(f"{self.gcell_dimension[0]} {self.gcell_dimension[1]}\n")
            for chip in self.chips:
                f.write(f".c\n")
                f.write(f"{chip.lbx} {chip.lby} {chip.width} {chip.height}\n")
                f.write(".b\n")
                for bump in chip.bumps:
                    f.write(f"{bump.idx} {bump.x} {bump.y}\n")
                f.write("\n")


class Cost:
    def __init__(self, width, height):
        self.alpha = random.uniform(0.5, 2)
        self.beta  = random.uniform(0.5, 2)
        self.gamma = random.uniform(0.5, 2)
        self.delta = random.uniform(0.5, 2)
        self.viaCost = random.uniform(2, 5)
        self.cost_map_l1 = [[random.uniform(0, 5) for _ in range(width)] for _ in range(height)]
        self.cost_map_l2 = [[random.uniform(0, 5) for _ in range(width)] for _ in range(height)]

    def write_cst_file(self, filename='testcase.cst'):
        with open(filename, 'w') as f:
            f.write(f".alpha {self.alpha}\n")
            f.write(f".beta  {self.beta}\n")
            f.write(f".gamma {self.gamma}\n")
            f.write(f".delta {self.delta}\n")
            f.write(f".v\n")
            f.write(f"{self.viaCost}\n")
            f.write(".l\n")
            for row in self.cost_map_l1:
                f.write(" ".join(map(str, row)))
                f.write("\n")
            f.write(".l\n")
            for row in self.cost_map_l2:
                f.write(" ".join(map(str, row)))
                f.write("\n")

def generate_routing_area():
    """Generate random routing area parameters"""
    lbx, lby = random.randint(0, 100), random.randint(0, 100)
    gcell_dimension = (10, 10)
    # gcell_dimension = (random.randint(5, 20), random.randint(5, 20))
    width  = gcell_dimension[0] * random.randint(50, 200)
    height = gcell_dimension[1] * random.randint(50, 200)
    ra = RoutingArea(lbx, lby, width, height, gcell_dimension)
    return ra

def generate_chips(ra, bump_number):
    """Generate random chips"""
    gcell_width, gcell_height = ra.gcell_dimension

    ### Generation of chip1
    # Generate chip1 (in representation of grid cells)
    width1  = random.randint(10, 50)
    height1 = random.randint(10, 50)
    lbx1    = random.randint(1, ra.width  // gcell_width  - width1  - 5)
    lby1    = random.randint(1, ra.height // gcell_height - height1 - 5)

    # Convert grid cells to real coordinates
    chip1_lbx    = lbx1 * gcell_width
    chip1_lby    = lby1 * gcell_height
    chip1_width  = width1  * gcell_width
    chip1_height = height1 * gcell_height
    chip1 = Chip(chip1_lbx, chip1_lby, chip1_width, chip1_height)
    ra.add_chip(chip1)

    # Generate bumps for chip1
    for i in range(bump_number):
        while True:
            x = random.randint(0, width1  - 1) * gcell_width
            y = random.randint(0, height1 - 1) * gcell_height
            bump = Bump(i + 1, x, y)
            if all(bump.x != b.x or bump.y != b.y for b in chip1.bumps):
                break
        chip1.add_bump(bump)


    ### Generation of chip2
    # chip2 should not overlap with chip1

    while True:
        width2  = random.randint(10, 50)
        height2 = random.randint(10, 50)
        lbx2    = random.randint(1, ra.width  // gcell_width  - width2  - 5)
        lby2    = random.randint(1, ra.height // gcell_height - height2 - 5)

        chip2_lbx    = lbx2 * gcell_width
        chip2_lby    = lby2 * gcell_height
        chip2_width  = width2  * gcell_width
        chip2_height = height2 * gcell_height
        chip2 = Chip(chip2_lbx, chip2_lby, chip2_width, chip2_height)

        if not ra.is_chip1_overlap(chip1, chip2):
            ra.add_chip(chip2)
            break

    # Generate bumps for chip2
    for i in range(bump_number):
        while True:
            x = random.randint(0, width2  - 1) * gcell_width
            y = random.randint(0, height2 - 1) * gcell_height
            bump = Bump(i + 1, x, y)
            if all(bump.x != b.x or bump.y != b.y for b in chip2.bumps):
                break
        chip2.add_bump(bump)

def write_gcl_file(width, height, filename="testcase.gcl"):
    with open(filename, 'w') as f:
        f.write(f".ec\n")
        for _ in range(width*height):
            f.write(f"{random.randint(1, 3)} {random.randint(1, 3)}\n")






if __name__ == '__main__':

    # User inputs
    testcase_name = input("Enter the testcase name: ")
    bump_number = int(input("Enter the number of bumps: "))

    # Create the directory
    if os.path.exists(testcase_name):
        print(f"Warning: Overwrite {testcase_name}")
    else:
        os.mkdir(testcase_name)

    # Generate Routing Area
    ra = generate_routing_area()
    generate_chips(ra, bump_number)
    ra.write_gmp_file(f"{testcase_name}/{testcase_name}.gmp")

    # Generate Cost
    cst = Cost(ra.width // ra.gcell_dimension[0], ra.height // ra.gcell_dimension[1])
    cst.write_cst_file(f"{testcase_name}/{testcase_name}.cst")

    # Generate GCell capacities
    write_gcl_file(
        ra.width // ra.gcell_dimension[0],
        ra.height // ra.gcell_dimension[1],
        filename=f"{testcase_name}/{testcase_name}.gcl")


