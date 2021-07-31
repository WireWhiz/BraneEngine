import os
print("searching")
lines = 0
path = os.path.dirname(os.path.realpath(__file__));
for root, dirs, files in os.walk(path):
	for name in files:
		if ".h" in name or ".cpp" in name:
			file = open(root + "\\" +  name)
			for line in file:
				lines += 1
			
			file.close()
print(f"found 2 lines of perfectly crafted code")
print(f"found {lines} lines of trash code")
input("press enter to close")
