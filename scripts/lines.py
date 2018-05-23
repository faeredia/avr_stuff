#! /usr/bin/python3

################################
#   Yuck!
#       This code is horrible
# It works,
#   but, my EYES hurt!
#           Don't look at me...
###############################

# call this script with:
#       scripts/lines.py
# from the project root directory

import os

#use .gitignore file to populate this
ignore_suffix = [".csv", ".srec", ".hex", ".bin"]


def recursive_file_line_count(path):
    global files
    global lines
    dir_list = []
    with os.scandir(path) as it:
        for entry in it:
            if not (entry.name.startswith('.') or os.path.splitext(entry.name)[1] in ignore_suffix):
                if entry.is_file():
                    files += 1
                    num = 0
                    fl = open(entry.path, "r")
                    for line in fl:
                        num += 1
                    #print(entry.path + " contains " + str(num))
                    print("Lines: " + "{:,}".format(num) + "\t" + entry.path)
                    lines += num
                elif entry.is_dir():
                    #print("adding: " + entry.path + " to scan list")
                    dir_list.append(entry.path)
    for f in dir_list:
        #print("scanning: " + f)
        recursive_file_line_count(f)

files = 0
lines = 0
recursive_file_line_count('.')
print("Total number of files:\t" + "{:,}".format(files))
print("Total lines of code:\t" + "{:,}".format(lines))