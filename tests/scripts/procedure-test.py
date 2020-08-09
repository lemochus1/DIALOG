import time

from utils.runner import TestRunner

runner = TestRunner()

runner.setup()
time.sleep(2)
runner.startTestProcess("procedure-handler")
time.sleep(1)
runner.startTestProcess("procedure-caller")
runner.waitWhileTesting(6)
runner.cleanup()

