#!/usr/bin/python

import socket
from subprocess import Popen, PIPE

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.bind(('', 9876))

while True:
  data, addr = sock.recvfrom(1024)
  if data == 'next\n':
    code = 124
    print "move forward"
  elif data == 'prev\n':
    code = 123
    print "move backward"
  else:
    print "illegal code: ", data
    continue

  script = '''
    tell application "Kindle" to activate
    tell application "System Events"
        key code {}
    end tell'''.format(code)

  p = Popen(['osascript', '-'], stdin=PIPE, stdout=PIPE, stderr=PIPE)
  stdout, stderr = p.communicate(script)
  print (p.returncode, stdout, stderr)

