# This Python file uses the following encoding: utf-8
from utils import *
import time

testsSetup()
time.sleep(1)
runTestProcess("command-handler.proc")
runTestProcess("command-handler.proc")
time.sleep(1)
runTestProcess("command-sender.proc")
# if__name__ == "__main__":
#     pass
