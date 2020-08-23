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

CALL_DURATION          = 0

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

PROCEDURE_PROVIDER_TOKENS = {
                             DURATION_TOKEN: CALL_DURATION
                            }

#===================================================================================================
# Evaluation
#===================================================================================================

class ControlCrashedEvaluator(TestEvaluator):
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

    runner.startTestProcess(COMMAND_HANDLER)
    runner.startTestProcess(PROCEDURE_PROVIDER, tokens=PROCEDURE_PROVIDER_TOKENS)
    runner.startTestProcess(SERVICE_PROVIDER, tokens=SENDER_TOKENS, pause=SAFE_START_PAUSE)

    runner.startTestProcess(COMMAND_SANDER, tokens=SENDER_TOKENS)
    runner.startTestProcess(PROCEDURE_CALLER, tokens=SENDER_TOKENS)
    runner.startTestProcess(SERVICE_SUBSCRIBER)

    runner.waitWhileTesting(PRE_KILL_DURATION)
    runner.killControlServer()
    print("Control server killed!")
    runner.waitWhileTesting(DEATH_DURATION)
    runner.startControlServer()
    print("Control server started!")
    runner.waitWhileTesting(AFTER_KILL_DURATION)

    evaluator = ControlCrashedEvaluator()
    evaluator.setLogger(runner.logger)
    runner.evaluate(evaluator)
