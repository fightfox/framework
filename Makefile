CC = g++

CFLAGS = -g -std=c++14 -O2

CPPFLAGS = -I/usr/local/opt/openssl/include \
					 -I/usr/local/opt/libuv/include \
					 -I/usr/local/include

LDFLAGS = -L/usr/local/lib -luWS -lz -lssl -luv -pthread

SOURCES = test_cc/animate_test.cc \
					lib/event/Event.cc \
					lib/event/Init.cc \
					lib/event/Start.cc \
					lib/event/Close.cc \
					lib/event/Command.cc \
					lib/event/UpdateWorld.cc \
					lib/event/OnCollision.cc \
					lib/event/EventQueue.cc \
					lib/engine/Game.cc \
					lib/io/SocketManager.cc \
					lib/engine/Player.cc \
					lib/event/RemoveUnit.cc \
					lib/event/CreateUnit.cc \
					lib/event/CreatePlayer.cc \
					lib/event/End.cc \
					lib/event/RemovePlayer.cc



OBJECTS = $(SOURCES:.cc=.o)

TARGET = animate_test.out

$(TARGET) : $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $^ $(CPPFLAGS) $(LDFLAGS)

%.o : %.cc
	$(CC) $(CFLAGS) -c $< -o $@ $(CPPFLAGS)

.PHONY: clean

clean:
	@rm -f $(TARGET) $(OBJECTS)