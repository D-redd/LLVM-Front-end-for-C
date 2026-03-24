# Usage
#
# make               - same as 'make source'
# make source        - build the application
# make test1         - test the application (Step 1 - Expressions)
# make test2         - test the application (Step 2 - Arguments)
# make test3         - test the application (Step 3 - Local variables)
# make test4         - test the application (Step 4 - Function calls)
# make clean         - remove generated files

all: source

source:
	make -C src

test1: source
	make -C test

test2: source
	make -C test TEST_ARGS=1

test3: source
	make -C test TEST_ARGS=1 TEST_LOCALS=1

test4: source
	make -C test TEST_ARGS=1 TEST_LOCALS=1 TEST_CALLS=1

clean:
	make -C src clean
	make -C test clean
