MODULE:=lightout
all :
-include extra.mk
##############################################################################
PKG_CONFIG?=pkg-config
##############################################################################
PACKAGES:=cairo-xlib
CFLAGS+=-Wall -g
CPPFLAGS+=$(shell $(PKG_CONFIG) --cflags $(PACKAGES))
LDLIBS+=$(shell $(PKG_CONFIG) --libs $(PACKAGES)) -lXext
##############################################################################
SRCS:=drawutil.c framework.c gradient.c graphictest.c lightout.c shaped.c
OBJS:=$(SRCS:%.c=%.o)
##############################################################################
all : $(MODULE)

clean :
	-rm -rf $(MODULE) $(OBJS)
##############################################################################
$(MODULE) : $(OBJS)
