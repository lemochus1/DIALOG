from define.processes import *
from define.arguments import *

from shared.running import *
from shared.evaluating import *

#===================================================================================================
# Parameters
#===================================================================================================

# Test
## Public

## Internal
MESSAGE_SIZE           = 0
PAUSE_BETWEEN_MESSAGES = 0.5
MESSAGE_COUNT          = 20

# Run
CYCLE_COUNT    = getArgumentValue(CYCLE_COUNT_KEY, 1)
CYCLE_DURATION = getArgumentValue(CYCLE_DURATION_KEY, MESSAGE_COUNT * PAUSE_BETWEEN_MESSAGES + 2)

#===================================================================================================
# Tokens
#===================================================================================================

SENDER_TOKENS = {
                 SIZE_TOKEN:     MESSAGE_SIZE,
                 DURATION_TOKEN: PAUSE_BETWEEN_MESSAGES,
                 REPEAT_TOKEN:   MESSAGE_COUNT,
                }

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
    runner    = TestRunner()
    evaluator = TemplateEvaluator()

    runner.addTestProcess()

    runner.runTest(evaluator, CYCLE_DURATION, CYCLE_COUNT)
