##############################################################################
CFLAGS:=-Wall -g
CPPFLAGS:=$(shell pkg-config --cflags cairo)
LDLIBS:=$(shell pkg-config --libs cairo)
##############################################################################
EXECNAME:=lightout
SRCS:=drawutil.c framework.c gradient.c lightout.c
OBJS:=$(SRCS:%.c=%.o)
##############################################################################
all : $(EXECNAME)

clean :
	$(RM) $(EXECNAME) $(OBJS)
##############################################################################
$(EXECNAME) : $(OBJS)
