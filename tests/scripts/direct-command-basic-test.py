from define.processes import *
from define.arguments import *

from support.running import *
from support.evaluating import *

#===================================================================================================
# Parameters
#===================================================================================================

# Public and adjustable
## Test
TARGETED_HANDLER_COUNT = getArgumentValue(TARGETED_HANDLER_COUNT_KEY, 2)
## Run
CYCLE_COUNT = getArgumentValue(CYCLE_COUNT_KEY, 1)

# Internal
CYCLE_DURATION = 15 # seconds
TERGETED_COMMAND_HANDLER_NAME = "targeted-handler"

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
            handlers = self.test_process_results[TERGETED_COMMAND_HANDLER_NAME]
            if self.hasRegisteredSomething(handlers):
                sender = self.test_process_results[COMMAND_SANDER][0]
                return self.isConsistent(sender, handlers)
        return False

#===================================================================================================
# Scripting
#===================================================================================================

if __name__ == "__main__":
    runner = TestRunner()
    evaluator = DirectCommandEvaluator()

    runner.addTestProcess(COMMAND_HANDLER)
    runner.addTestProcess(COMMAND_HANDLER,
                          name=TERGETED_COMMAND_HANDLER_NAME,
                          count=TARGETED_HANDLER_COUNT)

    runner.runTest(evaluator, CYCLE_DURATION, CYCLE_COUNT)
