import time
import sys

from utils.process import *
from utils.build import *

log_file = str(sys.argv[0]).replace(".py",".log")

testsSetup()
time.sleep(1)

runTestProcess("command-handler.proc", log_file)
runTestProcess("command-handler.proc", [log_file, "command-handler|command-handler2"])
time.sleep(1)

runTestProcess("direct-command-sender.proc", log_file)
