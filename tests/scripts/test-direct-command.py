# This Python file uses the following encoding: utf-8
from utils import *
import time

import sys

import subprocess

log_file = str(sys.argv[0]).replace(".py",".log")

testsSetup()
time.sleep(1)
runTestProcess("command-handler.proc", log_file)
runTestProcess("command-handler.proc", [log_file, "command-handler|command-handler2"])
time.sleep(1)
runTestProcess("direct-command-sender.proc", log_file)

subprocess.call(['xdotool', 'click', '1'])
