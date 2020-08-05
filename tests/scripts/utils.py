# This Python file uses the following encoding: utf-8
import subprocess
import shutil
import os
import signal

from define import *

opened_terminals = 0


def getNextTerminalGeometry():
    global opened_terminals
    horizonal_drift = (opened_terminals % TERMINALS_ON_LINE) * TERMINAL_WIDTH * LINE_WIDTH
    vertical_drift = (opened_terminals // TERMINALS_ON_LINE) * TERMINAL_HEIGHT * LINE_HEIGHT
    opened_terminals += 1
    return str(TERMINAL_WIDTH) + 'x' + str(TERMINAL_HEIGHT) + '+' + str(int(horizonal_drift)) + '+' + str(int(vertical_drift))


def runProcessNewTerminal(executable_path, arguments = None):
    args = ['gnome-terminal', '--geometry=' + getNextTerminalGeometry(), '--', executable_path]

    if arguments is not None:
        if isinstance(arguments, list):
            args.extend(arguments)
        else:
            args.append(arguments)
    subprocess.call(args)


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


def moveDialogToSharedDirectory(binaries_names = None, build_directory = None, shared_directory = None):
    if binaries_names  is None:
        binaries_names  = DIALOG_BINARIES_NAMES

    if build_directory is None:
        build_directory = DIALOG_BUILD_FOLDER

    if shared_directory is None:
        shared_directory = SHARED_OBJECTS_FOLDER

    for binary_name in binaries_names:
        origin_path = build_directory + binary_name
        target_path = shared_directory + binary_name
        if os.path.exists(target_path):
            os.remove(target_path)
        shutil.copyfile(origin_path, target_path)


def testsSetup(run_control_server = True):
    moveDialogToSharedDirectory()
    if run_control_server:
        runControlServer()


def getProcessPid(processName):
    try:
        pidlist = list(map(int, subprocess.check_output(["pidof", processName]).split()))
    except:
        pidlist = []
    return pidlist


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
