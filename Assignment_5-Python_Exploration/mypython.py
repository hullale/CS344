import os
import random

ASCII_MIN = ord('a')
ASCII_MAX = ord('z')

NUM_MIN = 1
NUM_MAX = 42

def make_file_with_random_letters(filename):
    file = open(filename, "w")

    for i in range(10):
        character_decimal = random.randint(ASCII_MIN, ASCII_MAX)
        file.write(chr(character_decimal))

    file.write('\n')
    file.close()

def print_out_file(filename):
    file = open(filename, "r")

    lines = file.readlines()

    for line in lines:
        print "File \"" + filename + "\" contains: " + line,

    file.close()

if __name__ == "__main__":
    current_pid = os.getpid()
    filenames = [
        str(current_pid) + "_1.txt",
        str(current_pid) + "_2.txt",
        str(current_pid) + "_3.txt"
    ]

    print "Generating files with names:"
    for name in filenames:
        make_file_with_random_letters(name)
        print name

    print "\nContents of files are:"
    for name in filenames:
        print_out_file(name)

    print "\nGenerating two random numbers:"
    first_number = random.randint(NUM_MIN, NUM_MAX)
    second_number = random.randint(NUM_MIN, NUM_MAX)

    print "First number is: " + str(first_number)
    print "Second number is: " + str(second_number)
    print "Product of the two numbers is: " + str(first_number+second_number)
