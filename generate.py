#!/usr/bin/env python3
import sys

def generate_file(size_in_bytes, filename="output.txt"):
    with open(filename, 'w') as file:
        # Writing 'a' character repeatedly to reach the specified size
        file.write('a' * size_in_bytes)

if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("Usage: {} <number_of_bytes>".format(sys.argv[0]))
        sys.exit(1)

    size_in_bytes = int(sys.argv[1])
    generate_file(size_in_bytes)
    print("File generated with {} bytes.".format(size_in_bytes))

