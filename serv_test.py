#!/usr/bin/env python2
import socket

s = socket.socket()

s.bind(('0.0.0.0', 2222 ))
s.listen(0)                 
 
while True:
 
    client, addr = s.accept()
 
    while True:
        content = client.recv(32)
 
        if len(content) == 0:
           print("Client sent nothing")
        else:
            print(content)
            client.send("{\"result\": 0, \"id\": 8721, \"errorMsg\": \"\"}")
 
    print("Closing connection")
    client.close()
