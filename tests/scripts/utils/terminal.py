import subprocess

from utils.define import *

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
    subprocess.Popen(args)
