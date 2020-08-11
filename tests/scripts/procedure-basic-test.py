import time

from utils.runner import TestRunner
from utils.evaluation import *

PROCEDURE_HANDLER = "procedure-handler"
PROCEDURE_CALLER = "procedure-caller"

TEST_DURATION = 10


def evaluateTest(runner):
    print("Evaluated...")
    passed = True

    logger = TestEvaluationLogger()
    logger.log_evaluation_start()

    all_results = runner.getResults()

    if hasHappendSomethingStrange(all_results, logger):
        passed = False
    else:
        handler = all_results[PROCEDURE_HANDLER][0]
        caller = all_results[PROCEDURE_CALLER][0]

        if handler.registered_counter is not 1:
            logger.log_and_print_message("Procedure was not registered.")
            passed = False

        lost_commands = caller.sent_counter - handler.received_counter
        if lost_commands is not 0:
            logger.log_and_print_message(str(lost_commands) + " commands were send but not received.")
            passed = False

        if not (handler.received_messages == caller.sent_messages and handler.sent_messages == caller.received_messages):
            logger.log_and_print_message("Some messages were delivered demaged.")
            passed =False
    if passed:
        logger.log_and_print_message("Passed!", False)
    else:
        logger.log_and_print_message("Failed!", False)
    runner.cleanup(not passed)

runner = TestRunner()

runner.setup()
time.sleep(0.2)
runner.startTestProcess(PROCEDURE_HANDLER)
time.sleep(0.5)
runner.startTestProcess(PROCEDURE_CALLER)
runner.waitWhileTesting(TEST_DURATION)
evaluateTest(runner)
