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
        for process_result in self.test_process_results[SERVICE_SUBSCRIBER]:
            process_result.addControlledMessage(LOST_SENDER)
            process_result.addIgnoredMessage(LOST_SENDER)
        self.control_server_result.addIgnoredMessage(INFO_ABOUT_SERVICE_MESSAGE)
        self.control_server_result.addIgnoredMessage(NO_HEART_BEAT)

    def evaluate(self):
        self.checkAllUnexpectedMessages()
        for process_result in self.test_process_results[SERVICE_SUBSCRIBER]:
            if not UNAVAILABLE_TAG in process_result.api_error_messages:
                self.logger.logAndPrint("Missing unavailable error message on {}."
                                        .format(process_result.process_name))
                return False
        return True

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
