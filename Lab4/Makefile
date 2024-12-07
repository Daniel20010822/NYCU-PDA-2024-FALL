.PHONY: all clean

# Compiler settings
CXX = g++
CXXFLAGS = -I ./inc -std=c++11
DEBUGFLAGS = #-DENABLE_DEBUG_D2DGR -DENABLE_DEBUG_CHIP -DENABLE_DEBUG_COST
WARNINGS = -g -Wall
PYTHON = python3.12

# Source files and object files
SRCDIR = src
OBJDIR = obj
INCDIR = inc
SRCS = $(wildcard $(SRCDIR)/*.cpp)
OBJS = $(patsubst $(SRCDIR)/%.cpp,$(OBJDIR)/%.o,$(SRCS))

# Control the build verbosity
ifeq ("$(VERBOSE)","1")
    Q :=
    VECHO = @true
else
    Q := @
    VECHO = @printf
endif

# Name of the executable
TARGET = D2DGRter

all: $(TARGET)

$(OBJDIR):
	@mkdir $(OBJDIR)

$(TARGET): main.cpp $(OBJS)
	$(VECHO) "	LD\t$@\n"
	$(Q) $(CXX) $(WARNINGS) $(CXXFLAGS) $(DEBUGFLAGS) $^ -o $@

# Compilation rule for object files with automatic dependency generation
$(OBJDIR)/%.o: $(SRCDIR)/%.cpp | $(OBJDIR) Makefile
	$(VECHO) "	CC\t$@\n"
	$(Q) $(CXX) $(WARNINGS) $(CXXFLAGS) $(DEBUGFLAGS) -c $< -o $@

clean:
	rm -rf $(OBJDIR) $(TARGET) ./publicCase/testcase0/*.lg ./publicCase/testcase1/*.lg ./publicCase/testcase2/*.lg


EVALUATOR := NYCU-PDA-Lab4-Evaluator/Evaluator/Evaluator

run0:
	time ./$(TARGET) publicCase/testcase0/testcase0.gmp publicCase/testcase0/testcase0.gcl publicCase/testcase0/testcase0.cst publicCase/testcase0/testcase0.lg
	$(EVALUATOR) ./publicCase/testcase0 testcase0
	$(PYTHON) draw/draw.py ./publicCase/testcase0 testcase0
run1:
	time ./$(TARGET) publicCase/testcase1/testcase1.gmp publicCase/testcase1/testcase1.gcl publicCase/testcase1/testcase1.cst publicCase/testcase1/testcase1.lg
	$(EVALUATOR) ./publicCase/testcase1 testcase1
	$(PYTHON) draw/draw.py ./publicCase/testcase1 testcase1

run2:
	time ./$(TARGET) publicCase/testcase2/testcase2.gmp publicCase/testcase2/testcase2.gcl publicCase/testcase2/testcase2.cst publicCase/testcase2/testcase2.lg
	$(EVALUATOR) ./publicCase/testcase2 testcase2
	$(PYTHON) draw/draw.py ./publicCase/testcase2 testcase2

runall: run0 run1 run2

