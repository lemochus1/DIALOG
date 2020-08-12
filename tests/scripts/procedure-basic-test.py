from define.processes import *
from define.arguments import *

from support.running import *
from support.evaluating import *

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

class ProcedureEvaluator(TestEvaluator):
    def __init__(self):
        pass

    def setupProcessResults(self):
        pass

    def evaluate(self):
        if self.noErrorOccured():
            self.checkAllUnexpectedMessages()
            handler = self.test_process_results[PROCEDURE_HANDLER][0]
            if self.hasRegisteredSomething(handler):
                sender = self.test_process_results[PROCEDURE_CALLER][0]
                return self.isConsistent(sender, handler) and self.isConsistent(handler, sender)
        return False

#===================================================================================================
# Scripting
#===================================================================================================

if __name__ == "__main__":
    runner = TestRunner()
    evaluator = ProcedureEvaluator()

    runner.addTestProcess(PROCEDURE_HANDLER, pause=0.5)
    runner.addTestProcess(PROCEDURE_CALLER)

    runner.runTest(evaluator, CYCLE_DURATION, CYCLE_COUNT)
