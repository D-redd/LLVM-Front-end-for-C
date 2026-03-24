# Usage
#
# make       - build the application
# make test  - test the application
# make clean - remove generated files

NAME=compiler
CPP_FILES=compiler.cpp
HDR_FILES=$(CPP_FILES:.cpp=.h)

export CFLAGS=`llvm-config --cxxflags`
export LDFLAGS=`llvm-config --cxxflags --ldflags --libs core analysis bitwriter --system-libs`

$(NAME): $(CPP_FILES) $(HDR_FILES)
	g++ -o $@ -g $(CPP_FILES) $(LDFLAGS)
	
clean:
	rm -f $(NAME) $(CPP_FILES:.cpp=.o)
