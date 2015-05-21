
# Makefile to build the simple examples of FreeLing usage.
# If your installation paths are not /usr/local, call make with:
#
# make FREELINGDIR=/my/freeling/dir BOOSTINCLUDEDIR=/my/boost/include BOOSTLIBDIR=/my/boost/lib


# directories where freeling and boost are installed
FREELINGDIR=/usr/local
BOOSTINCLUDEDIR=/usr/local/include
BOOSTLIBDIR=/usr/local/lib
FREELINGSHARE=/usr/local/share/freeling

# paths for compilation commands below
PATHS=-L$(FREELINGDIR)/lib -I$(FREELINGDIR)/include -L$(BOOSTLIBDIR) -I$(BOOSTINCLUDEDIR)

all: tests

tests: tests.cc
	g++ -std=c++0x *.cc -o tests -lboost_unit_test_framework -lfreeling -lboost_system $(PATHS)

clean:
	rm -f tests