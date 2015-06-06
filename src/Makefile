
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

all: doc_to_conll print_with_span

doc_to_conll: doc_to_conll.cc
	g++ -std=c++0x doc_to_conll.cc -lfreeling -lboost_system -o doc_to_conll $(PATHS)

print_with_span: print_with_span.cc
	g++ -std=c++0x print_with_span.cc -lfreeling -lboost_system -o print_with_span $(PATHS)

clean:
	rm -f doc_to_conll print_with_span