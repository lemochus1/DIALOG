import time
import os

from utils.runner import TestRunner
from utils.evaluation import *
from utils.define import *


SERVICE_PROVIDER = "service-provider"
COMMAND_HANDLER = "command-handler"
PROCEDURE_HANDLER = "procedure-handler"

PROCESS_NAMES = [SERVICE_PROVIDER, COMMAND_HANDLER, PROCEDURE_HANDLER]

SLEEP_TIME = 0.1

CYCLES = 5

def evaluateTest(runner):
    print("Evaluated...")
    passed = True

    logger = TestEvaluationLogger()
    logger.log_evaluation_start()

    if hasHappendSomethingStrange(runner.getResults(), logger):
        passed = False

    for name, result_object in runner.getResults().items():
        result_object = result_object[0]
        if result_object.registered_counter is not 1:
            logger.log_and_print_message(name + "did not send register message.")
            passed = False

    control_result = runner.getControlServerResult()
    if controlServerError(control_result.unknown_messages, logger):
        passed = False
    if len(control_result.unknown_messages) is not (len(PROCESS_NAMES) * 2 + 1):
        logger.log_and_print_message("Some process did not register anything on Control Server.")
        passed = False

    if passed:
        logger.log_and_print_message("Passed!", False)
    else:
        logger.log_and_print_message("Failed!", False)
    runner.cleanup(not passed, False)


runner = TestRunner()

for i in range(CYCLES):
    runner.clear()
    runner.setup()
    time.sleep(0.2)

    for name in PROCESS_NAMES:
        runner.startTestProcess(name)
        time.sleep(SLEEP_TIME)
    time.sleep(6)
    evaluateTest(runner)
print(TERMINAL_SEPARATOR)
