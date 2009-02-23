CFLAGS:=-Wall -g
CPPFLAGS:=$(shell pkg-config --cflags cairo)
LDLIBS:=$(shell pkg-config --libs cairo)

all : lightout

clean :
	$(RM) lightout
