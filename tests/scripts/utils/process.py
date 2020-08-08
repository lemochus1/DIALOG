import os
import signal

from utils.terminal import *
from utils.define import *


def runControlServer(executable_path = None):
    if executable_path is None:
        executable_path = CONTROL_SERVER_PATH
    runProcessNewTerminal(executable_path)


def runTestProcess(config_filename, arguments = None, executable_path = None):
    config_path = TEST_CONFIGS_FOLDER + config_filename

    if executable_path is None:
        executable_path = TEST_PROCESS_PATH
    if arguments is None:
        runProcessNewTerminal(executable_path, config_path)
    else:
        if isinstance(arguments, list):
            arguments.insert(0, config_path)
        else:
            arguments = [config_path, arguments]
        runProcessNewTerminal(executable_path, arguments)


def getProcessPid(processName):
    try:
        pidlist = list(map(int, subprocess.check_output(["pidof", processName]).split()))
    except:
        pidlist = []
    return pidlist


def isProcessRunning(processName):
    return bool(getProcessPid(processName))


def killProcessPid(pid):
    os.kill(pid, signal.SIGTERM)


def killProcessName(name):
    process_pids = getProcessPid(name)
    if process_pids:
        killProcessPid(process_pids[0])


def killControlServer():
    killProcessName(CONTROL_SERVER_NAME)


def killAllTestProcesses():
    test_process_pids = getProcessPid(TEST_PROCESS_NAME)
    for pid in test_process_pids:
        killProcessPid(pid)
