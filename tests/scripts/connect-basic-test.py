from define.processnames import *
from define.argumentkeys import *

from support.running import *
from support.evaluation import *

#===================================================================================================
# Parameters
#===================================================================================================

# Public and adjustable
## Test
PROCESS_COUNT = getArgumentValue(PROCESS_COUNT_KEY, 2)
SLEEP_TIME = 0
## Run
CYCLE_COUNT = getArgumentValue(CYCLE_COUNT_KEY, 5)

# Internal
CYCLE_DURATION = 2 # seconds

PROCESS_NAME = "dummy"

#===================================================================================================
# Evaluation
#===================================================================================================

NORMAL_LOG_START = 'Service  "Service"  has been subscribed by'

def evaluate(runner):
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

#===================================================================================================
# Scripting
#===================================================================================================

if __name__ == "__main__":
    runner = TestRunner()
    runner.addTestProcess(PROCESS_NAME, pause=SLEEP_TIME, count=PROCESS_COUNT)
    runner.runTest(evaluate, CYCLE_DURATION, CYCLE_COUNT)
