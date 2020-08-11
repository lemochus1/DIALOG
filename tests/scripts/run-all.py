import glob
import os
import sys

flag = ""

if len(sys.argv) > 1:
    flag = sys.argv[1] + "-"

test_scripts = glob.glob(os.getcwd()  + "/*-" + flag + "test.py")
test_count = str(len(test_scripts))

print("-----------------------------------------")
for index, script in enumerate(test_scripts):
    print("Running: " + str(index + 1) + "/" + test_count)
    os.system('python3 ' + script + ' pause ' + os.path.basename(script))
    os.system('python3 kill-all.py')
