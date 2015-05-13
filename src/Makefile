
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

all: summarizer_test

summarizer_test: summarizer_test.cc summarizer.cc summarizer.h lexical_chain.h lexical_chain.cc relation.h relation.cc
	g++ -std=c++0x summarizer_test.cc summarizer.cc lexical_chain.cc relation.cc -lfreeling -lboost_system -o summarizer_test $(PATHS)

clean:
	rm -f summarizer_test