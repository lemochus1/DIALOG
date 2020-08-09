import time
import sys

from utils.runner import TestRunner

runner = TestRunner()

runner.setup()
time.sleep(2)
runner.startTestProcess("command-handler")
runner.startTestProcess("command-handler", tokens="command-handler|command-handler2")
time.sleep(1)
runner.startTestProcess("direct-command-sender")
runner.waitWhileTesting(10)
runner.cleanup()
