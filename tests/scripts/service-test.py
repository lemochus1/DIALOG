import time

from utils.runner import TestRunner

runner = TestRunner()

runner.setup()
time.sleep(2)
runner.startTestProcess("service-subscriber")
runner.startTestProcess("service-provider")
time.sleep(1)
runner.startTestProcess("service-subscriber")
runner.waitWhileTesting(10)
runner.cleanup()
