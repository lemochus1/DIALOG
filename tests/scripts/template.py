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
CYCLE_DURATION = 5 # seconds

#===================================================================================================
# Evaluation
#===================================================================================================

class TemplateEvaluator(TestEvaluator):
    def __init__(self):
        pass

    def setupProcessResults(self):
        pass

    def evaluate(self):
        pass

#===================================================================================================
# Scripting
#===================================================================================================

if __name__ == "__main__":
    runner = TestRunner()
    evaluator = TemplateEvaluator()

    runner.addTestProcess()

    runner.runTest(evaluator, CYCLE_DURATION, CYCLE_COUNT)
