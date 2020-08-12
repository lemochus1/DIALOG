from define.processes import *
from define.arguments import *

from support.running import *
from support.evaluating import *

#===================================================================================================
# Parameters
#===================================================================================================

# Public and adjustable
## Test
SLEEP_TIME = 0.1
## Run
CYCLE_COUNT = getArgumentValue(CYCLE_COUNT_KEY, 5)

# Internal
CYCLE_DURATION = 2 # seconds

PROCESS_NAMES = [SERVICE_PROVIDER, COMMAND_HANDLER, PROCEDURE_HANDLER]

#===================================================================================================
# Evaluation
#===================================================================================================

class RegisterEvaluator(TestEvaluator):
    def __init__(self):
        pass

    def setupProcessResults(self):
        self.control_server_result.addControlledMessage(
                                   "has been registered on CommunicationControlServer")

    def evaluate(self):
        if self.noErrorOccured():
            self.checkAllUnexpectedMessages()

            processes = list()
            for result in self.test_process_results.items():
                processes.extend(result)
            if self.hasRegisteredSomething(processes):
                registred_count = len(self.control_server_result.controlled_messages)
                if registred_count is not len(PROCESS_NAMES):
                    logger.logAndPrint("Some process did not register anything on Control Server.")
                    return False
                else:
                    return True
        return False

#===================================================================================================
# Scripting
#===================================================================================================

if __name__ == "__main__":
    runner = TestRunner()
    evaluator = RegisterEvaluator()

    for name in PROCESS_NAMES:
        runner.addTestProcess(name, pause=SLEEP_TIME)

    runner.runTest(evaluator, CYCLE_DURATION, CYCLE_COUNT)
