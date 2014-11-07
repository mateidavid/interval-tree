BOOST_INCLUDE = /usr/include
BOOST_LIB = /usr/lib/x86_64-linux-gnu
BOOST_PROGRAM_OPTIONS_LIB = ${BOOST_LIB}/libboost_program_options.so

CPPFLAGS = -I${BOOST_INTRUSIVE}/include -I ../include -I${BOOST_INCLUDE}
CXXFLAGS = -std=c++11 -Wall -Wextra -pedantic -g3 -O0
LDFLAGS = -L${BOOST_LIB} -Wl,--rpath=${BOOST_LIB} ${BOOST_PROGRAM_OPTIONS_LIB}

.PHONY: all clean check-env

all: check-env test-itree

clean:
	rm -f test-itree

check-env:
	@[ -d ${BOOST_INTRUSIVE}/include ] || { echo "BOOST_INTRUSIVE not found: ${BOOST_INTRUSIVE}/include"; exit 1; }
	@[ -d ${BOOST_INCLUDE} ] || { echo "BOOST_INCLUDE not found: ${BOOST_INCLUDE}"; exit 1; }

test-itree: test-itree.cpp \
	${BOOST_INTRUSIVE}/include/boost/intrusive/bstree.hpp \
	${BOOST_INCLUDE}/boost/program_options.hpp \
	${BOOST_PROGRAM_OPTIONS_LIB}
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -o $@ $< $(LDFLAGS)
