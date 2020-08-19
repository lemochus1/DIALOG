import glob
import os
import sys

from define.arguments import *

#===================================================================================================
# Parameters
#===================================================================================================

# Test
PAUSE_BETWEEN_CONNECTS = [100, 50, 20, 0]
PROCESS_COUNT          = [20, 50, 100, 200]

# Run
CYCLE_COUNT = 2
CYCLE_DURATION = 20000 # Shorter waiting will cause fail as the connects and logs take some time.

RUNNING_SCRIPT = "connect-basic-test.py"

#===================================================================================================
# Scripting
#===================================================================================================

if __name__ == "__main__":
    print("-----------------------------------------")
    index = 1
    total = len(PAUSE_BETWEEN_CONNECTS) * len(PROCESS_COUNT)

    for connect_pause in PAUSE_BETWEEN_CONNECTS:
        for process_count in PROCESS_COUNT:
            print("Test: {}/{}".format(index, total))
            index += 1
            arguments = [
                         "{}={}".format(TEST_NAME_KEY, sys.argv[0]),
                         "{}={}".format(PAUSE_BETWEEN_CONNECTS_KEY, connect_pause),
                         "{}={}".format(PROCESS_COUNT_KEY, process_count),
                         "{}={}".format(CYCLE_COUNT_KEY, CYCLE_COUNT),
                         "{}={}".format(CYCLE_DURATION_KEY, CYCLE_DURATION),
                         "{}={}".format(SILENT_KEY, True)
                        ]
            os.system("python3 {} {}".format(RUNNING_SCRIPT, " ".join(arguments)))
