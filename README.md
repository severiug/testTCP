# Сlient-server application for Linux
-------------------------------------

Client - a program launched from the console.
The server is a daemon that terminates correctly using the SIGTERM and SIGHUP signals.
The client sends the contents of the text file via TCP.
Server must accept and save the message text to file.
<hr>
Includes 2 directories with source code and makefile in each one.
To build tcpclient.c and tcpserver.c use command "make"
To launch tcpserver with port to listen use "./server port"
to launch tcpclient to connect server with ip and port use "./client ip:port"
Then follow menu options, to input path to file use its location on disk.

Compiling with gcc version 7.5.0 (Ubuntu 7.5.0-3ubuntu1~16.04) 

