# noinspection PyUnresolvedReferences
import os, sys
print("searching files:")
lines = 0
num_files = 0
rootPath = os.path.dirname(os.path.realpath(__file__))
for path in [rootPath, rootPath + "/../libraries/internal/BraneScript/src"]:
	print(path)
	if not os.path.exists(path):
		print("path does not exist")
		sys.exit(1)
	if not os.path.isdir(path):
		print("path is not a directory")
		sys.exit(1)
	print_filenames = "-n" in sys.argv

	for root, dirs, files in os.walk(path):
		num_files += len(files)
		for name in files:
			if print_filenames:
				print(name)
			if ".h" in name or ".cpp" in name:
				file = open(root + "/" +  name)
				for line in file:
					lines += 1
				file.close()
print("\n")
print(f"Found {num_files} files")
print(f"found 2 lines of perfectly crafted code")
print(f"found {lines} lines of trash code")
input("press enter to close")
