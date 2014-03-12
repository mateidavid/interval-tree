CPPFLAGS=-I${BOOST_INTRUSIVE}/include -I ../include -I${BOOST}/include
CXXFLAGS=-std=c++0x -Wall -Wextra -Wno-unused-local-typedefs -Wno-ignored-qualifiers -g -O0
LDFLAGS=-L${BOOST}/lib -Wl,--rpath=${BOOST}/lib -lboost_program_options

.PHONY: all clean

all: test-itree

test-itree: test-itree.cpp \
	${BOOST_INTRUSIVE}/include/boost/intrusive/bstree.hpp \
	${BOOST}/include/boost/program_options.hpp \
	${BOOST}/lib/libboost_program_options.so
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(LDFLAGS) -o $@ $<

clean:
	rm test-itree
