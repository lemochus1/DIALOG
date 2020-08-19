from __future__ import with_statement

import os
import sys

import xml.etree.ElementTree as ET

from shared.utils.containers import *

from define.files import *
from define.logs import *
from define.xmlelements import *

#===================================================================================================
# Class TestProcessResult
#===================================================================================================

class TestProcessResult:

    def __init__(self, process_log_path, test_name=sys.argv[0]):
        self.process_log_path = process_log_path
        self.test_name        = test_name
        self.process_name     = os.path.basename(process_log_path).replace(LOG_SUFFIX, "")

        self.standard_messages_parts = STANDARD_MESSAGE_PARTS
        self.error_messages_parts    = ERROR_MESSAGE_PARTS
        self.ignored_messages_parts    = list()
        self.controlled_messages_parts = list()

        self.controlled_messages = list()

        self.api_sent_messages       = dict()
        self.api_registered_messages = dict()
        self.api_requested_messages  = dict()
        self.api_received_messages   = dict()

        self.internal_standard_messages = list()
        self.internal_ignored_messages  = list()
        self.internal_error_messages    = list()

        self.unknown_messages = list()

        self.api_message_counter = 0

        self.sent_counter       = 0
        self.registered_counter = 0
        self.requested_counter  = 0
        self.received_counter   = 0

        self.connected_control_server = False

    def addControlledMessage(self, messages_parts):
        addToList(self.controlled_messages_parts, messages_parts)

    def addStandardMessage(self, messages_parts):
        addToList(self.standard_messages_parts, messages_parts)

    def addIgnoreMessages(self, messages_parts):
        addToList(self.ignored_messages_parts, messages_parts)

    def addErrorMessages(self, messages_parts):
        addToList(self.error_messages_parts, messages_parts)

    def __readInternalMessage(self, line):
        if CONNECTED_MESSAGE in line:
            self.connected_control_server = True

        if insertIfBelongs(self.internal_ignored_messages, line, self.ignored_messages_parts):
            return
        if insertIfBelongs(self.internal_error_messages, line, self.error_messages_parts):
            return
        if insertIfBelongs(self.internal_standard_messages, line, self.standard_messages_parts):
            return

        self.unknown_messages.insert(0, line)

    def __readApiMessage(self, line):
        self.api_message_counter += 1

        root    = ET.fromstring(line)
        action  = root[0].tag
        type    = root[0][0].tag
        name    = root[0][0].attrib[NAME_ATTRIBUTE]
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
        try:
            with open(self.process_log_path, 'r') as file_object:
                for line in reversed(list(file_object)):
                    if line.startswith(TERMINAL_SEPARATOR):
                        return reversed_lines
                    line = line.strip()
                    line = line[21:].strip()# Removes date and time
                    reversed_lines.append(line)
        except:
            print("Log file could not be parsed.")
        return reversed_lines

    def readLog(self):
        reversed_lines = self.getReversedLogLines()
        for line in reversed_lines:
            insertIfBelongs(self.controlled_messages, line, self.controlled_messages_parts)
            if "<" + API_TAG + ">" in line:
                self.__readApiMessage(line)
            else:
                self.__readInternalMessage(line)

#===================================================================================================
# Class TestEvaluator
#===================================================================================================

class TestEvaluator:
    def __init__(self):
        self.test_process_results = None
        self.control_server_result = None
        self.logger = None

    def setLogger(self, logger):
        self.logger = logger

    def setProcessResults(self, test_process_resutls, control_server_result):
        self.test_process_results  = test_process_resutls
        self.control_server_result = control_server_result

    def setupProcessResults(self):
        pass

    def evaluate(self):
        raise NotImplementedError("Custom evaluator must implement its own 'evaluate()' method.")

    def readAllLogs(self):
        self.control_server_result.readLog()
        for process_list in self.test_process_results.values():
            for process in process_list:
                process.readLog()

    def checkProcessConnected(self, process):
        if not process.connected_control_server:
            self.logger.logAndPrint(NO_CONNECTION_FORMAT_MESSAGE.format(process.process_name), 1)
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
            messages = [ERROR_OCCURED_FORMAT_MESSAGE.format(process.process_name)]
            for message in process.internal_error_messages:
                messages.append(TAB + message)
            logger.logAndPrintList(messages, 1)
            return True
        return False

    def noErrorOccured(self):
        if not self.everyProcessConnected():
            return False
        if self.checkErrorOccurred(self.control_server_result):
            return False
        for process_list in self.test_process_results.values():
            for process in process_list:
                if self.checkErrorOccurred(process):
                    return False
        return True

    def checkUnexpectedOccurred(self, process):
        if process.unknown_messages:
            messages = [UNEXPECTED_LOG_FORMAT_MESSAGE.format(process.process_name)]
            for message in process.unknown_messages:
                messages.append(TAB + message)
            self.logger.logAndPrintList(messages, 1)
            return True
        return False

    def checkAllUnexpectedMessages(self):
        result = self.checkUnexpectedOccurred(self.control_server_result)
        for process_list in self.test_process_results.values():
            for process in process_list:
                result = (result and self.checkUnexpectedOccurred(process))
        return  result

    def hasRegisteredSomething(self, handlers):
        if not isinstance(handlers, list):
            handlers = [handlers]
        for handler in handlers:
            if handler.registered_counter < 1:
                self.logger.logAndPrint(NO_REGISTER_FORMAT_MESSAGE.format(handler.process_name), 1)
                return False
        return True

    def areConsistent(self, sender, receivers):
        if not isinstance(receivers, list):
            receivers = [receivers]

        for receiver in receivers:
            lost_messages_count = sender.sent_counter - receiver.received_counter
            if lost_messages_count:
                self.logger.logAndPrint(NO_CONSISTENT_FORMAT_MESSAGE.format(receiver.process_name,
                                                                            lost_messages_count), 1)
                return False
            if receiver.api_received_messages != sender.api_sent_messages:
                self.logger.logAndPrint(DEMAGED_MESSAGE_MESSAGE, 1)
                return False
        return True
