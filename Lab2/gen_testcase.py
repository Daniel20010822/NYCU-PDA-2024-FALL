import os
import random

def write_block(caseName, outlineWidth, outlineHeight, numBlocks, blocks, numTerminals, terminals):
    with open(f"./{caseName}/{caseName}.block", "w") as f_block:
        f_block.write(f"Outline: {outlineWidth} {outlineHeight}\n")
        f_block.write(f"NumBlocks: {numBlocks}\n")
        f_block.write(f"NumTerminals: {numTerminals}\n")
        for block in blocks:
            (name, width, height) = block
            f_block.write(f"{name:<5} {width:>5} {height:>5}\n")
        for terminal in terminals:
            (name, x, y) = terminal
            f_block.write(f"{name:<4} terminal {x:>8} {y:>8}\n")


def write_nets(caseName, numNets, nets):
    with open(f"./{caseName}/{caseName}.nets", "w") as f_nets:
        f_nets.write(f"NumNets: {numNets}\n")
        for net in nets:
            f_nets.write(f"NetDegree: {len(net)}\n")
            for name in net:
                f_nets.write(f"{name}\n")


def main():
    caseName         = input("Case name: ")
    numBlocks        = int(input("Number of blocks (1 ~ 500): "))
    numTerminals     = int(input("Number of Terminals (1 ~ 500): "))
    numNets          = int(input("Number of nets (1 ~ 500): "))
    oArea            = float(input("Outline area factor (1.0x ~ 2.0x): "))
    oAR              = float(input("Outline aspect ratio (0.5 ~ 2): "))


    # Create the directory
    try:
        os.mkdir(caseName)
        print(f"Directory '{caseName}' created successfully.")
    except FileExistsError:
        print(f"Your case '{caseName}' already exist, the contents have been overrided")
    except PermissionError:
        print(f"Permission denied: Unable to create '{caseName}'.")
    except Exception as e:
        print(f"An error occurred: {e}")

    totalArea = 0
    blocks = []
    terminals = []
    nets = []

    # Generate random blocks
    for i in range(numBlocks):
        name = f"m{i+1}"
        width  = random.randint(200, 300)
        height = random.randint(200, 300)
        blocks.append((name, width, height))
        totalArea += width * height

    # Calculate outline dimensions based on total block area
    outlineArea = totalArea * oArea
    outlineWidth = int((outlineArea * oAR) ** 0.5)
    outlineHeight = int(outlineArea / outlineWidth)
    # Define imaginary outline dimensions based on the factor
    imaginaryOutlineWidth  = random.randint(outlineWidth+100, outlineWidth+1000)  
    imaginaryOutlineHeight = random.randint(outlineHeight+100, outlineHeight+1000) 

    # Generate random terminals
    used_positions = set()  # Set to track used terminal positions (x, y)
    for i in range(numTerminals):
        name = "T" + str(i+1)
        # Generate a unique terminal position on the sides of the imaginary outline
        while True:
            side = random.choice(['top', 'bottom', 'left', 'right'])  # Choose a random side of the imaginary outline
            
            if side == 'top':
                x = random.randint(0, imaginaryOutlineWidth)  
                y = imaginaryOutlineHeight
            elif side == 'bottom':
                x = random.randint(0, imaginaryOutlineWidth)  
                y = 0  
            elif side == 'left':
                x = 0
                y = random.randint(0, imaginaryOutlineHeight)  
            else:  # 'right'
                x = imaginaryOutlineWidth
                y = random.randint(0, imaginaryOutlineHeight)  

            if (x, y) not in used_positions:  # Ensure the position is unique
                used_positions.add((x, y))  # Add the unique position to the set
                break
        terminals.append((name, x, y))

    # Generate random nets
    all_components = [block[0] for block in blocks] + [terminal[0] for terminal in terminals]
    for _ in range(numNets):
        net_size = random.randint(2, min(len(all_components), 5)) # Nets connect between 2 and 5 components
        net = random.sample(all_components, net_size)
        nets.append(net)


    write_block(caseName, outlineWidth, outlineHeight, numBlocks, blocks, numTerminals, terminals)
    write_nets(caseName, numNets, nets)

    print(f"File: '{caseName}/{caseName}.block' '{caseName}/{caseName}.nets' created successfully.")






if __name__ == '__main__':
    main()

