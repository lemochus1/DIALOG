from define.processes import *
from define.arguments import *
from define.logs import *

from shared.running import *
from shared.evaluating import *

#===================================================================================================
# Parameters
#===================================================================================================

# Test
## Public
PAUSE_BETWEEN_CONNECTS    = getArgumentValue(PAUSE_BETWEEN_CONNECTS_KEY, 100)

SETUP_STRING = "Connects pause: {}ms".format(PAUSE_BETWEEN_CONNECTS)
## Internal
PROCESS_COUNT          = 1

MESSAGE_SIZE           = 0
PAUSE_BETWEEN_MESSAGES = 1000
MESSAGE_COUNT          = 0

CALL_DURATION = 1000

# Run
CYCLE_COUNT    = getArgumentValue(CYCLE_COUNT_KEY, 5)
CYCLE_DURATION = getArgumentValue(CYCLE_DURATION_KEY, 2000)

#===================================================================================================
# Tokens
#===================================================================================================

SERVICE_PROVIDER_TOKENS = {
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

class RegisterEvaluator(TestEvaluator):
    def __init__(self):
        pass

    def setupProcessResults(self):
        self.control_server_result.addControlledMessage(REGISTERED_MESSAGE)

    def evaluate(self):
        if self.noErrorOccured():
            self.checkAllUnexpectedMessages()

            processes = list()
            for result in self.test_process_results.values():
                processes.extend(result)
            if self.hasRegisteredSomething(processes):
                registred_count = len(self.control_server_result.controlled_messages)
                if registred_count is not 3 * PROCESS_COUNT:
                    self.logger.logAndPrint(
                                "Some process did not register anything on Control Server.")
                    return False
                else:
                    return True
        return False

#===================================================================================================
# Scripting
#===================================================================================================

if __name__ == "__main__":
    runner    = TestRunner(setup_string=SETUP_STRING)
    evaluator = RegisterEvaluator()

    runner.addTestProcess(SERVICE_PROVIDER,
                          tokens=SERVICE_PROVIDER_TOKENS,
                          count=PROCESS_COUNT,
                          pause=PAUSE_BETWEEN_CONNECTS)
    runner.addTestProcess(PROCEDURE_PROVIDER,
                          tokens=PROCEDURE_PROVIDER_TOKENS,
                          count=PROCESS_COUNT,
                          pause=PAUSE_BETWEEN_CONNECTS)
    runner.addTestProcess(COMMAND_HANDLER,
                          count=PROCESS_COUNT,
                          pause=PAUSE_BETWEEN_CONNECTS)

    runner.runTest(evaluator, CYCLE_DURATION, CYCLE_COUNT)
