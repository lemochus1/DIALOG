from define.processnames import *
from define.argumentkeys import *

from support.running import *
from support.evaluation import *

#===================================================================================================
# Parameters
#===================================================================================================

# Public and adjustable
## Test
HANDLER_COUNT = getArgumentValue(HANDLER_COUNT_KEY, 2)
## Run
CYCLE_COUNT = getArgumentValue(CYCLE_COUNT_KEY, 1)

# Internal
CYCLE_DURATION = 15 # seconds

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

#===================================================================================================
# Scripting
#===================================================================================================

if __name__ == "__main__":
    runner = TestRunner()

    runner.addTestProcess(COMMAND_HANDLER, count=HANDLER_COUNT)
    runner.addTestProcess(COMMAND_SANDER)

    runner.runTest(evaluate, CYCLE_DURATION, CYCLE_COUNT)
