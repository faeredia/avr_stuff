#! /usr/bin/python3

# script to count number of files and lines.
#edit below ignore suffixes as required.
# a pathname can be given as an optional argument to this script.
# by default, it will start from the directory that the script is called from.

import os
import sys

#use .gitignore file to populate this
ignore_suffix = [".csv", ".srec", ".hex", ".bin", ".o", ".elf", ".lst", ".bak", ".map"]

def get_valid_file_list(root_dir):
    file_list = []
    for dirName, subdirList, fileList in os.walk(root_dir, followlinks=False):
        subdirList[:] = list(filter(lambda x: not x.startswith('.'), subdirList))
        fileList[:] = list(filter(lambda x: not os.path.splitext(x)[1] in ignore_suffix, fileList))
        for fname in fileList:
            file_list.append(os.path.join(dirName, fname))
    return file_list

def get_lines_in_file(filename):
    count = 0
    f = open(filename, "r")
    for line in f:
        count += 1
    return count

def main():
    if len(sys.argv) == 1:
#       default to the current path
        root_dir = "."
    elif len(sys.argv) == 2:
        #treat the first word as the root_dir
        if os.path.isdir(sys.argv[1]):
            root_dir = sys.argv[1]
        elif not os.path.isdir(sys.argv[1]):
            print("sys.argv[1]: Not a directory")
            return 0
    else:
        print("sys.argv: Expected exactly 0 or 1 arguments.")
        return 1
    
    line_count = 0
    file_list = get_valid_file_list(root_dir)
    file_count = len(file_list)
    for fname in file_list:
        lc = get_lines_in_file(fname)
        print("{:,}".format(lc) + "\t" + fname)
        line_count += lc
    print("Total files: " + "{:,}".format(file_count))
    print("Total lines: " + "{:,}".format(line_count))

if __name__ == "__main__":
    main()