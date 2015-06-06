#-*- coding:utf-8 -*-
import sys
import os
import time

reload(sys)
sys.setdefaultencoding('utf8')

v = 0
for i in range(0, 10000000):
    v += i
