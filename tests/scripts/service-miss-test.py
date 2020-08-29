from define.processes import *
from define.arguments import *
from define.logs import *

from shared.running import *
from shared.evaluating import *

#===================================================================================================
# Parameters
#===================================================================================================

# Test
SERVICE_NAME = "Service"

# Run
CYCLE_COUNT    = getArgumentValue(CYCLE_COUNT_KEY, 1)
CYCLE_DURATION = getArgumentValue(CYCLE_DURATION_KEY, 3000)

#===================================================================================================
# Evaluation
#===================================================================================================

class ServiceMissEvaluator(TestEvaluator):
    def __init__(self):
        pass

    def setupProcessResults(self):
        self.control_server_result.addIgnoredMessage(NO_SENDER)

    def evaluate(self):
        self.checkAllUnexpectedMessages()
        error_messages      = self.test_process_results[SERVICE_SUBSCRIBER][0].api_error_messages
        if not UNAVAILABLE_TAG in error_messages:
            self.logger.logAndPrint("Missing unavailable error message.")
            return False
        return True

#===================================================================================================
# Scripting
#===================================================================================================

if __name__ == "__main__":
    runner    = TestRunner()
    evaluator = ServiceMissEvaluator()

    runner.addTestProcess(SERVICE_SUBSCRIBER)

    runner.runTest(evaluator, CYCLE_DURATION, CYCLE_COUNT)
