# This Python file uses the following encoding: utf-8
from utils import *
import time

testsSetup()
time.sleep(1)
runTestProcess("procedure-handler.proc")
time.sleep(1)
runTestProcess("procedure-caller.proc")
