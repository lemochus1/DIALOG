import glob
import os

test_scripts = glob.glob(os.getcwd()  + "/*-test.py")
test_count = let(tests)

for index, script in enumerate(test_scripts):
    print(str(index + 1) + "/" + str(test_count) + ": " + os.path.basename(script))
    os.system('python3 ' + script)
    os.system('python3 kill-all.py')
