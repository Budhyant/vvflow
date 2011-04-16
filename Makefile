#‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾#
#                 VVHD makefile                  #
#       to understand what's written here        #
#  read http://www.opennet.ru/docs/RUS/gnumake/  #
#                                                #
#  (c) Rosik                                     #
#________________________________________________#

#
# Usage:
# make [optimization="-O3 -pg"] [warnings="-Wall"]
# make [INSTALLDIR="~/.libVVHDinstall"] install
#

#‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾#
#                   VARIABLES                    #
#________________________________________________#

CC		= gcc

parts 	:= core modules
core_objects 	:= elementary space tree body
modules_objects := convectivefast flowmove diffmergefast merge objectinfluence
VPATH := $(addprefix source/, $(parts) ) 
# VPATH is special make var 

optimization 	:= -O3 -g
warnings 		:= -Wall
INCLUDE 		:= headers/

INSTALLDIR 		:= ~/.libVVHDinstall

AR		= ar
ifeq ($(CC),icc)
  AR = xiar
endif

#‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾#
#                    TARGETS                     #
#________________________________________________#


all: $(patsubst %, bin/libVVHD%.a, $(parts))

clean:
	rm -rf bin

install: | $(INSTALLDIR)/lib/ $(INSTALLDIR)/include/
	cp $(patsubst %, bin/libVVHD%.a, $(parts)) -t $(INSTALLDIR)/lib/
	cp headers/*.h -t $(INSTALLDIR)/include/

uninstall:
	rm -f $(patsubst %, $(INSTALLDIR)/lib/libVVHD%.a, $(parts))
	rm -f $(patsubst headers/%, $(INSTALLDIR)/include/%, $(wildcard headers/*.h))
	rmdir $(INSTALLDIR)/lib/ $(INSTALLDIR)/include/ $(INSTALLDIR)/ --ignore-fail-on-non-empty

#‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾#
#                     RULES                      #
#________________________________________________#

bin/%.o: %.cpp headers/%.h | bin/
	$(CC) -xc++ $< $(optimization) $(warnings) $(addprefix -I, $(INCLUDE)) -c -o $@

bin/%.o: %.c headers/%.h | bin/
	$(CC) -xc   $< $(optimization) $(warnings) $(addprefix -I, $(INCLUDE)) -c -o $@

bin/%.o: %.f95 | bin/
	$(CC) -xf95 $< $(optimization) $(warnings) -c -o $@

bin/libVVHDcore.a: $(patsubst %, bin/%.o, $(core_objects))
bin/libVVHDmodules.a: $(patsubst %, bin/%.o, $(modules_objects))
bin/libVVHD%.a:
	$(AR) ruc $@ $^
	ranlib $@

bin/:
	mkdir $@ -p

$(INSTALLDIR)/%:
	mkdir $@ -p

