from define.processes import *
from define.arguments import *
from define.xmlelements import *

from shared.running import *
from shared.evaluating import *

#===================================================================================================
# Parameters
#===================================================================================================

# Test
## Public
HANDLER_COUNT          = getArgumentValue(HANDLER_COUNT_KEY, 3)
PAUSE_BETWEEN_MESSAGES = getArgumentValue(PAUSE_BETWEEN_MESSAGES_KEY, 500)
MESSAGE_SIZE           = getArgumentValue(MESSAGE_SIZE_KEY, 0)
MESSAGE_COUNT          = getArgumentValue(MESSAGE_COUNT_KEY, 20)

SETUP_STRING = "Handler count: {}; Messages pause: {}ms, size: {}B, count: {}".format(HANDLER_COUNT,
                                                                             PAUSE_BETWEEN_MESSAGES,
                                                                             MESSAGE_SIZE,
                                                                             MESSAGE_COUNT)
## Internal
SENDER_COUNT           = 1
PAUSE_BETWEEN_CONNECTS = 300

# Run
CYCLE_COUNT    = getArgumentValue(CYCLE_COUNT_KEY, 1)
SAFE_TIMEOUT   = 2000
CYCLE_DURATION = MESSAGE_COUNT * PAUSE_BETWEEN_MESSAGES + getArgumentValue(CYCLE_DURATION_KEY,
                                                                           SAFE_TIMEOUT)

#===================================================================================================
# Tokens
#===================================================================================================

SENDER_TOKENS = {DURATION_TOKEN: PAUSE_BETWEEN_MESSAGES,
                 SIZE_TOKEN:     MESSAGE_SIZE,
                 REPEAT_TOKEN:   MESSAGE_COUNT,
                }

#===================================================================================================
# Evaluation
#===================================================================================================

class CommandEvaluator(TestEvaluator):
    def __init__(self):
        pass

    def setupProcessResults(self):
        pass

    def evaluate(self):
        if self.noErrorOccured():
            self.checkAllUnexpectedMessages()
            sender = self.test_process_results[COMMAND_SANDER][0]
            handlers = self.test_process_results[COMMAND_HANDLER]
            if self.hasRegisteredSomething(handlers):
                return self.areConsistent(sender, handlers)
        return False

#===================================================================================================
# Scripting
#===================================================================================================

if __name__ == "__main__":
    runner    = TestRunner(setup_string=SETUP_STRING)
    evaluator = CommandEvaluator()

    runner.addTestProcess(COMMAND_HANDLER, count=HANDLER_COUNT, pause=PAUSE_BETWEEN_CONNECTS)
    runner.addTestProcess(COMMAND_SANDER, tokens=SENDER_TOKENS, count=SENDER_COUNT)

    runner.runTest(evaluator, CYCLE_DURATION, CYCLE_COUNT)
