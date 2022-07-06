#!/usr/bin/env python3
#
import os
import shutil
import sys
import subprocess
import time
import datetime

#'IT-Panel-1.1','-f','spleen-16x32.bdf','--led-gpio-mapping=adafruit-hat-pwm','--led-chain=4','--led-brightness=50','--led-slowdown-gpio=4','--led-limit-refresh=120'

myPopen = subprocess.Popen(['/root/rpi-rgb-led-matrix/utils/IT-Panel-1.1','-f','/root/rpi-rgb-led-matrix/utils/spleen-16x32.bdf','--led-gpio-mapping=adafruit-hat-pwm','--led-chain=4','--led-brightness=50','--led-slowdown-gpio=4','--led-limit-refresh=120'], stdin = subprocess.PIPE,stdout = subprocess.PIPE, stderr = subprocess.PIPE, encoding = 'ascii',bufsize=1,universal_newlines=True)
time.sleep (10)
myPopen.stdin.write('ligne1\n')
time.sleep (10)
myPopen.stdin.write('ligne2\n')
time.sleep (10)
myPopen.stdin.write('ligne3\n')
time.sleep (10)

myPopen.stdin.close()
while True:
    status = myPopen.poll()
    if status is not None:
        break
for line in myPopen.stdout:
    sys.stdout.write(line)
print(status)
