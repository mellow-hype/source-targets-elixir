# compiler flags
#CC = gcc
#M4= m4

#CFLAGS =-Wall -g -O2
#CFLAGS =-DDPROXY_DEBUG
CFLAGS =-DDNS_DYN_CACHE
# CFLAGS =-I$(BUILD_DIR)/userspace/public/include
BIN_DIR=$(INSTALL_DIR)/bin
#CONFIG_DIR=$(INSTALL_DIR)/etc

SLACKWARE_SCRIPT=/etc/rc.d/rc.inet2
##############################################
# uncoment the following if you have REDHAT 
#DIST= -DREDHAT
#CACHE_DIR=/var/cache
#DHCP_LEASES=/var/state/dhcp.leases
#RC_SCRIPT_DIR=/etc/rc.d/init.d

CACHE_DIR=$(INSTALL_DIR)/var/cache
DHCP_LEASES=$(INSTALL_DIR)/var/udhcpd/udhcpd.leases

##############################################
# uncoment the following if you have DEBIAN
#DIST= -DDEBIAN
#CACHE_DIR=/var/cache
# not shure where the dhcp stuff is
#DHCP_LEASES=/var/state/dhcp.leases
#RC_SCRIPT_DIR=/etc/rc.d/init.d

##############################################
# uncoment the following if you have SUSE
#DIST= -DSUSE
#CACHE_DIR=/var/dproxy
# not shure where the dhcp stuff is
#DHCP_LEASES=/var/dhcpd/dhcp.leases
#RC_SCRIPT_DIR=/sbin/init.d

##############################################
# uncoment the following if you have SLACKWARE
#DIST= -DSLACK
#CACHE_DIR=/var/cache
#DHCP_LEASES=/var/state/dhcp.leases
######## END OF CONFIGURABLE OPTIONS #########

CACHE_FILE=$(CACHE_DIR)/dproxy.cache
CONFIG_FILE=$(CONFIG_DIR)/dproxy.conf

#DEFAULTS= -DCACHE_FILE_DEFAULT=\"$(CACHE_FILE)\" \
#	  -DDHCP_LEASES_DEFAULT=\"$(DHCP_LEASES)\" \
#	  -DCONFIG_FILE_DEFAULT=\"$(CONFIG_FILE)\"

RCDEFS= $(DIST) -DBIN_DIR="$(BIN_DIR)" -DCONFIG_DIR="$(CONFIG_DIR)" 

# install stuf
INSTALL=install

OBJS=dproxy.o dns_decode.o dns_list.o cache.o conf.o dns_construct.o dns_io.o dns_probe.o dns_dyn_cache.o

#dynamic: install

#all: dproxy dproxy.rc dproxy.conf
all: dnsproxy

dnsproxy: $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS)

%.o : %.c Makefile
	$(CC) -c $(DEFAULTS) $(CFLAGS) $<

dproxy.rc:  dproxy.rc.m4 Makefile
	$(M4) $(RCDEFS) $< >$@

dproxy.conf:  dproxy
	@./dproxy -P > dproxy.conf

clean:
	rm -f *.o *~ core dproxy dproxy.rc dproxy.conf

install: all 
	$(INSTALL) -m 755 dproxy $(BIN_DIR)/dnsproxy

xxx-install: all 
	$(INSTALL) -s dproxy $(BIN_DIR)/dnsproxy
	$(INSTALL) -d $(CACHE_DIR)

ifeq ($(DIST), -DSLACK)
	@echo "Attempting Slackware install";
	@if [ -f $(SLACKWARE_SCRIPT) ] && ! grep -e dproxy $(SLACKWARE_SCRIPT) >/dev/null; then \
		echo 'Adding dproxy to $(SLACKWARE_SCRIPT)'; \
      echo '# dproxy (DNS caching proxy)' >>$(SLACKWARE_SCRIPT); \
      echo 'if [ -x $(BIN_DIR)/dproxy ]; then' >>$(SLACKWARE_SCRIPT); \
      echo '  echo "Starting dproxy"' >>$(SLACKWARE_SCRIPT); \
      echo '  $(BIN_DIR)/dproxy -c$(CONFIG_DIR)/dproxy.conf' >>$(SLACKWARE_SCRIPT); \
      echo 'fi' >>$(SLACKWARE_SCRIPT); \
		echo "success."; \
	else \
		echo "Slackware install failed, could not find script '$(SLACKWARE_SCRIPT)' or dproxy was already enabled within"; \
	fi	

else
	$(INSTALL) dproxy.rc $(RC_SCRIPT_DIR)/dproxy
endif

	@if [ -f $(CONFIG_DIR)/dproxy.conf ] ; \
	then \
	   mv $(CONFIG_DIR)/dproxy.conf $(CONFIG_DIR)/dproxy.conf.saved ; \
	   echo "*******************************************************";\
	   echo "NOTE : your old dproxy configuration has been moved to"; \
	   echo "  $(CONFIG_DIR)/dproxy.conf.saved " ; \
	   echo "you may want to restore it before you restart dproxy." ; \
	   echo "*******************************************************";\
	fi
	$(INSTALL) dproxy.conf $(CONFIG_DIR)/dproxy.conf

uninstall:
	rm -f $(BIN_DIR)/dnsproxy
	rm -f $(CACHE_DIR)/dproxy.cache $(CACHE_DIR)/dproxy.
	rm -f $(RC_SCRIPT_DIR)/dproxy
	rm -f $(CONFIG_DIR)/dproxy.conf

dproxy.o: dproxy.c dproxy.h dns.h cache.h conf.h
cache.o: cache.c cache.h dproxy.h dns.h conf.h
conf.o: conf.c conf.h dproxy.h dns.h
dns_decode.o: dns_decode.c dns_decode.h dproxy.h
dns_list.o: dns_list.c dns_list.h dproxy.h
dns_construct.o: dns_construct.c dns_construct.h dproxy.h
dns_probe.o: conf.h dproxy.h
dns_dyn_cache.o: dns_dyn_cache.h dproxy.h dns.h cache.h conf.h
