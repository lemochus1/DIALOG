import time

from utils.build import *
from utils.process import *


testsSetup()
time.sleep(1)

runTestProcess("procedure-handler.proc")
time.sleep(1)

runTestProcess("procedure-caller.proc")
