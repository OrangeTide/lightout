MODULE:=graphictest
all :
-include extra.mk
##############################################################################
PKG_CONFIG?=pkg-config
##############################################################################
PACKAGES:=cairo-xlib
CFLAGS+=-Wall -g
CPPFLAGS+=$(shell $(PKG_CONFIG) --cflags $(PACKAGES))
LDLIBS+=$(shell $(PKG_CONFIG) --libs $(PACKAGES))
##############################################################################
SRCS:=drawutil.c framework.c gradient.c graphictest.c lightout.c
OBJS:=$(SRCS:%.c=%.o)
##############################################################################
all : $(MODULE)

clean :
	-rm -rf $(MODULE) $(OBJS)
##############################################################################
$(MODULE) : $(OBJS)
