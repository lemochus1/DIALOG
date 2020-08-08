import time

from utils.build import *
from utils.process import *

testsSetup()
time.sleep(1)

runTestProcess("service-subscriber.proc")
time.sleep(1)

runTestProcess("service-provider.proc")
time.sleep(1)

runTestProcess("service-subscriber.proc")
