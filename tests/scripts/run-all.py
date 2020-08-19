import glob
import os
import sys

from define.arguments import *


if __name__ == "__main__":
    flag = ""

    if len(sys.argv) > 1:
        flag = sys.argv[1] + "-"

    test_scripts = glob.glob(os.getcwd()  + "/*-" + flag + "test.py")
    test_count = str(len(test_scripts))

    print("-----------------------------------------")
    for index, script in enumerate(test_scripts):
        print("Test: " + str(index + 1) + "/" + test_count)
        os.system('python3 ' + script + ' pause ' + TEST_NAME_KEY + "=" + os.path.basename(script))
        os.system('python3 kill-all.py')
