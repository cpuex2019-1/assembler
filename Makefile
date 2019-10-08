CXX = g++
PROG = onabe
CXXFLAGS = -Wall -O2 -std=c++11

SRCS :=  loader.cpp controller.cpp main.cpp
OBJS := $(SRCS:%.cpp=%.o)


$(PROG): $(OBJS)
	$(CXX) -o $(PROG) $(OBJS)



clean:
	rm -f $(PROG) $(OBJS)
