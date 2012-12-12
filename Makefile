default: speedtest
speedtest: speedtest.c
	gcc -Wall -pthread -lssl -lcrypto -o speedtest speedtest.c
