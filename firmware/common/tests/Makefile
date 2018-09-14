#
# Temporary Makefile for our Unit tests. This should be in cmake, soon.
#


TARGET=test_runner
TESTS = \
	test_debug.o

OBJS = \
	../debug.o

COMMON_FLAGS = \
	-D__RUNNING_ON_HOST__ \
	-D__USE_SYSTEM_HEADERS__ \
	-Iinclude \
	-I.. \
	-fno-stack-protector \
	-fno-common \
	-fno-builtin \
	-Werror \
	-Wall

CFLAGS = \
	$(COMMON_FLAGS) \
	-std=gnu99 \

CXXFLAGS = \
	-std=c++11 \
	$(COMMON_FLAGS)

LDFLAGS =

all: $(TARGET)

run_tests: $(TARGET)
	./test_runner

$(TARGET): $(TARGET).o $(OBJS) $(TESTS)
	$(CXX) $(CXXFLAGS) $^ -o $@

%.o: %.S
	$(CC) $(CFLAGS) $< -c -o $@

%.o: %.c
	$(CC) $(CFLAGS) $< -c -o $@

%.o: %.c
	$(CC) $(CFLAGS) $< -c -o $@

%.o: %.cpp
	$(CXX) $(CXXFLAGS) $< -c -o $@

clean:
	rm -f *.o $(TARGET) $(TARGET).bin $(TARGET).elf $(TARGET).fit

.PHONY: all clean run_tests
