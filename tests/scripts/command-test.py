import time

from utils.runner import TestRunner
from utils.runner import ResultObject

runner = TestRunner()

runner.setup()
time.sleep(1)
runner.startTestProcess("command-handler")
runner.startTestProcess("command-handler")
time.sleep(1)
runner.startTestProcess("command-sender")
runner.waitWhileTesting(6)
runner.cleanup()
results = runner.getResults()


