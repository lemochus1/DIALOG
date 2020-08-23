from define.processes import *
from define.arguments import *

from shared.running import *
from shared.evaluating import *

#===================================================================================================
# Parameters
#===================================================================================================

# Test
MESSAGE_SIZE        = 0
PAUSE_BETWEEN_CALLS = 200
CALL_COUNT          = 20

# Run
CYCLE_COUNT    = getArgumentValue(CYCLE_COUNT_KEY, 1)
CYCLE_DURATION = getArgumentValue(CYCLE_DURATION_KEY, 10)

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
        pass

    def evaluate(self):
        if self.noErrorOccured():
            self.checkAllUnexpectedMessages()
            handler = self.test_process_results[PROCEDURE_PROVIDER][0]
            if self.hasRegisteredSomething(handler):
                sender = self.test_process_results[PROCEDURE_CALLER][0]
                return self.areConsistent(sender, handler) and self.areConsistent(handler, sender)
        return False

#===================================================================================================
# Scripting
#===================================================================================================

if __name__ == "__main__":
    runner    = TestRunner()
    evaluator = ProcedureMissEvaluator()

    runner.addTestProcess(PROCEDURE_CALLER, tokens=CALLER_TOKENS)

    runner.runTest(evaluator, CYCLE_DURATION, CYCLE_COUNT)
