import os
import sys

import xml.etree.ElementTree as ET

from support.utils.define import *
from support.utils.containers import *

SENT_TAG = "sent"
RECEIVED_TAG = "received"
REQUESTED_TAG = "requested"
REGISTRED_TAG = "registered"

WARNING = "WARNING:"
CRITICAL = "CRITICAL:"
FATAL = "FATAL:"

NO_HEART_BEAT = "crashed (No HeartBeats have been received)."

ERROR_LINE_STARTS = [WARNING, CRITICAL, FATAL, NO_HEART_BEAT]

START_MESSAGE_START = "The server is running on IP:"
CONNECTED_MESSAGE = "Successfuly connected to"
KILLED_MESSAGE = "Killed during cleanup.";

STANDARD_MESSAGE_PARTS = [START_MESSAGE_START, CONNECTED_MESSAGE, KILLED_MESSAGE]
ERROR_MESSAGE_PARTS = [WARNING, CRITICAL, FATAL]

#===================================================================================================
# Class TestProcessResult
#===================================================================================================

class TestProcessResult:

    def __init__(self, process_log_path, test_name=sys.argv[0]):
        self.process_log_path = process_log_path
        self.test_name = test_name
        self.process_name = os.path.basename(process_log_path).replace(LOG_SUFFIX, "")

        self.standard_messages_parts = STANDARD_MESSAGE_PARTS
        self.error_messages_parts = ERROR_MESSAGE_PARTS
        self.ignored_messages_parts = list()
        self.controlled_messages_parts = list()

        self.controlled_messages = list()

        self.api_sent_messages = dict()
        self.api_registered_messages = dict()
        self.api_requested_messages = dict()
        self.api_received_messages = dict()

        self.internal_standard_messages = list()
        self.internal_ignored_messages = list()
        self.internal_error_messages = list()

        self.unknown_messages = list()

        self.api_message_counter = 0

        self.sent_counter = 0
        self.registered_counter = 0
        self.requested_counter = 0
        self.received_counter = 0

        self.connected_conrol_server = False

    def addControlledMessage(self, messages_parts):
        addToList(self.controlled_messages_parts, messages_parts)

    def addStandardMessage(self, messages_parts):
        addToList(self.standard_messages_parts, messages_parts)

    def addIgnoreMessages(self, messages_parts):
        addToList(self.ignored_messages_parts, messages_parts)

    def addErrorMessages(self, messages_parts):
        addToList(self.error_messages_parts, messages_parts)

    def __readinternalMessage(self, line):
        if insertIfBelongs(self.internal_ignored_messages, line, self.ignored_messages_parts):
            return
        if insertIfBelongs(self.internal_error_messages, line, self.error_messages_parts):
            return
        if insertIfBelongs(self.internal_standard_messages, line, self.standard_messages_parts):
            return

        if CONNECTED_MESSAGE in line:
            self.connected_control_server = True

        self.unknown_messages.insert(0, line)

    def __readApiMessage(self, line):
        self.api_message_counter += 1

        root = ET.fromstring(line)
        action = root[0].tag
        type = root[0][0].tag
        name = root[0][0].attrib["name"]
        message = root[0][0].text

        if action == SENT_TAG:
            self.sent_counter += 1
            placeDictToDict(self.api_sent_messages, type)
            placeToDictOfLists(self.api_sent_messages[type], name, message)
        elif action == REGISTRED_TAG:
            self.registered_counter += 1
            placeDictToDict(self.api_registered_messages, type)
            placeToDictOfLists(self.api_registered_messages[type], name, message)
        elif action == REQUESTED_TAG:
            self.requested_counter += 1
            placeDictToDict(self.api_requested_messages, type)
            placeToDictOfLists(self.api_requested_messages[type], name, message)
        elif action == RECEIVED_TAG:
            self.received_counter += 1
            placeDictToDict(self.api_received_messages, type)
            placeToDictOfLists(self.api_received_messages[type], name, message)
        else:
            self.unknown_messages.insert(0, line)
            self.api_message_counter -= 1

    def getReversedLogLines(self):
        reversed_lines = list()
        with open(self.process_log_path, 'r') as file_object:
            for line in reversed(list(file_object)):
                if line.startswith(LOG_SEPARATOR):
                    return reversed_lines
                line = line.strip()
                line = line[21:].strip()# Removes date and time
                reversed_lines.append(line)
        return reversed_lines

    def readLog(self):
        reversed_lines = self.getReversedLogLines()
        for line in reversed_lines:
            insertIfBelongs(self.controlled_messages, line, self.controlled_messages_parts)
            if "<" + API_TAG + ">" in line:
                self.__readApiMessage(line)
            else:
                self.__readinternalMessage(line)

#===================================================================================================
# Class TestEvaluator
#===================================================================================================

class TestEvaluator:
    def __init__(self):
        self.test_process_results = None
        self.control_server_result = None
        self.logger = None

        self.standard_message_starts = STANDARD_LINE_STARTS

    def setLogger(self, logger):
        self.logger = logger

    def setProcessResults(self, test_process_resutls, control_server_result):
        self.test_process_results = test_process_resutls
        self.control_server_result = control_server_result

    def setupProcessResults(self):
        pass

    def evaluate(self):
        raise NotImplementedError("Custom evaluator must implement its own 'evaluate()' method.")

    def checkProcessConnected(self, process):
        if not process.connected_control_server:
            logger.logAndPrint("Process " + process.process_name +
                               " was not connected to Control Server.")
            return False
        return True

    def everyProcessConnected(self):
        for process_list in self.test_process_results.values():
            for process in process_list:
                if not self.checkProcessConnected(process):
                    return False
        return True

    def checkErrorOccurred(self, process):
        if process.internal_error_messages:
            messages = ["Error occurred in {}:".format(process.process_name)]
            for message in process.internal_error_messages:
                messages.append(TAB + message)
            logger.logAndPrintList(messages)
            return True
        return False

    def noErrorOccured(self):
        if self.checkErrorOccurred(self.control_server_result):
            return False
        for process_list in self.test_process_results.values():
            for process in process_list:
                if self.checkErrorOccurred(process):
                    return False
        return True

    def checkUnexpectedOccurred(self, process):
        if process.internal_error_messages:
            messages = ["Unexpected log found in {}:".format(process.process_name)]
            for message in process._internal_error_messages:
                messages.append(TAB + message)
            self.logger.logAndPrintList(messages)
            return True
        return False

    def checkAllUnexpectedMessages(self):
        for process_list in self.test_process_results.values():
            for process in process_list:
                if self.checkUnexpectedOccurred(process):
                    return False
        return True

    def hasRegisteredSomething(self, handlers):
        if not isinstance(handlers, list):
            handlers = [handlers]
        for handler in handlers:
            if handler.registered_counter < 1:
                self.logger.logAndPrint("Handler {} did not register anything.".format(handler.process_name))
                return False
        return True


    def isConsistent(self, sender, receivers):
        if not isinstance(receivers, list):
            receivers = [receivers]

        for receiver in receivers:
            lost_messages_count = sender.sent_counter -receiver.received_counter
            if lost_messages_count:
                self.logger.logAndPrint(
                            "{} commands were send but not received.".format(lost_messages_count))
                return False
            if handler.received_messages != sender_result.sent_messages:
                self.logger.logAndPrint("Some message was delivered demaged.")
                return False
        return True
