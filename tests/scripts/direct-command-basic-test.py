from define.processes import *
from define.arguments import *

from shared.running import *
from shared.evaluating import *

#===================================================================================================
# Parameters
#===================================================================================================

# Test
## Public
TARGETED_HANDLER_COUNT = getArgumentValue(TARGETED_HANDLER_COUNT_KEY, 3)

SETUP_STRING = "Targeted handler count: {}".format(TARGETED_HANDLER_COUNT)
## Internal
SENDER_COUNT           = 1
HANDLER_COUNT          = 1
PAUSE_BETWEEN_CONNECTS = 400

MESSAGE_SIZE           = 0
PAUSE_BETWEEN_MESSAGES = 500
MESSAGE_COUNT          = 20

TARGETED_PROCESS_NAME = "targeted-handler"

# Run
CYCLE_COUNT    = getArgumentValue(CYCLE_COUNT_KEY, 1)
CYCLE_DURATION = getArgumentValue(CYCLE_DURATION_KEY, MESSAGE_COUNT * PAUSE_BETWEEN_MESSAGES + 2000)

#===================================================================================================
# Tokens
#===================================================================================================

SENDER_TOKENS = {
                 SIZE_TOKEN:             MESSAGE_SIZE,
                 DURATION_TOKEN:         PAUSE_BETWEEN_MESSAGES,
                 REPEAT_TOKEN:           MESSAGE_COUNT,
                 TARGETED_PROCESS_TOKEN: TARGETED_PROCESS_NAME,
                }

#===================================================================================================
# Evaluation
#===================================================================================================

class DirectCommandEvaluator(TestEvaluator):
    def __init__(self):
        pass

    def setupProcessResults(self):
        pass

    def evaluate(self):
        if self.noErrorOccured():
            self.checkAllUnexpectedMessages()
            handlers = self.test_process_results[TARGETED_PROCESS_NAME]
            if self.hasRegisteredSomething(handlers):
                sender = self.test_process_results[DIRECT_COMMAND_SANDER][0]
                return self.areConsistent(sender, handlers)
        return False

#===================================================================================================
# Scripting
#===================================================================================================

if __name__ == "__main__":
    runner    = TestRunner(setup_string=SETUP_STRING)
    evaluator = DirectCommandEvaluator()

    runner.addTestProcess(COMMAND_HANDLER)
    runner.addTestProcess(COMMAND_HANDLER,
                          name=TARGETED_PROCESS_NAME,
                          count=TARGETED_HANDLER_COUNT,
                          pause=PAUSE_BETWEEN_CONNECTS)
    runner.addTestProcess(DIRECT_COMMAND_SANDER, tokens=SENDER_TOKENS)

    runner.runTest(evaluator, CYCLE_DURATION, CYCLE_COUNT)
