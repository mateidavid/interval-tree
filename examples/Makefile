BOOST_INCLUDE = /usr/include
BOOST_LIB = /usr/lib/x86_64-linux-gnu

CPPFLAGS = -I${BOOST_INTRUSIVE}/include -I ../include -I${BOOST_INCLUDE}
CXXFLAGS = -std=c++11 -Wall -Wextra -pedantic -g3 -O0
LDFLAGS = -L${BOOST_LIB} -Wl,--rpath=${BOOST_LIB} -lboost_program_options

.PHONY: all clean check-env

all: check-env test-itree

clean:
	rm test-itree

check-env:
	@[ -d ${BOOST_INTRUSIVE}/include ] || { echo "BOOST_INTRUSIVE not found: ${BOOST_INTRUSIVE}/include"; exit 1; }
	@[ -d ${BOOST_INCLUDE} ] || { echo "BOOST_INCLUDE not found: ${BOOST_INCLUDE}"; exit 1; }

test-itree: test-itree.cpp \
	${BOOST_INTRUSIVE}/include/boost/intrusive/bstree.hpp \
	${BOOST_INCLUDE}/boost/program_options.hpp \
	${BOOST_LIB}/libboost_program_options.so
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(LDFLAGS) -o $@ $<
