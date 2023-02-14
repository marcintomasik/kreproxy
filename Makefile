#
#	GPL v3.0 license
#
#	kREproxy project (C) Marcin Tomasik 2009
#
#	<marcin[at]tomasik.pl>
#
#	http://kreproxy.sourceforge.net/
#

MODULENAME=kreproxy
DEBUG=n

ifeq ($(DEBUG),y)
DEBFLAGS= -DKRE_DEBUG_MODE
else
DEBFLAGS=
endif

LDFLAGS=
# probably CFLAGS is set before this declaration
#CFLAGS= # this cause problem with compilation for example with linux/netdevices.h !!!
CFLAGS+= -Wall -Wstrict-prototypes 
CFLAGS+=$(DEBFLAGS)

# if kbuild system is avalible we use its syntax
ifneq ($(KERNELRELEASE),) 
	# main kREproxy module objects
	obj-m   		:= $(MODULENAME).o

	# all kREproxy module's objects 
	$(MODULENAME)-objs	:= kre_options.o kre_debug.o kre_list.o kre_config.o kre_threads.o kre_network.o kre_utils.o kre_http.o kre_main.o

# if kbuild not avalible
else

# some tricks for compilation in local directory
KDIR    		:= /lib/modules/$(shell uname -r)/build
PWD     		:= $(shell pwd)

default: 
	$(MAKE) -C $(KDIR) M=$(PWD) modules

install:
#	$(MAKE) -C $(KDIR) M=$(PWD) modules install

uninstall:
	
clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean

start:
	@echo -ne "Starting server ..."
	@/sbin/insmod ./$(MODULENAME).ko
	@echo -ne "[ OK ]\n"

stop:
	@echo -ne "Stoping kREproxy server\n\n"
#	@echo -ne "You must send one request to stop server now ...\n"
#	@echo -ne "(The simplest way to do this is send this from your web browser)\n\n"
	@/bin/sleep 2 && echo "x" > /dev/tcp/localhost/80 &
	@/sbin/rmmod $(MODULENAME)
	@echo -ne "Thank you, for using kREproxy\n"

#show:
#	@echo -n $(KERNELRELEASE)

about:
	@echo -e "make [clean|start|stop]"
	@echo -e ""
	@echo -e "kREproxy project (C) Marcin Tomasik 2009"
	@echo -e "http://kreproxy.sourceforge.net/"

.PHONY: default install uninstall clean start stop show about

endif
