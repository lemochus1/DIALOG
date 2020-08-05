# This Python file uses the following encoding: utf-8
from utils import *
import time

testsSetup()
time.sleep(1)
runTestProcess("service-subscriber.proc")
time.sleep(1)
runTestProcess("service-provider.proc")
time.sleep(1)
runTestProcess("service-subscriber.proc")
