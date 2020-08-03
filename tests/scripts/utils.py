# This Python file uses the following encoding: utf-8
import subprocess
import shutil
import os
import signal

from define import *

def runProcessNewTerminal(executable_path, arguments = None):
    args = ['gnome-terminal', '--', executable_path]

    if arguments is not None:
        args.append('--')
        if isinstance(arguments, list):
            args.extend(arguments)
        else:
            args.append(arguments)
    subprocess.call(args)


def runControlServer(executable_path = None):
    if executable_path is None:
        executable_path = CONTROL_SERVER_PATH

    runProcessNewTerminal(executable_path)


def runTestProcess(config_filename, executable_path = None):
    if executable_path is None:
        executable_path = TEST_PROCESS_PATH

    cofig_path = TEST_CONFIGS_FOLDER + config_filename

    runProcessNewTerminal(executable_path, cofig_path)


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
    return list(map(int, subprocess.check_output(["pidof", processName]).split()))


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

