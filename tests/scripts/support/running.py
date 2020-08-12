import os
import sys
import time

from support.utils.terminal import *
from support.utils.define import *
from support.utils.build import *
from support.utils.process import *

from support.evaluating import *
from support.logging import *

from define.arguments import *
from define.processes import *

#===================================================================================================
# Help functions
#===================================================================================================

def getArgumentValue(arg_key, default_value, type=int):
    for argument in sys.argv:
        if "=" in argument:
            key_value = argument.split("=")
            if key_value[0] == arg_key:
                return type(key_value[1])
    return type(default_value)

#===================================================================================================
# Class TestRunner
#===================================================================================================

class TestRunner:

    def __init__(self, name=sys.argv[0], install=True):
        self.name = getArgumentValue(TEST_NAME_KEY, name, str)

        self.logger = TestLogger(self.name)
        self.install = install

        self.clear()

        self.cycle_index = 0
        self.cycle_count = 0

        self.result_folder = os.path.join(TEST_RESULTS_FOLDER, self.name.replace(SCRIPT_SUFFIX, ""))
        self.control_server_log_path = os.path.join(self.result_folder, CONTROL_SERVER + LOG_SUFFIX)

    def __getProcessStartedMessage(self):
        if self.cycle_count > 1:
            return "Started in run number: {} of ".format(self.cycle_index) + self.name
        else:
            return "Started in" + self.name

    def clear(self):
        self.opened_terminals = 0

        self.test_process_names = list()
        self.test_process_after_start_pause = list()
        self.test_process_log_paths = list()
        self.test_process_arguments = list()

    def install_dialog(self):
        if self.install:
            moveDialogToSharedDirectory()

    def startControlServer(self):
        if not isProcessRunning(CONTROL_SERVER_NAME):
            runProcessNewTerminal(CONTROL_SERVER_PATH, self.control_server_log_path,
                                  self.opened_terminals)
            self.logger.logSeparatedMessage(self.__getProcessStartedMessage(),
                                            self.control_server_log_path)
        self.opened_terminals+=1

    def setup(self, pause=0.2):
        self.install_dialog()

        if not os.path.exists(self.result_folder):
            os.mkdir(self.result_folder)

        self.opened_terminals = 0
        self.startControlServer()
        time.sleep(pause)

    @staticmethod
    def __tokenMapToArguments(tokens):
        arguments = list()
        for key, value in tokens.items():
            argumets.append(key + TOKEN_VALUE_SEPARATOR +  value)
        return arguments

    def tokensToArgumentList(self, tokens):
        if isinstance(tokens, list):
            return tokens
        elif isinstance(tokens, dict):
            return self.__tokenMapToArguments(tokens)
        elif tokens is not None:
            return [tokens]
        return list()

    def getTestProcessArguments(self, config_path, log_path, tokens=None, name_token=None):
        arguments = [config_path, log_path]
        if name_token is not None:
            arguments.append(name_token)
        if tokens is not None:
            arguments.extend(self.tokensToArgumentList(tokens))
        return arguments

    def addTestProcess(self, config_file, name = None, tokens = None,
                       log_file = None, count = 1, pause=0.1):
        for i in range(count):
            if config_file.endswith(CONFIG_SUFFIX):
                config_file = config_file.replace(CONFIG_SUFFIX, "")
            if name is None:
                name = config_file
            if log_file is None:
                log_file = name
                if log_file in self.test_process_names:
                    log_file += "({})".format(self.process_names.count(log_file))
            if not log_file.endswith(LOG_SUFFIX):
                log_file += LOG_SUFFIX

            config_path = os.path.join(TEST_CONFIGS_FOLDER, config_file + CONFIG_SUFFIX)
            log_path = os.path.join(self.result_folder, log_file)

            name_token = None
            if config_file != name:
                name_token = config_file + "|" + name
            arguments = self.getTestProcessArguments(config_path, log_path, tokens, name_token)

            self.test_process_names.append(name)
            self.test_process_after_start_pause.append(pause)
            self.test_process_log_paths.append(log_path)
            self.test_process_arguments.append(arguments)

    def __runTestProcess(self, index):
        self.logger.logSeparatedMessage(self.__getProcessStartedMessage(),
                                        self.test_process_log_paths[index])

        runProcessNewTerminal(TEST_PROCESS_PATH, self.test_process_arguments[index],
                              self.opened_terminals)
        self.opened_terminals += 1

        time.sleep(self.test_process_after_start_pause[index])

    def startTestProcess(self, config_file, name = None, tokens = None,
                         log_file = None, count = 1, pause=0.1):
        self.addTestProcess(config_file, name, tokens, log_file, count, pause)

        process_count = len(self.test_process_names)
        first_new_process_index = process_count - count
        for index in range(first_new_process_index, process_count):
            self.__runTestProcess(index)

    def killAll(self, ask=True):
        if ask:
            input("Press enter to continue...")

        message = "Killed during cleanup."

        killProcessName(CONTROL_SERVER_NAME)
        self.logger.logMessage(message, self.control_server_log_path)

        killProcessName(TEST_PROCESS_NAME)
        for log_path in self.test_process_log_paths:
            self.logger.logMessage(message, log_path)

    def waitWhileTesting(self, duration=15):
        for remaining in range(duration, 0, -1):
            sys.stdout.write("\r")
            sys.stdout.write("Running: {:2d} seconds".format(remaining))
            sys.stdout.flush()
            time.sleep(1)
        sys.stdout.write("\r")
        print("Finished...                                      ")

    def getTestProcessResults(self):
        results = dict()
        for log_path, name in zip(self.test_process_log_paths, self.test_process_names):
            result_object = TestProcessResult(log_path, self.name)
            result_object.readLog()
            if name in results:
                results[name].append(result_object)
            else:
                results[name] = [result_object]
        return results

    def getControlServerResult(self):
        result_object = TestProcessResult(self.control_server_log_path, self.name)
        result_object.readLog()
        return result_object

    def runTest(self, evaluator = None, cycle_duration=10, cycle_count=1):
        self.cycle_count = cycle_count

        print(TERMINAL_SEPARATOR)
        print("Test: " + self.name)

        for run_index in range(self.cycle_count):
            self.cycle_index = run_index
            self.setup()
            for process_index in range(len(self.test_process_names)):
                self.__runTestProcess(process_index)
            self.waitWhileTesting(cycle_duration)
            if evaluator is not None:
                print("Evaluation:")
                self.logger.logEvaluationStart()
                evaluator.setLogger(self.logger)
                evaluator.setProcessResults(self.getTestProcessResults(),
                                            self.getControlServerResult())
                evaluator.setupProcessResults()
                passed = evaluator.evaluate()
                if passed:
                    self.logger.logAndPrint("Passed!")
                else:
                    self.logger.logAndPrint("Failed!")
                self.killAll(not passed)
            else:
                 print("No evaluation!")
                 self.killAll()
        print(TERMINAL_SEPARATOR)
