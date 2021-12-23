###############################################################################
#                                                                             #
# MAKEFILE for xword                                                          #
#                                                                             #
# (c) Guy Wilson 2021                                                         #
#                                                                             #
###############################################################################

# Directories
SOURCE = src
BUILD = build
DEP = dep

# What is our target
TARGET = xword

# Tools
CPP = g++-11
LINKER = g++-11

# postcompile step
PRECOMPILE = @ mkdir -p $(BUILD) $(DEP)
# postcompile step
POSTCOMPILE = @ mv -f $(DEP)/$*.Td $(DEP)/$*.d

CPPFLAGS = -c -O2 -Wall -pedantic -std=c++11
DEPFLAGS = -MT $@ -MMD -MP -MF $(DEP)/$*.Td

# Libraries
STDLIBS = -lstdc++

COMPILE.cpp = $(CPP) $(CPPFLAGS) $(DEPFLAGS) -o $@
LINK.o = $(LINKER) $(STDLIBS) -o $@

CPPSRCFILES = $(wildcard $(SOURCE)/*.cpp)
OBJFILES = $(patsubst $(SOURCE)/%.cpp, $(BUILD)/%.o, $(CPPSRCFILES))
DEPFILES = $(patsubst $(SOURCE)/%.cpp, $(DEP)/%.d, $(CPPSRCFILES))

all: $(TARGET)

# Compile C/C++ source files
#
$(TARGET): $(OBJFILES)
	$(LINK.o) $^ $(EXTLIBS)

$(BUILD)/%.o: $(SOURCE)/%.cpp
$(BUILD)/%.o: $(SOURCE)/%.cpp $(DEP)/%.d
	$(PRECOMPILE)
	$(COMPILE.cpp) $<
	$(POSTCOMPILE)

.PRECIOUS = $(DEP)/%.d
$(DEP)/%.d: ;

-include $(DEPFILES)

clean:
	rm -r $(BUILD)
	rm -r $(DEP)
	rm $(TARGET)
