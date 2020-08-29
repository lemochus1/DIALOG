from define.processes import *
from define.arguments import *

from shared.running import *
from shared.evaluating import *

#===================================================================================================
# Parameters
#===================================================================================================

# Test
MESSAGE_SIZE           = 0
PAUSE_BETWEEN_MESSAGES = 500
MESSAGE_COUNT          = 20

TARGETED_PROCESS_NAME = "targeted-handler"

# Run
CYCLE_COUNT    = getArgumentValue(CYCLE_COUNT_KEY, 1)
CYCLE_DURATION = getArgumentValue(CYCLE_DURATION_KEY, 3000)

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
        self.control_server_result.addControlledMessage(NOT_REGISTERED)
        self.control_server_result.addIgnoredMessage(NOT_REGISTERED)

    def evaluate(self):
        if self.noErrorOccured():
            self.checkAllUnexpectedMessages()
            return bool(self.control_server_result.controlled_messages)
        return False

#===================================================================================================
# Scripting
#===================================================================================================

if __name__ == "__main__":
    runner    = TestRunner()
    evaluator = DirectCommandEvaluator()

    runner.addTestProcess(DIRECT_COMMAND_SANDER, tokens=SENDER_TOKENS)

    runner.runTest(evaluator, CYCLE_DURATION, CYCLE_COUNT)
