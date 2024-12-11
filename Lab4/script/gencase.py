import os
import random
import matplotlib.pyplot as plt
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
    def __init__(self):
        self.lbx = random.randint(0, 100)
        self.lby = random.randint(0, 100)
        self.gcell_dimension = (random.randint(5, 15), random.randint(5, 15))
        self.width  = self.gcell_dimension[0] * random.randint(600, 1500)
        self.height = self.gcell_dimension[1] * random.randint(600, 1000)
        self.chips = []

    def add_bump(self, bump):
        self.bumps.append(bump)

    def is_chip1_overlap(self, chip1, chip2):
        """Check if two chips overlap"""
        return not (chip1.lbx + chip1.width <= chip2.lbx or
                    chip1.lbx >= chip2.lbx + chip2.width or
                    chip1.lby + chip1.height <= chip2.lby or
                    chip1.lby >= chip2.lby + chip2.height)

    def generate_chips(self, bump_number):
        gcell_width, gcell_height = self.gcell_dimension

        ### Generation of chip1
        # Generate chip1 (in representation of grid cells)
        width1  = random.randint(40, 80)
        height1 = random.randint(40, 80)
        lbx1    = random.randint(1, self.width  // gcell_width  // 2)
        lby1    = random.randint(1, self.height // gcell_height // 2)

        # Convert grid cells to real coordinates
        chip1_lbx    = lbx1 * gcell_width
        chip1_lby    = lby1 * gcell_height
        chip1_width  = width1  * gcell_width
        chip1_height = height1 * gcell_height
        chip1 = Chip(chip1_lbx, chip1_lby, chip1_width, chip1_height)
        self.chips.append(chip1)

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
            width2  = random.randint(40, 80)
            height2 = random.randint(40, 80)
            lbx2    = random.randint(self.width  // gcell_width  // 2, self.width  // gcell_width  - width2  - 5)
            lby2    = random.randint(self.height // gcell_height // 2, self.height // gcell_height - height2 - 5)

            chip2_lbx    = lbx2 * gcell_width
            chip2_lby    = lby2 * gcell_height
            chip2_width  = width2  * gcell_width
            chip2_height = height2 * gcell_height
            chip2 = Chip(chip2_lbx, chip2_lby, chip2_width, chip2_height)

            if not self.is_chip1_overlap(chip1, chip2):
                self.chips.append(chip2)
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
        # self.cost_map_l1 = [[random.uniform(0, 5) for _ in range(width)] for _ in range(height)]
        # self.cost_map_l2 = [[random.uniform(0, 5) for _ in range(width)] for _ in range(height)]
        self.cost_map_l1 = self.generate_gaussian_cost_map(width, height)
        self.cost_map_l2 = self.generate_gaussian_cost_map(width, height)

    def generate_gaussian_cost_map(self, width, height):
        """生成具有多個高斯分佈中心的成本地圖"""
        cost_map = [[0 for _ in range(width)] for _ in range(height)]

        # 創建多個高斯分佈中心點
        centers = [
            (width//4, height//4),      # 左下
            (3*width//4, height//4),    # 右下
            (width//4, 3*height//4),    # 左上
            (3*width//4, 3*height//4),  # 右上
            (width//2, height//2)       # 中心
        ]

        # 為每個中心點生成高斯分佈
        for center_x, center_y in centers:
            std_dev = min(width, height) // 8
            intensity = random.uniform(3, 8)  # 隨機強度

            for y in range(height):
                for x in range(width):
                    # 計算到中心的距離
                    distance = ((x - center_x) ** 2 + (y - center_y) ** 2) ** 0.5
                    # 計算高斯值
                    gaussian_value = intensity * math.exp(-(distance ** 2) / (2 * std_dev ** 2))
                    cost_map[y][x] += gaussian_value

        # 標準化成本值到合理範圍 (0-5)
        max_value = max(max(row) for row in cost_map)
        cost_map = [[val * 5 / max_value for val in row] for row in cost_map]

        return cost_map

    def write_cst_file(self, filename='testcase.cst'):
        with open(filename, 'w') as f:
            f.write(f".alpha {self.alpha:.2f}\n")
            f.write(f".beta  {self.beta:.2f}\n")
            f.write(f".gamma {self.gamma:.2f}\n")
            f.write(f".delta {self.delta:.2f}\n")
            f.write(f".v\n")
            f.write(f"{self.viaCost:.2f}\n")
            f.write(".l\n")
            for row in self.cost_map_l1:
                f.write(" ".join(f"{val:.2f}" for val in row))
                f.write("\n")
            f.write(".l\n")
            for row in self.cost_map_l2:
                f.write(" ".join(f"{val:.2f}" for val in row))
                f.write("\n")


def write_gcl_file(width, height, filename="testcase.gcl"):
    with open(filename, 'w') as f:
        f.write(f".ec\n")
        for _ in range(width*height):
            f.write(f"{random.randint(1, 3)} {random.randint(1, 3)}\n")

def draw_cost_map(cost_map_l1, cost_map_l2, testcase_name):
    """Draw cost map"""
    # Save the figure for layer 1
    plt.imshow(cost_map_l1, cmap='hot', interpolation='nearest')
    plt.title('Cost Map Layer 1')
    plt.colorbar()
    plt.savefig(f"{testcase_name}/M1.png", dpi=300, bbox_inches='tight')
    plt.close()

    # Save the figure for layer 2
    plt.imshow(cost_map_l2, cmap='hot', interpolation='nearest')
    plt.title('Cost Map Layer 2')
    plt.colorbar()
    plt.savefig(f"{testcase_name}/M2.png", dpi=300, bbox_inches='tight')
    plt.close()

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
    ra = RoutingArea()
    ra.generate_chips(bump_number)
    ra.write_gmp_file(f"{testcase_name}/{testcase_name}.gmp")

    # Generate Cost
    cst = Cost(ra.width // ra.gcell_dimension[0], ra.height // ra.gcell_dimension[1])
    cst.write_cst_file(f"{testcase_name}/{testcase_name}.cst")

    # Generate GCell capacities
    write_gcl_file(
        ra.width // ra.gcell_dimension[0],
        ra.height // ra.gcell_dimension[1],
        filename=f"{testcase_name}/{testcase_name}.gcl")

    # Draw cost map
    draw_cost_map(cst.cost_map_l1, cst.cost_map_l2, testcase_name)



