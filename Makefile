default: speedtest-locked speedtest-copied
speedtest-locked: speedtest.c Makefile
	gcc -Wall -pthread -lrt -lssl -DLOCK_STATIC_EVP_MD_CTX -o speedtest-locked speedtest.c
speedtest-copied: speedtest.c Makefile
	gcc -Wall -pthread -lrt -lssl -o speedtest-copied speedtest.c
clean:
	rm -f speedtest-copied speedtest-locked
