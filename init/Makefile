###############################################################################
# Copyright © 2016 epsilonRT, All rights reserved.                            #
# This software is governed by the CeCILL license <http://www.cecill.info>    #
###############################################################################

TARGET = dmg-iaqmon

#---------------- Install Options ----------------
INSTALL_BINDIR=/etc/init.d
INSTALL_DATDIR=/etc
MSG_INSTALL = [INSTALL]
MSG_UNINSTALL = [UNINSTALL]

all:
rebuild: 
clean: 
distclean: 

install: uninstall
	@echo "$(MSG_INSTALL) $(TARGET) in $(INSTALL_BINDIR)"
	@-install -m 0644 --backup=existing $(TARGET).conf $(INSTALL_DATDIR)
	@-install -m 0755 $(TARGET) $(INSTALL_BINDIR)

uninstall:
	@echo "$(MSG_UNINSTALL) $(TARGET) from $(INSTALL_BINDIR)"
ifeq (,$(wildcard $(INSTALL_BINDIR)/$(TARGET)))
else
	@-$(INSTALL_BINDIR)/$(TARGET) stop
endif
	@-(test -r $(INSTALL_DATDIR)/$(TARGET).conf && mv -f $(INSTALL_DATDIR)/$(TARGET).conf $(INSTALL_DATDIR)/$(TARGET).conf.bak)
	@-rm -f $(INSTALL_BINDIR)/$(TARGET)

.PHONY: all rebuild clean distclean install uninstall
