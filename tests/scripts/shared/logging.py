import os

from datetime import datetime

from define.logs import *
from define.files import *

#===================================================================================================
# Class TestLogger
#===================================================================================================

class TestLogger:

    def __init__(self, test_name):
        if test_name.endswith(SCRIPT_SUFFIX):
            test_name = test_name.replace(SCRIPT_SUFFIX, "")
        self.default_log_path = os.path.join(TEST_RESULTS_FOLDER,
                                             test_name,
                                             "evaluation" + LOG_SUFFIX)

    def __getTimeString(self):
        now = datetime.now()
        return '"' + now.strftime("%Y-%m-%d %H:%M:%S") + '" '

    def logMessage(self, message, log_path=None):
        if log_path is None:
            log_path = self.default_log_path;

        with open(log_path, 'a') as file_object:
            file_object.write(self.__getTimeString() + message + "\n")

    def logSeparatedMessage(self, message, file_paths):
        if not isinstance(file_paths, list):
            file_paths = [file_paths]
        for log_path in file_paths:
            with open(log_path, 'a') as file_object:
                file_object.write(LOG_SEPARATOR)
                file_object.write(self.__getTimeString() + message + "\n")
                file_object.write(LOG_SEPARATOR)

    def logSeparator(self):
        with open(self.default_log_path, 'a') as file_object:
            file_object.write(LOG_SEPARATOR)

    def logEvaluationStart(self):
        with open(self.default_log_path, 'a') as file_object:
            file_object.write(LOG_SEPARATOR)
            file_object.write(self.__getTimeString() + "Evaluation started!\n")
            file_object.write(LOG_SEPARATOR)

    def logAndPrintTestStarted(self, name, params_string=""):
        print(TERMINAL_SEPARATOR)
        self.logSeparator()
        self.logAndPrint(name)

        if params_string:
            self.logAndPrint("Params: " + params_string, 1)
        self.logSeparator()

    def logAndPrint(self, message, tab_count=0):
        prefix = ""
        for i in range(tab_count):
            prefix += TAB
        self.logMessage(message)
        print(prefix + message)

    def logAndPrintList(self, messages, tab_count=0):
        for message in messages:
            self.logAndPrint(message, tab_count)
