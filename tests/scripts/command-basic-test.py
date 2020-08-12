from define.processes import *
from define.arguments import *

from support.running import *
from support.evaluating import *

#===================================================================================================
# Parameters
#===================================================================================================

# Public and adjustable
## Test
HANDLER_COUNT = getArgumentValue(HANDLER_COUNT_KEY, 2)
## Run
CYCLE_COUNT = getArgumentValue(CYCLE_COUNT_KEY, 1)

# Internal
CYCLE_DURATION = 6 # seconds

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
            handlers = self.test_process_results[COMMAND_HANDLER]
            if self.hasRegisteredSomething(handlers):
                sender = self.test_process_results[COMMAND_SANDER][0]
                return self.isConsistent(sender, handlers)
        return False

#===================================================================================================
# Scripting
#===================================================================================================

if __name__ == "__main__":
    runner = TestRunner()
    evaluator = CommandEvaluator()

    runner.addTestProcess(COMMAND_HANDLER, count=HANDLER_COUNT)
    runner.addTestProcess(COMMAND_SANDER)

    runner.runTest(evaluator, CYCLE_DURATION, CYCLE_COUNT)
