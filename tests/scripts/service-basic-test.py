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
SUBSCRIBER_COUNT = getArgumentValue(SUBSCRIBER_COUNT_KEY, 3)

SETUP_STRING = "Subscriber count: {}".format(SUBSCRIBER_COUNT)
## Internal
PAUSE_BETWEEN_CONNECTS = 1000

MESSAGE_SIZE           = 0
PAUSE_BETWEEN_MESSAGES = 500
MESSAGE_COUNT          = 20

SERVICE_NAME = "Service"

# Run
CYCLE_COUNT    = getArgumentValue(CYCLE_COUNT_KEY, 1)
CYCLE_DURATION = getArgumentValue(CYCLE_DURATION_KEY, MESSAGE_COUNT * PAUSE_BETWEEN_MESSAGES + 2000)

#===================================================================================================
# Tokens
#===================================================================================================

PROVIDER_TOKENS = {
                   SIZE_TOKEN:     MESSAGE_SIZE,
                   DURATION_TOKEN: PAUSE_BETWEEN_MESSAGES,
                   REPEAT_TOKEN:   MESSAGE_COUNT,
                  }

#===================================================================================================
# Evaluation
#===================================================================================================

class ServiceEvaluator(TestEvaluator):
    def __init__(self):
        pass

    def setupProcessResults(self):
        self.control_server_result.addStandardMessage(INFO_ABOUT_SERVICE_MESSAGE)
        self.control_server_result.addStandardMessage(SUBSCRIBED_MESSAGE)

    def evaluate(self):
        if self.noErrorOccured():
            self.checkAllUnexpectedMessages()
            provider = self.test_process_results[SERVICE_PROVIDER][0]
            if self.hasRegisteredSomething(provider):
                subscribers = self.test_process_results[SERVICE_SUBSCRIBER]
                if self.wasServiceDataDelivered(provider, subscribers):
                    return True
        return False

    def wasServiceDataDelivered(self, provider, subscribers):
        if not isinstance(subscribers, list):
            subscribers = [subscribers]
        sent_messages = provider.api_sent_messages[SERVICE_DATA_TAG][SERVICE_NAME]

        for subscriber in subscribers:
            if subscriber.requested_counter is not 1:
                self.logger.logAndPrint("Service was not subscribed by all subscribers.")
                return False
            if SERVICE_DATA_TAG not in subscriber.api_received_messages:
                self.logger.logAndPrint("No service data was received by a subscriber.")
                return False
            received_messages = subscriber.api_received_messages[SERVICE_DATA_TAG][SERVICE_NAME]
            for index, sent in enumerate(sent_messages):
                if index < len(received_messages):
                    if not sent == received_messages[index]:
                        self.logger.logAndPrint("Subscribed data was not delivered right.")
                        return False
        return True

#===================================================================================================
# Scripting
#===================================================================================================

if __name__ == "__main__":
    runner    = TestRunner(setup_string=SETUP_STRING)
    evaluator = ServiceEvaluator()

    runner.addTestProcess(SERVICE_PROVIDER, tokens=PROVIDER_TOKENS, pause=PAUSE_BETWEEN_CONNECTS)
    runner.addTestProcess(SERVICE_SUBSCRIBER)

    runner.runTest(evaluator, CYCLE_DURATION, CYCLE_COUNT)
