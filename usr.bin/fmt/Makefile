#
# $Id: Makefile,v 1.1 1997/09/21 22:12:26 gdr Exp $
#
include ../../../paths.mk
include ../../constants.mk

BUILD_TARGET = $(BUILD_DIR)/fmt
CPPFLAGS += -I../../hdr

build: setup $(BUILD_TARGET)

$(BUILD_TARGET): $(OBJS)
	$(CC) -o $@ $(LDFLAGS) $(OBJS) $(LDLIBS)

install:
	$(INSTALL) -m755 -d $(INSTROOT)/$(BUILD_DIR)/install
	$(INSTALL) -m755 $(BUILD_TARGET) $(INSTROOT)/$(BUILD_DIR)/install
	$(INSTALL) -m644 fmt.README $(INSTROOT)/$(BUILD_DIR)/install

clobber::
	$(RM) -f *~

include ../../rules.mk
-include $(BUILD_DIR)/dependencies
