all:
	gcc tcpserver.c -o server

clean: 
	rm -rf server *.log *.txt
