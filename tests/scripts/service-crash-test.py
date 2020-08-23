from define.processes import *
from define.arguments import *

from shared.running import *
from shared.evaluating import *

import time

#===================================================================================================
# Parameters
#===================================================================================================

# Test
MESSAGE_SIZE           = 0
PAUSE_BETWEEN_MESSAGES = 500
MESSAGE_COUNT          = 20

SUBSCRIBER_COUNT = 3

# Run
SAFE_START_PAUSE    = 500
PRE_KILL_DURATION   = 5000
DEATH_DURATION      = 2000
AFTER_KILL_DURATION = 5000

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

class ServiceCrashedEvaluator(TestEvaluator):
    def __init__(self):
        pass

    def setupProcessResults(self):
        for process_result_list in self.test_process_results.values():
            for process_result in process_result_list:
                process_result.addControlledMessage(CONNECT_REFUSED)
                process_result.addIgnoredMessage(CONNECT_REFUSED)
        self.control_server_result.addIgnoredMessage(INFO_ABOUT_SERVICE_MESSAGE)
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
    runner.setup()

    runner.startTestProcess(SERVICE_PROVIDER, tokens=SENDER_TOKENS, pause=SAFE_START_PAUSE)

    runner.startTestProcess(SERVICE_SUBSCRIBER, count=SUBSCRIBER_COUNT)

    runner.waitWhileTesting(PRE_KILL_DURATION)
    runner.killTestProcess(SERVICE_PROVIDER)
    print("Service provider killed!")
    runner.waitWhileTesting(DEATH_DURATION)
    runner.startTestProcess(SERVICE_PROVIDER, tokens=SENDER_TOKENS, pause=SAFE_START_PAUSE)
    print("Service provider started!")
    runner.waitWhileTesting(AFTER_KILL_DURATION)

    evaluator = ServiceCrashedEvaluator()
    evaluator.setLogger(runner.logger)
    runner.evaluate(evaluator)
