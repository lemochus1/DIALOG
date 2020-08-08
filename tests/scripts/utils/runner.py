import os
import sys
import time
import xml.etree.ElementTree as ET

from datetime import datetime
from enum import Enum

from utils.terminal import *
from utils.define import *
from utils.build import *
from utils.process import *

CONFIG_SUFFIX = ".proc"
LOG_SUFFIX = ".log"
SCRIPT_SUFFIX = ".py"

TERMINAL_SEPARATOR = "-----------------------------------------"
LOG_SEPARATOR = TERMINAL_SEPARATOR + TERMINAL_SEPARATOR + "\n"
TOKEN_VALUE_SEPARATOR = "|"


API = "API: "


class MessageType(Enum):

    COMMAND = 1
    SERVICE = 2
    PROCEDURE = 3

    @staticmethod
    def getString(type):
        if type is MessageType.COMMAND:
            return "command"
        elif type is MessageType.SERVICE:
            return "service"
        elif type is MessageType.PROCEDURE:
            return "procedure"


class MessageObject:

    def __init__(self, type: MessageType, name, message=""):
        self.type = type
        self.name = name
        self.message = message


class ResultObject:

    def __init__(self, process_log_path, test_name=sys.argv[0]):
#        if test_name.endswith(SCRIPT_SUFFIX):
#            test_name = test.name.replace(SCRIPT_SUFFIX,"")
        #self.result_log_path = TEST_RESULTS_FOLDER + "/" + test_name "/results.log"
        self.test_name = test_name
        self.process_log_path = process_log_path

        self.standard_messages = list()
        self.unknown_messages = list()

        self.received_commands = list()
        self.received_procedure_calls = list()
        self.received_procedure_data = list()
        self.received_service_data = list()
        self.received_service_request = list()

        self.registered_commands = list()
        self.registered_services = list()
        self.registered_procedures = list()

        self.requested_services = list()

        self.send_commands = list()
        self.send_procedure_calls = list()
        self.send_procedure_data = list()
        self.send_service_data = list()
        self.send_service_request = list()

        self.api_message_counter = 0

    def readTestLog(self):
        with open(self.process_log_path, 'r') as file_object:
            for line in reversed(list(file_object)):
#                line = line.rstrip()

                if line.startswith(LOG_SEPARATOR):
                    return # zatim

                line = line.rstrip()
                line = line[21:] #removes date and time

                if API in line:
                    line = line.replace(API,"")
                    self.api_message_counter+=1
                    #root = ET.fromstring(country_data_as_string)

                print(line)


    def consistentCheck(self):
        pass


class TestRunner:

    def __init__(self, name=sys.argv[0], install=True):
        self.name = name
        self.install = install
        self.processes = list()
        self.log_paths = list()

        if name.endswith(SCRIPT_SUFFIX):
            name = name.replace(SCRIPT_SUFFIX, "")
        self.result_folder = TEST_RESULTS_FOLDER + name + "/"

        print(TERMINAL_SEPARATOR)
        print("Test: " + self.name)

    def setup(self):
        if self.install:
            moveDialogToSharedDirectory()

        if not isProcessRunning(CONTROL_SERVER_NAME):
            runProcessNewTerminal(CONTROL_SERVER_PATH)

        if not os.path.exists(self.result_folder):
            os.mkdir(result_folder)
        print("Setuped...")

    @staticmethod
    def __tokenMapToArguments(tokens):
        arguments = list()
        for key, value in tokens.items():
            argumets.append(key + TOKEN_VALUE_SEPARATOR +  value)
        return arguments

    def __getTimeString(self):
        now = datetime.now()
        return '"' + now.strftime("%Y-%m-%d %H:%M:%S") + '"'

    def __logProcessStart(self, log_path):
        with open(log_path, 'a') as file_object:
            file_object.write(LOG_SEPARATOR)
            file_object.write(self.__getTimeString() + " Started in: " + self.name + "\n")
            file_object.write(LOG_SEPARATOR)

    def __logProcessKilled(self, log_path):
        with open(log_path, 'a') as file_object:
            file_object.write(self.__getTimeString() + " Killed during cleanup.\n")

    def startTestProcess(self, config_file, log_file = None, tokens = None):
        if config_file.endswith(CONFIG_SUFFIX):
            config_file = config_file.replace(CONFIG_SUFFIX, "")

        if log_file is None:
            log_file = config_file
            if log_file in self.processes:
                log_file += "(" + str(self.processes.count(config_file)) + ")"
            self.processes.append(config_file)
        else:
            self.processes.append(log_file)

        config_path = TEST_CONFIGS_FOLDER + config_file + CONFIG_SUFFIX
        log_path = self.result_folder + log_file + LOG_SUFFIX
        self.log_paths.append(log_path)
        executable_path = TEST_PROCESS_PATH

        arguments = [config_path, log_path]

        if isinstance(tokens, list):
            arguments.extend(tokens)
        elif isinstance(tokens, dict):
            arguments.append(self.__tokenMapToArguments(tokens))
        elif tokens is not None:
            argummets.append(tokens)

        self.__logProcessStart(log_path)
        runProcessNewTerminal(executable_path, arguments)

    @staticmethod
    def waitWhileTesting(duration=15):
        print("Started...")
        for remaining in range(duration, 0, -1):
            sys.stdout.write("\r")
            sys.stdout.write("Running: {:2d} seconds".format(remaining))
            sys.stdout.flush()
            time.sleep(1)
        sys.stdout.write("\r")
        print("Completed!                                      ")
        print(TERMINAL_SEPARATOR)

    def cleanup(self):
        # Killed while cleanup log...
        runningProcesses = [TEST_PROCESS_NAME, CONTROL_SERVER_NAME]
        for processName in runningProcesses:
            test_process_pids = getProcessPid(processName)
            for pid in test_process_pids:
                killProcessPid(pid)
        for log_path in self.log_paths:
            self.__logProcessKilled(log_path)

    def getResults(self):
        results = list()
        for log_path in self.log_paths:
            result_object = ResultObject(log_path, self.name)
            result_object.readTestLog()
            results.append(result_object)
        return results

