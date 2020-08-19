import os
import signal
import subprocess

from shared.utils.containers import *

TERMINAL_WIDTH = 65 # lines
TERMINAL_HEIGHT = 18 # lines
LINE_HEIGHT = 25
LINE_WIDTH = 10
TERMINALS_ON_LINE = 3


def getNextTerminalGeometry(opened_terminals = 0):
    horizonal_drift = (opened_terminals % TERMINALS_ON_LINE) * TERMINAL_WIDTH * LINE_WIDTH
    vertical_drift = (opened_terminals // TERMINALS_ON_LINE) * TERMINAL_HEIGHT * LINE_HEIGHT
    opened_terminals += 1
    return "{}x{}+{}+{}".format(TERMINAL_WIDTH, TERMINAL_HEIGHT, horizonal_drift, vertical_drift)


def runProcessNewTerminal(executable_path, arguments = None, opened_terminals = 0):
    args = ['gnome-terminal',
            '--geometry=' + getNextTerminalGeometry(opened_terminals),
            '--',
            executable_path]
    addToList(args, arguments)
    subprocess.Popen(args)


def runProcessBackground(executable_path, arguments = None):
    args = [executable_path]
    addToList(args, arguments)

    FNULL = open(os.devnull, 'w')
    subprocess.Popen(args, stdout=FNULL, stderr=subprocess.STDOUT)


def getProcessPid(process_name):
    try:
        pid_list = list(map(int,
                           subprocess.check_output(["pidof",
                                                    process_name]).split()))
    except:
        pid_list = []
    return pid_list


def isProcessRunning(process_name):
    return bool(getProcessPid(process_name))


def killProcessPid(pid):
    os.kill(pid, signal.SIGTERM)


def killProcessName(process_name):
    for pid in getProcessPid(process_name):
        killProcessPid(pid)
