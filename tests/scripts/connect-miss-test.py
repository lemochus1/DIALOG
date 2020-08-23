from define.processes import *
from define.arguments import *

from shared.running import *
from shared.evaluating import *

#===================================================================================================
# Parameters
#===================================================================================================

# Test
MESSAGE_SIZE           = 0
PAUSE_BETWEEN_MESSAGES = 500
MESSAGE_COUNT          = 20

PROCESS_NAMES = [SERVICE_PROVIDER, SERVICE_SUBSCRIBER, COMMAND_SANDER, PROCEDURE_CALLER]

# Run
CYCLE_COUNT    = getArgumentValue(CYCLE_COUNT_KEY, 1)
CYCLE_DURATION = getArgumentValue(CYCLE_DURATION_KEY, 4000)

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

class ConnectMissEvaluator(TestEvaluator):
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
    runner    = TestRunner()
    evaluator = ConnectMissEvaluator()

    for process_name in PROCESS_NAMES:
        runner.addTestProcess(process_name, tokens=SENDER_TOKENS)
    runner.runTest(evaluator, CYCLE_DURATION, CYCLE_COUNT, start_control_server=False)
