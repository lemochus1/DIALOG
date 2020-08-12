from define.processnames import *
from define.argumentkeys import *

from support.running import *
from support.evaluation import *

#===================================================================================================
# Parameters
#===================================================================================================

# Public and adjustable
## Test
SLEEP_TIME = 0.1
## Run
CYCLE_COUNT = getArgumentValue(CYCLE_COUNT_KEY, 5)

# Internal
CYCLE_DURATION = 2 # seconds

PROCESS_NAMES = [SERVICE_PROVIDER, COMMAND_HANDLER, PROCEDURE_HANDLER]

#===================================================================================================
# Evaluation
#===================================================================================================

def evaluate(runner):
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
    if len(control_result.unknown_messages) is not (len(PROCESS_NAMES) * 2):
        logger.log_and_print_message("Some process did not register anything on Control Server.")
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

    for name in PROCESS_NAMES:
        runner.addTestProcess(name, pause=SLEEP_TIME)

    runner.runTest(evaluate, CYCLE_DURATION, CYCLE_COUNT)
