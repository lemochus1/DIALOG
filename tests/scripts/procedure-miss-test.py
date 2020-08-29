from define.processes import *
from define.arguments import *
from define.xmlelements import *

from shared.running import *
from shared.evaluating import *

#===================================================================================================
# Parameters
#===================================================================================================

# Test
MESSAGE_SIZE        = 0
PAUSE_BETWEEN_CALLS = 1000
CALL_COUNT          = 20

# Run
CYCLE_COUNT    = getArgumentValue(CYCLE_COUNT_KEY, 1)
CYCLE_DURATION = getArgumentValue(CYCLE_DURATION_KEY, 5000)

#===================================================================================================
# Tokens
#===================================================================================================

CALLER_TOKENS = {
                 SIZE_TOKEN:     MESSAGE_SIZE,
                 DURATION_TOKEN: PAUSE_BETWEEN_CALLS,
                 REPEAT_TOKEN:   CALL_COUNT,
                }

#===================================================================================================
# Evaluation
#===================================================================================================

class ProcedureMissEvaluator(TestEvaluator):
    def __init__(self):
        pass

    def setupProcessResults(self):
        self.control_server_result.addControlledMessage(NOT_REGISTERED)
        self.control_server_result.addIgnoredMessage(NOT_REGISTERED)

    def evaluate(self):
        self.checkAllUnexpectedMessages()
        return UNAVAILABLE_TAG in self.test_process_results[PROCEDURE_CALLER][0].api_error_messages

#===================================================================================================
# Scripting
#===================================================================================================

if __name__ == "__main__":
    runner    = TestRunner()
    evaluator = ProcedureMissEvaluator()

    runner.addTestProcess(PROCEDURE_CALLER, tokens=CALLER_TOKENS)

    runner.runTest(evaluator, CYCLE_DURATION, CYCLE_COUNT)
