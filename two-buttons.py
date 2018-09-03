#!/usr/bin/python

import socket
from subprocess import Popen, PIPE

# listen on a UDP port
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.bind(('', 9876))

# loop forever, blocking until socket data is available
while True:
    data, addr = sock.recvfrom(1024)
    # the commands should just be 4 characters
    data = data[0:4]
    if data == 'next':
        code = 124
        print "move forward"
    elif data == 'prev':
        code = 123
        print "move backward"
    else:
        print "illegal code:", data
        continue

    # Apple, I applaud you for making it so easy to automate stuff!
    # Define the script and fill in the key code from above
    script = '''
        tell application "Kindle" to activate
        tell application "System Events"
            key code {}
        end tell'''.format(code)

    # Actually run the applescript
    p = Popen(['osascript', '-'], stdin=PIPE, stdout=PIPE, stderr=PIPE)
    stdout, stderr = p.communicate(script)
    print (p.returncode, stdout, stderr)
