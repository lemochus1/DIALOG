import time

from utils.runner import TestRunner
from utils.evaluation import *


COMMAND_HANDLER = "command-handler"
COMMAND_SANDER = "command-sender"
HANDLERS_COUNT = 2
TEST_DURATION = 15


def evaluateTest(all_results):
    print("Evaluated...")
    if hasHappendSomethingStrange(all_results):
        return False

    handlers_results = all_results[COMMAND_HANDLER]
    sender_result = all_results[COMMAND_SANDER][0]

    for handler in handlers_results:
        lost_commands = sender_result.sent_counter - handler.received_counter
        if lost_commands == 1:
            print("-> Some command was send but not received.")
            return False
        elif lost_commands != 0:
            print("->" + str(lost_commands) + " commands were send but not received.")
            return False
        if not handler.received_messages == sender_result.sent_messages:
            print("-> Some message was delivered demaged.")
            return False
    return True


runner = TestRunner()

runner.setup()
time.sleep(2)

for i in range(HANDLERS_COUNT):
    runner.startTestProcess(COMMAND_HANDLER)
    time.sleep(1)
time.sleep(1)

runner.startTestProcess(COMMAND_SANDER)
runner.waitWhileTesting(TEST_DURATION)

all_results = runner.getResults()

if evaluateTest(all_results):
    print("Passed!")
    runner.cleanup(False)
else:
    print("Failed!")
    runner.cleanup(True)
