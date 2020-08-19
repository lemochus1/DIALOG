from define.processes import *
from define.arguments import *

from shared.running import *
from shared.evaluating import *

#===================================================================================================
# Parameters
#===================================================================================================

# Test
## Public
PROCESS_COUNT             = getArgumentValue(PROCESS_COUNT_KEY, 5)
PAUSE_BETWEEN_CONNECTS    = getArgumentValue(PAUSE_BETWEEN_CONNECTS_KEY, 100)

SETUP_STRING = "Process count: {}, Connects pause: {}ms".format(PROCESS_COUNT,
                                                                PAUSE_BETWEEN_CONNECTS)
## Internal
PROCESS_NAME   = DUMMY_PROCESS

# Run
CYCLE_COUNT    = getArgumentValue(CYCLE_COUNT_KEY, 5)
CYCLE_DURATION = getArgumentValue(CYCLE_DURATION_KEY, 4000)

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
    runner    = TestRunner(setup_string=SETUP_STRING)
    evaluator = ConnectEvaluator()

    runner.addTestProcess(PROCESS_NAME, pause=PAUSE_BETWEEN_CONNECTS, count=PROCESS_COUNT)
    runner.runTest(evaluator, CYCLE_DURATION, CYCLE_COUNT)
