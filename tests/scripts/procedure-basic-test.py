from define.processnames import *
from define.argumentkeys import *

from support.running import *
from support.evaluation import *

#===================================================================================================
# Parameters
#===================================================================================================

# Public and adjustable
## Test

## Run
CYCLE_COUNT = getArgumentValue(CYCLE_COUNT_KEY, 1)

# Internal
CYCLE_DURATION = 10 # seconds

#===================================================================================================
# Evaluation
#===================================================================================================

def evaluate(runner):
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

#===================================================================================================
# The scripting starts here
#===================================================================================================

if __name__ == "__main__":
    runner = TestRunner()

    runner.addTestProcess(PROCEDURE_HANDLER, pause=0.5)
    runner.addTestProcess(PROCEDURE_CALLER)

    runner.runTest(evaluate, CYCLE_DURATION, CYCLE_COUNT)
