# This Python file uses the following encoding: utf-8
import subprocess
import shutil

original = "test.py"
target = "test2.py"

shutil.copyfile(original, target)


#args = ['test.exe']
#subprocess.call(args)
