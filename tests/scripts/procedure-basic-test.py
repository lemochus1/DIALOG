from define.processes import *
from define.arguments import *

from shared.running import *
from shared.evaluating import *

#===================================================================================================
# Parameters
#===================================================================================================

# Test
## Public
CALL_DURATION = getArgumentValue(CALL_DURATION_KEY, 300)

SETUP_STRING = "Call duration: {} ms".format(CALL_DURATION)
## Internal
PROVIDER_COUNT         = 1
CALLER_COUNT           = 1
PAUSE_BETWEEN_CONNECTS = 400

MESSAGE_SIZE        = 0
PAUSE_BETWEEN_CALLS = 200
CALL_DURATION       = 300
CALL_COUNT          = 20

# Run
CYCLE_COUNT    = getArgumentValue(CYCLE_COUNT_KEY, 1)
CYCLE_DURATION = getArgumentValue(CYCLE_DURATION_KEY,
                                  CALL_COUNT * (PAUSE_BETWEEN_CALLS + CALL_DURATION) + 1000)

#===================================================================================================
# Tokens
#===================================================================================================

CALLER_TOKENS = {
                 SIZE_TOKEN:     MESSAGE_SIZE,
                 DURATION_TOKEN: PAUSE_BETWEEN_CALLS,
                 REPEAT_TOKEN:   CALL_COUNT,
                }

PROVIDER_TOKENS = {
                   DURATION_TOKEN: CALL_DURATION
                  }

#===================================================================================================
# Evaluation
#===================================================================================================

class ProcedureEvaluator(TestEvaluator):
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
    runner    = TestRunner(setup_string=SETUP_STRING)
    evaluator = ProcedureEvaluator()

    runner.addTestProcess(PROCEDURE_PROVIDER,
                          tokens=PROVIDER_TOKENS,
                          count=PROVIDER_COUNT,
                          pause=PAUSE_BETWEEN_CONNECTS)
    runner.addTestProcess(PROCEDURE_CALLER,
                          tokens=CALLER_TOKENS,
                          count=CALLER_COUNT)

    runner.runTest(evaluator, CYCLE_DURATION, CYCLE_COUNT)
