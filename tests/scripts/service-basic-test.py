from define.processes import *
from define.arguments import *

from support.running import *
from support.evaluating import *

import xml.etree.ElementTree as ET

#===================================================================================================
# Parameters
#===================================================================================================

# Public and adjustable
## Test
HANDLER_COUNT = getArgumentValue(HANDLER_COUNT_KEY, 2)
TEST_DURATION = 10
## Run
CYCLE_COUNT = getArgumentValue(CYCLE_COUNT_KEY, 1)

# Internal
CYCLE_DURATION = 6

NORMAL_LOG_START = 'Service  "Service"  has been subscribed by'
SERVICE_DATA = "service-data"

#===================================================================================================
# Evaluation
#===================================================================================================

def evaluate(runner):
    print("Evaluated...")
    passed = True

    logger = TestEvaluationLogger()
    logger.log_evaluation_start()

    all_results = runner.getResults()

    subscriber = all_results[SERVICE_SUBSCRIBER][0]
    provider = all_results[SERVICE_PROVIDER][0]

    if subscriber.somethingStrange(logger):
        passed = False

    if provider.registered_counter is not 1:
        logger.log_and_print_message("Service was not registered.")
        passed = False

    if subscriber.requested_counter is not 1:
        logger.log_and_print_message("Service was not requested.")
        passed = False

    if not provider.connected:
        logger.log_and_print_message("Provider was not connected.")
        passed = False

    subscribed_counter = 0
    for message in provider.unknown_messages:
        if not message.startswith(NORMAL_LOG_START):
            logger.log_and_print_message("Unknown message logged by provider:")
            logger.log_and_print_message(message)
        else:
            subscribed_counter += 1

    if subscribed_counter is not 1:
        logger.log_and_print_message("Service was not subscribed by all subscribers.")
        passed = False

    if not SERVICE_DATA in provider.sent_messages:
        logger.log_and_print_message("No data send by provider.")

    if not SERVICE_DATA in subscriber.received_messages:
        logger.log_and_print_message("No data received by subscriber.")

    sent_messages = provider.sent_messages[SERVICE_DATA]["Service"]
    received_messages = subscriber.received_messages[SERVICE_DATA]["Service"]

    for index, sent in enumerate(sent_messages):
        if index < len(received_messages):
            if not sent == received_messages[index]:
                logger.log_and_print_message("Subscribed data was not delivered right.")
                passed = False

    if passed:
        logger.log_and_print_message("Passed!", False)
    else:
        logger.log_and_print_message("Failed!", False)
    runner.cleanup(not passed)

#===================================================================================================
# Scripting
#===================================================================================================

if __name__ == "__main__":
    runner = TestRunner()

    runner.addTestProcess(SERVICE_PROVIDER, pause=0.5)
    runner.addTestProcess(SERVICE_SUBSCRIBER)

    runner.runTest(evaluate, CYCLE_DURATION, CYCLE_COUNT)
