#
# name: Kaylie Pham
# RedID: 828129478
#
# name: Aditya Bhagat
# RedID: 828612974

# Compiler and flags
CXX = g++
# -std=c++11  C/C++ variant to use, e.g. C++ 2011
# -Wall       show the necessary warning files
# -g3         include information for symbolic debugger e.g. gdb
CXXFLAGS = -std=c++11 -Wall -g3

# object files 
OBJS = log_helpers.o page_table.o WSClock.o vaddr_tracereader.o pagingwithwsclock.o

# executable name
PROGRAM = pagingwithwsclock

# default target
all: $(PROGRAM)

# Rules format:
# target : dependency1 dependency2 ... dependencyN
#     Command to make target, uses default rules if not specified

# First target is the one executed if you just type make
# make target specifies a specific target
# $^ is an example of a special variable.  It substitutes all dependencies
$(PROGRAM): $(OBJS)
	$(CXX) -o $(PROGRAM) $(OBJS)

# Individual compilation rules
log_helpers.o: log_helpers.c log_helpers.h
	$(CXX) $(CXXFLAGS) -c log_helpers.c

PageTable.o: page_table.cpp page_table.h
	$(CXX) $(CXXFLAGS) -c page_table.cpp

WSClock.o: WSClock.cpp WSClock.h
	$(CXX) $(CXXFLAGS) -c WSClock.cpp

vaddr_tracereader.o: vaddr_tracereader.c vaddr_tracereader.h
	$(CXX) $(CXXFLAGS) -c vaddr_tracereader.c

pagingwithwsclock.o: pagingwithwsclock.cpp page_table.h WSClock.h log_helpers.h vaddr_tracereader.h
	$(CXX) $(CXXFLAGS) -c pagingwithwsclock.cpp

# Clean rule
clean:
	rm -f *.o $(PROGRAM)