import os
import sys
import time

from shared.utils.terminal import *
from shared.utils.build import *

from shared.evaluating import *
from shared.logging import *

from define.arguments import *
from define.processes import *
from define.logs import *

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

    def __init__(self, name=sys.argv[0], setup_string = "", install=True):
        self.name = getArgumentValue(TEST_NAME_KEY, name, str)

        self.logger       = TestLogger(self.name)
        self.setup_string = setup_string
        self.install      = install

        self.clear()

        self.cycle_index = 0
        self.cycle_count = 0

        self.result_folder = os.path.join(TEST_RESULTS_FOLDER, self.name.replace(SCRIPT_SUFFIX, ""))
        if not os.path.exists(self.result_folder):
            os.mkdir(self.result_folder)

        self.control_server_log_path = os.path.join(self.result_folder, CONTROL_SERVER + LOG_SUFFIX)
        self.kill_control_server = True

    def __getProcessStartedMessage(self):
        if self.cycle_count > 1:
            return "Started in run number: {} of ".format(self.cycle_index) + self.name
        else:
            return "Started in " + self.name

    def clear(self):
        self.opened_terminals = 0

        self.test_process_names             = list()
        self.test_process_after_start_pause = list()
        self.test_process_log_paths         = list()
        self.test_process_arguments         = list()

    def install_dialog(self):
        if self.install:
            moveDialogToSharedDirectory()

    def startControlServer(self):
        self.kill_control_server = not isProcessRunning(CONTROL_SERVER_NAME)
        if self.kill_control_server:
            runProcessNewTerminal(CONTROL_SERVER_PATH, self.control_server_log_path,
                                  self.opened_terminals)
            self.logger.logSeparatedMessage(self.__getProcessStartedMessage(),
                                            self.control_server_log_path)

        self.opened_terminals+=1

    def setup(self, pause=0.5):
        self.install_dialog()

        self.opened_terminals = 0
        self.startControlServer()
        time.sleep(pause)

    @staticmethod
    def __tokenMapToArguments(tokens):
        arguments = list()
        for key, value in tokens.items():
            arguments.append("{}{}{}".format(key, TOKEN_VALUE_SEPARATOR, value))
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

    def addTestProcess(self, config_file, name=None, tokens=None,
                       log_file=None, count=1, pause=0.1):
        if config_file.endswith(CONFIG_SUFFIX):
            config_file = config_file.replace(CONFIG_SUFFIX, "")
        if name is None:
            name = config_file

        for i in range(count):
            if log_file is None:
                log_filename = name
                if name in self.test_process_names:
                    log_filename += "({})".format(self.test_process_names.count(name))
            else:
                log_filename = log_file

            if not log_filename.endswith(LOG_SUFFIX):
                log_filename += LOG_SUFFIX

            config_path = os.path.join(TEST_CONFIGS_FOLDER, config_file + CONFIG_SUFFIX)
            log_path    = os.path.join(self.result_folder, log_filename)

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

        if getArgumentValue(SILENT_KEY, False, type=bool):
            runProcessBackground(TEST_PROCESS_PATH,
                                self.test_process_arguments[index])
        else:
            runProcessNewTerminal(TEST_PROCESS_PATH,
                                  self.test_process_arguments[index],
                                  self.opened_terminals)
            self.opened_terminals += 1

        time.sleep(self.test_process_after_start_pause[index]/1000)

    def startTestProcess(self, config_file, name = None, tokens = None,
                         log_file = None, count = 1, pause=0.1):
        self.addTestProcess(config_file, name, tokens, log_file, count, pause)

        process_count = len(self.test_process_names)
        first_new_process_index = process_count - count
        for index in range(first_new_process_index, process_count):
            self.__runTestProcess(index)

    def killAll(self, passed=True):
        wait = getArgumentValue(WAIT_KEY, False, type=bool)
        if wait or not passed:
            input("Press enter to continue...")

        killProcessName(TEST_PROCESS_NAME)
        for log_path in self.test_process_log_paths:
            self.logger.logMessage(KILLED_MESSAGE, log_path)

        if self.kill_control_server:
            killProcessName(CONTROL_SERVER_NAME)
            self.logger.logMessage(KILLED_MESSAGE, self.control_server_log_path)

    def getRunLogPrexif(self):
        message = ""
        if self.cycle_count > 1:
            message = "Run {}/{}: ".format(self.cycle_index + 1, self.cycle_count)
        return message

    def waitWhileTesting(self, duration=15000):
        run_prefix = self.getRunLogPrexif()
        if run_prefix:
            self.logger.logMessage(run_prefix)
        for remaining in range(duration//1000, 0, -1):
            sys.stdout.write("\r")
            sys.stdout.write(run_prefix + "Remaining: {:2d} seconds".format(remaining))
            sys.stdout.flush()
            time.sleep(1)
        sys.stdout.write("\r")
        print(run_prefix + "Finished...                                      ")

    def getTestProcessResults(self):
        results = dict()
        for log_path, name in zip(self.test_process_log_paths, self.test_process_names):
            result_object = TestProcessResult(log_path, self.name)
            if name in results:
                results[name].append(result_object)
            else:
                results[name] = [result_object]
        return results

    def getControlServerResult(self):
        result_object = TestProcessResult(self.control_server_log_path, self.name)
        return result_object

    def runTest(self, evaluator = None, cycle_duration=10000, cycle_count=1):
        self.cycle_count = cycle_count

        self.logger.logAndPrintTestStarted(self.name, self.setup_string)

        for run_index in range(self.cycle_count):
            self.cycle_index = run_index
            self.setup()
            for process_index in range(len(self.test_process_names)):
                self.__runTestProcess(process_index)
            self.waitWhileTesting(cycle_duration)
            if evaluator is not None:
                evaluator.setLogger(self.logger)
                evaluator.setProcessResults(self.getTestProcessResults(),
                                            self.getControlServerResult())
                evaluator.setupProcessResults()
                # Reading logs before killing the processes saves a lot of troubles.
                evaluator.readAllLogs()
                passed = evaluator.evaluate()
                if passed:
                    self.logger.logAndPrint(PASSED_MESSAGE, 1)
                else:
                    self.logger.logAndPrint(FAILED_MESSAGE, 1)
                self.killAll(passed)
            else:
                 self.logger.logAndPrint(NO_EVALUATION_MESSAGE, 1)
                 self.killAll()
        print(TERMINAL_SEPARATOR)
