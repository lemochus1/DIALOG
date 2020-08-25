import glob
import os
import sys

from define.arguments import *

#===================================================================================================
# Parameters
#===================================================================================================

# Test
HANDLER_COUNT          = [30]
PAUSE_BETWEEN_MESSAGES = [10]
MESSAGE_SIZE           = [1000000]
MESSAGE_COUNT          = [20]

# Run
CYCLE_COUNT = 1
CYCLE_DURATION = 20000

RUNNING_SCRIPT = "command-basic-test.py"

#===================================================================================================
# Scripting
#===================================================================================================

if __name__ == "__main__":
    print("-----------------------------------------")
    index = 1
    total = len(MESSAGE_COUNT) * len(HANDLER_COUNT) * len(PAUSE_BETWEEN_MESSAGES) * len(MESSAGE_SIZE)

    for handler_count in HANDLER_COUNT:
        for message_pause in PAUSE_BETWEEN_MESSAGES:
            for message_size in MESSAGE_SIZE:
                for message_count in MESSAGE_COUNT:
                    print("Test: {}/{}".format(index, total))
                    index += 1
                    arguments = [
                                 "{}={}".format(TEST_NAME_KEY, sys.argv[0]),
                                 "{}={}".format(HANDLER_COUNT_KEY, handler_count),
                                 "{}={}".format(PAUSE_BETWEEN_MESSAGES_KEY, message_pause),
                                 "{}={}".format(MESSAGE_SIZE_KEY, message_size),
                                 "{}={}".format(MESSAGE_COUNT_KEY, message_count),
                                 "{}={}".format(CYCLE_COUNT_KEY, CYCLE_COUNT),
                                 "{}={}".format(CYCLE_DURATION_KEY, CYCLE_DURATION),
                                 "{}={}".format(SILENT_KEY, True)
                                ]
                    os.system("python3 {} {}".format(RUNNING_SCRIPT, " ".join(arguments)))
