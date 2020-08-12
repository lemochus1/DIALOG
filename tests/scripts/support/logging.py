import os
from datetime import datetime

from support.utils.define import *

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

    def logEvaluationStart(self):
        with open(self.default_log_path, 'a') as file_object:
            file_object.write(LOG_SEPARATOR)
            file_object.write(self.__getTimeString() + "Evaluation started!\n")
            file_object.write(LOG_SEPARATOR)

    def logAndPrint(self, message, tab=False):
        self.logMessage(message)
        if tab:
            print(TAB + message)
        else:
            print(message)

    def logAndPrintList(self, messages, tab=False):
        for message in messages:
            self.logAndPrint(message, tab)
