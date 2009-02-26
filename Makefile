##############################################################################
CFLAGS:=-Wall -g
CPPFLAGS:=$(shell pkg-config --cflags cairo)
LDLIBS:=$(shell pkg-config --libs cairo)
##############################################################################
EXECNAME:=lightout
SRCS:=lightout.c framework.c
OBJS:=$(SRCS:%.c=%.o)
##############################################################################
all : $(EXECNAME)

clean :
	$(RM) $(EXECNAME) $(OBJS)
##############################################################################
$(EXECNAME) : $(OBJS)
