all: tcpclient async-tcpserver

tcpclient: tcpclient.c
	gcc -o tcpclient tcpclient.c

async-tcpserver: async-tcpserver.c
	gcc -pthread -o async-tcpserver async-tcpserver.c

clean:
	rm -f *.o tcpclient async-tcpserver