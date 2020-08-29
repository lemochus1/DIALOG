from define.processes import *
from define.arguments import *

from shared.running import *
from shared.evaluating import *

#===================================================================================================
# Parameters
#===================================================================================================

# Test
MESSAGE_SIZE           = 0
PAUSE_BETWEEN_MESSAGES = 1000
MESSAGE_COUNT          = 20

PROCESS_NAMES = [SERVICE_PROVIDER, SERVICE_SUBSCRIBER, COMMAND_SANDER, PROCEDURE_CALLER]

# Run
CYCLE_COUNT    = getArgumentValue(CYCLE_COUNT_KEY, 1)
CYCLE_DURATION = getArgumentValue(CYCLE_DURATION_KEY, 5000)

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
        for process_name in PROCESS_NAMES:
            result_object = self.test_process_results[process_name][0]
            result_object.addIgnoredMessage(CONNECT_REFUSED)
            result_object.addIgnoredMessage(CONNECT_FAILED)
            result_object.addIgnoredMessage(CONNECT_REFUSED_DEFAULT_HANDLER)

    def evaluate(self):
        for process_name in PROCESS_NAMES:
            result_object = self.test_process_results[process_name][0]
            if result_object.connected_control_server:
                self.logger.logAndPrint("Process {} signalizes it was connected "
                                        "but no Control Server is running"
                                        .format(result_object.process_name))
                return False
            self.checkAllUnexpectedMessages()
        return True

#===================================================================================================
# Scripting
#===================================================================================================

if __name__ == "__main__":
    runner    = TestRunner()
    evaluator = ConnectMissEvaluator()

    for process_name in PROCESS_NAMES:
        runner.addTestProcess(process_name, tokens=SENDER_TOKENS)
    runner.runTest(evaluator, CYCLE_DURATION, CYCLE_COUNT, start_control_server=False)
