import os
import sys
import time
import xml.etree.ElementTree as ET

from datetime import datetime

from utils.terminal import *
from utils.define import *
from utils.build import *
from utils.process import *
from utils.evaluation import ResultObject


class TestRunner:

    def __init__(self, name=sys.argv[0], install=True):

        if len(sys.argv) > 2:
            name = sys.argv[2]

        self.name = name
        self.install = install
        self.process_names = list()
        self.log_paths = list()
        self.control_server_log_path = None
        self.opened_terminals = 0

        if name.endswith(SCRIPT_SUFFIX):
            name = name.replace(SCRIPT_SUFFIX, "")
        self.result_folder = TEST_RESULTS_FOLDER + name + "/"

        print(TERMINAL_SEPARATOR)
        print("Test: " + self.name)

    def __logProcessStart(self, log_path):
        with open(log_path, 'a') as file_object:
            file_object.write(LOG_SEPARATOR)
            file_object.write(self.__getTimeString() + " Started in: " + self.name + "\n")
            file_object.write(LOG_SEPARATOR)

    def setup(self):
        if self.install:
            moveDialogToSharedDirectory()
        if not os.path.exists(self.result_folder):
            os.mkdir(self.result_folder)
        if not isProcessRunning(CONTROL_SERVER_NAME):
            self.control_server_log_path = self.result_folder + "control-server" + LOG_SUFFIX
            runProcessNewTerminal(CONTROL_SERVER_PATH, self.control_server_log_path, opened_terminals=self.opened_terminals)
            self.opened_terminals+=1
            self.__logProcessStart(self.control_server_log_path)
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

    def __logProcessKilled(self, log_path):
        with open(log_path, 'a') as file_object:
            file_object.write(self.__getTimeString() + " Killed during cleanup.\n")

    def startTestProcess(self, config_file, name = None, tokens = None, log_file = None):
        if config_file.endswith(CONFIG_SUFFIX):
            config_file = config_file.replace(CONFIG_SUFFIX, "")

        arguments = list()
        if name is None:
            name = config_file
        else:
            arguments.append(config_file + "|" + name)

        if log_file is None:
            log_file = name
            if log_file in self.process_names:
                log_file += "(" + str(self.process_names.count(name)) + ")"

        config_path = TEST_CONFIGS_FOLDER + config_file + CONFIG_SUFFIX
        log_path = self.result_folder + log_file + LOG_SUFFIX

        arguments.insert(0, config_path)
        arguments.insert(1, log_path)

        if isinstance(tokens, list):
            arguments.extend(tokens)
        elif isinstance(tokens, dict):
            arguments.append(self.__tokenMapToArguments(tokens))
        elif tokens is not None:
            arguments.append(tokens)

        self.__logProcessStart(log_path)

        runProcessNewTerminal(TEST_PROCESS_PATH, arguments, opened_terminals=self.opened_terminals)

        self.process_names.append(name)
        self.log_paths.append(log_path)
        self.opened_terminals += 1

    def clear(self):
        self.opened_terminals = 0
        self.process_names = list()
        self.log_paths = list()

    def cleanup(self, ask=True, close=True):
        if ask and (len(sys.argv) < 2 or str(sys.argv[1]) == 'pause'):
            input("Press enter to continue...")

        runningProcesses = [TEST_PROCESS_NAME, CONTROL_SERVER_NAME]
        for processName in runningProcesses:
            test_process_pids = getProcessPid(processName)
            for pid in test_process_pids:
                killProcessPid(pid)
        for log_path in self.log_paths:
            self.__logProcessKilled(log_path)
        if close:
            print(TERMINAL_SEPARATOR)

    def waitWhileTesting(self, duration=15):
        print("Started...")
        for remaining in range(duration, 0, -1):
            sys.stdout.write("\r")
            sys.stdout.write("Running: {:2d} seconds".format(remaining))
            sys.stdout.flush()
            time.sleep(1)
        sys.stdout.write("\r")
        print("Finished...                                      ")

    def getResults(self):
        results = dict()
        for log_path, name in zip(self.log_paths, self.process_names):
            result_object = ResultObject(log_path, self.name)
            result_object.readTestLog()
            if name in results:
                results[name].append(result_object)
            else:
                results[name] = [result_object]
        return results

    def getControlServerResult(self):
        result_object = ResultObject(self.control_server_log_path, self.name)
        result_object.readTestLog()
        return result_object
