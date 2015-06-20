
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

all: summarizer_main

summarizer_main: summarizer_main.cc Summarizer.cc Summarizer.h LexicalChain.h LexicalChain.cc Relation.h Relation.cc
	g++ -std=c++0x summarizer_main.cc Summarizer.cc LexicalChain.cc Relation.cc -lfreeling -lboost_system -o summarizer_main $(PATHS)

clean:
	rm -f summarizer_main *~
