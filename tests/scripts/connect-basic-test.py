import time
import os

from utils.runner import TestRunner
from utils.evaluation import *
from utils.define import *

SERVICE_SUBSCRIBER = "service-subscriber"
SERVICE_PROVIDER = "service-provider"
HANDLERS_COUNT = 2
TEST_DURATION = 10

PROCESS_NAME = "dummy"

PROCESS_COUNT = 5
SLEEP_TIME = 0

CYCLES = 5

NORMAL_LOG_START = 'Service  "Service"  has been subscribed by'


def evaluateTest(runner):
    print("Evaluated...")
    passed = True

    logger = TestEvaluationLogger()
    logger.log_evaluation_start()

    if hasHappendSomethingStrange(runner.getResults(), logger):
        passed = False

    control_result = runner.getControlServerResult()
    if controlServerError(control_result.unknown_messages, logger):
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
    time.sleep(0.5)
    for j in range(PROCESS_COUNT):
        if SLEEP_TIME:
            time.sleep(SLEEP_TIME)
        runner.startTestProcess(PROCESS_NAME)
    time.sleep(2)
    evaluateTest(runner)
print(TERMINAL_SEPARATOR)
