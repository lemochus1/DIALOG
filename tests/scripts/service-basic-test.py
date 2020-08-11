import time
import xml.etree.ElementTree as ET

from utils.runner import TestRunner
from utils.evaluation import *

SERVICE_SUBSCRIBER = "service-subscriber"
SERVICE_PROVIDER = "service-provider"
HANDLERS_COUNT = 2
TEST_DURATION = 10

NORMAL_LOG_START = 'Service  "Service"  has been subscribed by'

SERVICE_DATA = "service-data"

def evaluateTest(runner):
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


runner = TestRunner()

runner.setup()
time.sleep(0.2)
runner.startTestProcess(SERVICE_PROVIDER)
time.sleep(1)
runner.startTestProcess(SERVICE_SUBSCRIBER)
runner.waitWhileTesting(8)
evaluateTest(runner)
