import time

from utils.runner import TestRunner
from utils.evaluation import *


COMMAND_HANDLER = "command-handler"
COMMAND_SANDER = "command-sender"
HANDLERS_COUNT = 2
TEST_DURATION = 15


def evaluateTest(runner):
    print("Evaluated...")
    passed = True

    logger = TestEvaluationLogger()
    logger.log_evaluation_start()

    all_results = runner.getResults()

    if hasHappendSomethingStrange(all_results, logger):
        passed = False
    else:
        handlers_results = all_results[COMMAND_HANDLER]
        sender_result = all_results[COMMAND_SANDER][0]

        for handler in handlers_results:
            if handler.registered_counter is not 1:
                logger.log_and_print_message("Command was not registered.")
                passed = False
                break

            lost_commands = sender_result.sent_counter - handler.received_counter
            if lost_commands == 1:
                logger.log_and_print_message(str(lost_commands) + " commands were send but not received.")
                passed = False

            if passed and not handler.received_messages == sender_result.sent_messages:
                logger.log_and_print_message("Some message was delivered demaged.")
                passed = False

    if passed:
        logger.log_and_print_message("Passed!", False)
    else:
        logger.log_and_print_message("Failed!", False)
    runner.cleanup(not passed)


runner = TestRunner()

runner.setup()
time.sleep(2)

for i in range(HANDLERS_COUNT):
    runner.startTestProcess(COMMAND_HANDLER)
    time.sleep(1)
time.sleep(1)

runner.startTestProcess(COMMAND_SANDER)
runner.waitWhileTesting(TEST_DURATION)

evaluateTest(runner)
