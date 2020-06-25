options = -lrt -lpthread -Wall -pedantic -std=gnu99

default: clean mapper2310 control2310 roc2310

mapper2310: mapper2310.o error.o server.o list.o utils.o
	gcc $(options) -g -o mapper2310 mapper2310.o error.o server.o list.o utils.o

mapper2310.o:
	gcc $(options) -g -c mapper2310.c

control2310: control2310.o error.o server.o client.o list.o utils.o
	gcc $(options) -g -o control2310 control2310.o error.o server.o client.o list.o utils.o

control2310.o:
	gcc $(options) -g -c control2310.c

roc2310: roc2310.o error.o client.o list.o utils.o
	gcc $(options) -g -o roc2310 roc2310.o error.o client.o list.o utils.o

roc2310.o:
	gcc $(options) -g -c roc2310.c

server.o:
	gcc $(options) -g -c server.c

client.o:
	gcc $(options) -g -c client.c

error.o:
	gcc $(options) -g -c error.c

list.o:
	gcc $(options) -g -c list.c

utils.o:
	gcc $(options) -g -c utils.c

clean:
	$(RM) roc2310 control2310 mapper2310 *.o