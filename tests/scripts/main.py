# This Python file uses the following encoding: utf-8
import subprocess
import shutil
import os
from utils import *


so_directory = "/home/suchoale/my_so/"

build_dialog = "/home/suchoale/Vyzkumak/DIALOG/build/DIALOGCommunication/"
so_dialog = ["libDIALOGCommunication.so","libDIALOGCommunication.so.1","libDIALOGCommunication.so.1.0","libDIALOGCommunication.so.1.0.0"]

executable_control_server = "/home/suchoale/Vyzkumak/DIALOG/build/DIALOGCommunicationControlServer/_rccars_DIALOGCommunicationControlServer"


executable_test_process = "/home/suchoale/Vyzkumak/DIALOG/build/DIALOGTestProcess/DIALOGTestProcess"


for binary in so_dialog:
    if os.path.exists(so_directory + binary):
        os.remove(so_directory + binary)
    shutil.copyfile(build_dialog + binary, so_directory + binary)


runControlServer()

#args = ['gnome-terminal', '--', executable_control_server, '--', '-b=10', '/c']
#subprocess.call(args)

#args = ['gnome-terminal', '--', executable_test_process]
#subprocess.call(args)


#subprocess.call(['gnome-terminal', '-x', 'python bb.py'])
