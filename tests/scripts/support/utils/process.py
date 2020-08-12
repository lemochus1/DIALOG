import os
import signal

from support.utils.terminal import *
from support.utils.define import *


def getProcessPid(processName):
    try:
        pidlist = list(map(int,
                           subprocess.check_output(["pidof",
                                                    processName]).split()))
    except:
        pidlist = []
    return pidlist


def isProcessRunning(processName):
    return bool(getProcessPid(processName))


def killProcessPid(pid):
    os.kill(pid, signal.SIGTERM)


def killProcessName(name):
    for pid in getProcessPid(name):
        killProcessPid(pid)
