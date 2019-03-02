
LDFLAGS+= -L.. -ldarknet -lpthread -lmicrohttpd -ljansson
CFLAGS += -fdiagnostics-color=auto -Wall -Wextra -Wno-unused -Wno-unused-parameter -Wno-error -g -I../include/

CFLAGS += -g -O3 -march=native -mtune=native


TARGET=restserver
OBJS=$(TARGET).o base64.o debug.o config.o httpjson.o darknetif.o

all: $(TARGET)

$(OBJS): Makefile *.h

$(TARGET): $(OBJS)

.PHONY: clean
clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: stop
stop:
	echo "Implement me"

.PHONY: start
start:
	echo "Implement me"

.PHONY: restart
restart: stop start

.PHONY: test
test: all
	./restserver -vvv
