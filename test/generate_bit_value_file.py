import random

def generate_bit_value_file(filename, num_lines):
    # Open the file for writing
    with open(filename, 'w') as file:
        for _ in range(num_lines):
            # Generate the first number (bit count) randomly between 1 and 8
            bit_count = random.randint(1, 8)
            
            # Generate the second number (value) randomly between 0 and 255
            value = random.randint(0, 255)
            
            # Write the pair to the file
            file.write(f"{bit_count} {value}\n")

generate_bit_value_file('bit_value_pairs.txt', int(1e6))
