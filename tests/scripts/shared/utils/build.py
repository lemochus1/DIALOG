import shutil
import os

from define.files import *


def moveDialogToSharedDirectory(binaries_names = None, build_directory = None,
                                shared_directory = None):
    if binaries_names  is None:
        binaries_names  = DIALOG_BINARIES_NAMES

    if build_directory is None:
        build_directory = DIALOG_BUILD_FOLDER

    if shared_directory is None:
        shared_directory = SHARED_OBJECTS_FOLDER

    for binary_name in binaries_names:
        origin_path = os.path.join(build_directory, binary_name)
        target_path = os.path.join(shared_directory, binary_name)
        if os.path.exists(target_path):
            os.remove(target_path)
        shutil.copyfile(origin_path, target_path)
