from define.processes import *
from define.arguments import *

from support.running import *
from support.evaluating import *

#===================================================================================================
# Parameters
#===================================================================================================

# Public and adjustable
## Test
PROCESS_COUNT = getArgumentValue(PROCESS_COUNT_KEY, 2)
SLEEP_TIME = 0
## Run
CYCLE_COUNT = getArgumentValue(CYCLE_COUNT_KEY, 5)

# Internal
CYCLE_DURATION = 2 # seconds

PROCESS_NAME = "dummy"

#===================================================================================================
# Evaluation
#===================================================================================================

class ConnectEvaluator(TestEvaluator):
    def __init__(self):
        pass

    def setupProcessResults(self):
        pass

    def evaluate(self):
        if self.noErrorOccured():
            self.checkAllUnexpectedMessages()
            return True
        return False

#===================================================================================================
# Scripting
#===================================================================================================

if __name__ == "__main__":
    runner = TestRunner()
    evaluater = ConnectEvaluator()

    runner.addTestProcess(PROCESS_NAME, pause=SLEEP_TIME, count=PROCESS_COUNT)
    runner.runTest(evaluator, CYCLE_DURATION, CYCLE_COUNT)