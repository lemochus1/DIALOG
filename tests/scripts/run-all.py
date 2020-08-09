import glob
import os

test_scripts = glob.glob(os.getcwd()  + "/*-test.py")
test_count = str(len(test_scripts))

print("-----------------------------------------")
for index, script in enumerate(test_scripts):
    print("Running: " + str(index + 1) + "/" + test_count)
    os.system('python3 ' + script + ' auto ' + os.path.basename(script))
    os.system('python3 kill-all.py')
