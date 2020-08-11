import sys
import xml.etree.ElementTree as ET
from datetime import datetime

from utils.define import *


SENT_TAG = "sent"
RECEIVED_TAG = "received"
REQUESTED_TAG = "requested"
REGISTRED_TAG = "registered"

WARNING = "WARNING:"
CRITICAL = "CRITICAL:"
FATAL = "FATAL:"

ERROR_LINE_STARTS = [WARNING, CRITICAL, FATAL]

START_MESSAGE_START = "The server is running on IP:"
CONNECTED_MESSAGE = "Successfuly connected to"
KILLED_MESSAGE = "Killed during cleanup.";

STANDARD_LINE_STARTS = [START_MESSAGE_START, CONNECTED_MESSAGE, KILLED_MESSAGE]


class TestEvaluationLogger:

    def __init__(self, name=sys.argv[0], install = True):
        if len(sys.argv) > 2:
            name = sys.argv[2]
        if name.endswith(SCRIPT_SUFFIX):
            name = name.replace(SCRIPT_SUFFIX, "")
        self.log_file_path = TEST_RESULTS_FOLDER + name + "/" + "evaluation" + LOG_SUFFIX

    def __getTimeString(self):
        now = datetime.now()
        return '"' + now.strftime("%Y-%m-%d %H:%M:%S") + '" '

    def log_message(self, message):
        with open(self.log_file_path, 'a') as file_object:
            file_object.write(self.__getTimeString() + message + "\n")

    def log_evaluation_start(self):
        with open(self.log_file_path, 'a') as file_object:
            file_object.write(LOG_SEPARATOR)
            file_object.write(self.__getTimeString() + "Evaluation started!\n")
            file_object.write(LOG_SEPARATOR)

    def log_and_print_message(self, message, tab=True):
        self.log_message(message)
        if tab:
            print("--> " + message)
        else:
            print(message)

    def log_and_print_list(self, messages, tab=True):
        for message in messages:
            self.log_and_print_message(message)


class ResultObject:

    def __init__(self, process_log_path, test_name=sys.argv[0]):
        self.test_name = test_name
        self.process_log_path = process_log_path

        self.standard_messages = list()
        self.unknown_messages = list()
        self.error_messages = list()

        self.api_message_counter = 0
        self.sent_counter = 0
        self.registered_counter = 0
        self.requested_counter = 0
        self.received_counter = 0

        self.sent_messages = dict()
        self.registered_messages = dict()
        self.requested_messages = dict()
        self.received_messages = dict()

        self.connected = False

    def place_dict_to_dict(self, parent_map, key):
        if key not in parent_map:
            parent_map[key] = dict()
        return parent_map

    def place_to_dict(self, map, key, value):
        if key in map:
            map[key].append(value)
        else:
            map[key] = [value]
        return map

    def readInternMessage(self, line):
        for error in ERROR_LINE_STARTS:
            if line.startswith(error):
                self.error_messages.insert(0, line)
                return
        for standard in STANDARD_LINE_STARTS:
            if line.startswith(standard):
                self.standard_messages.insert(0, line)
                if standard == CONNECTED_MESSAGE:
                    self.connected = True
                return
        self.unknown_messages.insert(0, line)

    def readApiMessage(self, line):
        self.api_message_counter+=1
        root = ET.fromstring(line)

        action = root[0].tag
        type = root[0][0].tag
        name = root[0][0].attrib["name"]
        message = root[0][0].text

        if action == SENT_TAG:
            self.sent_counter+=1
            self.place_dict_to_dict(self.sent_messages, type)
            self.place_to_dict(self.sent_messages[type], name, message)
        if action == REGISTRED_TAG:
            self.registered_counter+=1
            self.place_dict_to_dict(self.registered_messages, type)
            self.place_to_dict(self.registered_messages[type], name, message)
        if action == REQUESTED_TAG:
            self.requested_counter+=1
            self.place_dict_to_dict(self.requested_messages, type)
            self.place_to_dict(self.requested_messages[type], name, message)
        if action == RECEIVED_TAG:
            self.received_counter+=1
            self.place_dict_to_dict(self.received_messages, type)
            self.place_to_dict(self.received_messages[type], name, message)

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

    def readTestLog(self):
        reversed_lines = self.getReversedLogLines()
        for line in reversed_lines:
            if line.startswith("<" + API_TAG + ">"):
                self.readApiMessage(line)
            else:
                self.readInternMessage(line)

    def somethingStrange(self, logger=None):
        messages = list()
        result = False
        if not self.connected:
            messages.append("Process was not connected to Control Server.")
            result = True

        if self.error_messages:
            messages.append("Errors occured:")
            for message in self.error_messages:
                messages.append(message)
                result = True

        if self.unknown_messages:
            messages.append("Unexpected log found:")
            for message in self.unknown_messages:
                messages.append(message)
        logger.log_and_print_list(messages)
        return result


def hasHappendSomethingStrange(resultObjectDict, logger=None):
    error = False
    for name, type_results in resultObjectDict.items():
        for result in type_results:
            if result.somethingStrange(logger):
                message = "^^ Happened in " + name + " ^^"
                if logger is not None:
                    logger.log_and_print_message(message)
                else:
                    print(message)
                error = True
    return error


def controlServerError(logs, logger=None):
    error = False
    for line in logs:
        if "crashed (No HeartBeats have been received)." in line:
            messages = ["Error on control server:"]
            messages.append(line)
            if logger is not None:
                logger.log_and_print_list(messages)
            else:
                print(messages)
            error = True
    return error
