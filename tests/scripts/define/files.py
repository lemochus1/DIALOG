#===================================================================================================
# File names
#===================================================================================================

CONTROL_SERVER_NAME   = "DIALOGCommunicationTestControlServer"
TEST_PROCESS_NAME     = "DIALOGTestProcess"
DIALOG_BINARIES_NAMES = ["libDIALOGCommunication.so",
                         "libDIALOGCommunication.so.1",
                         "libDIALOGCommunication.so.1.0",
                         "libDIALOGCommunication.so.1.0.0"]

#===================================================================================================
# Paths
#===================================================================================================

CONTROL_SERVER_PATH = "/home/suchoale/Vyzkumak/DIALOG/build/DIALOGCommunicationTestControlServer/" \
                      + CONTROL_SERVER_NAME
TEST_PROCESS_PATH   = "/home/suchoale/Vyzkumak/DIALOG/build/DIALOGTestProcess/" + TEST_PROCESS_NAME

#===================================================================================================
# Folders
#===================================================================================================

TEST_CONFIGS_FOLDER = "/home/suchoale/Vyzkumak/DIALOG/tests/processes/"
TEST_RESULTS_FOLDER = "/home/suchoale/Vyzkumak/DIALOG/tests/results/"

SHARED_OBJECTS_FOLDER = "/home/suchoale/my_so/"
DIALOG_BUILD_FOLDER   = "/home/suchoale/Vyzkumak/DIALOG/build/DIALOGCommunication/"

#===================================================================================================
# Suffixes
#===================================================================================================

CONFIG_SUFFIX = ".proc"
LOG_SUFFIX = ".log"
SCRIPT_SUFFIX = ".py"
