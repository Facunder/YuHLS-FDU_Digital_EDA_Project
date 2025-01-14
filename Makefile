# Compiler and linker settings
CXX = g++
CFLAGS = -g -DDEBUG -W -Wall -fPIC
LDFLAGS =
LIBS =

# Source files and object files
SRCDIR = src
BUILDDIR = build
SRCS = $(wildcard $(SRCDIR)/*.cpp)
HDRS = $(wildcard $(SRCDIR)/*.h)
OBJS = $(SRCS:.cpp=.o)
TARGET = YuHLS.exe

# Rules
all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(LDFLAGS) -o $@ $^ $(LIBS)

%.o: %.cpp $(HDRS)
	$(CXX) $(CFLAGS) -c -o $@ $<

clean:
	del /Q /F $(TARGET) $(subst /,\,$(OBJS))
