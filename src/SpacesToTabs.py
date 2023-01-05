# noinspection PyUnresolvedReferences
import os, sys

print("searching files:")
lines = 0
path = os.path.dirname(os.path.realpath(__file__))
print_filenames = "-n" in sys.argv
num_files = 0

for root, dirs, files in os.walk(path):
    num_files += len(files)
    for name in files:
        if print_filenames:
            print(name)
        if ".h" in name or ".cpp" in name:
            file = open(root + "\\" + name, "r+")
            contents = file.read()
            newContents = ""
            for char in contents:
                if char == '\t':
                    newContents += "    "
                else:
                    newContents += char
            file.seek(0)
            file.write(newContents)
            file.close()
print("\n")
print(f"Found {num_files} files")
print(f"found 2 lines of perfectly crafted code")
print(f"found {lines} lines of trash code")
input("press enter to close")
