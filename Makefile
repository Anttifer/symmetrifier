# These can be modified.
NAME     := supersymmetry
SRCDIR   := src
OBJDIR   := obj
BINDIR   := bin
DEPDIR   := dep

# These can be modified too.
CXX      := g++
CPPFLAGS := -DGLEW_STATIC
CXXFLAGS := -std=c++14 -O3 -Wall -Wextra -Wshadow -pedantic
LDFLAGS  := -L/usr/local/lib
LDLIBS   := -lGLEW -lglfw3 -framework Cocoa -framework IOKit -framework CoreVideo -framework OpenGL
INCL     := -Iinclude -I/usr/local/include -I/usr/local/include/eigen3

# These should not be modified.
BIN  := $(BINDIR)/$(NAME)
SRCS := $(wildcard $(SRCDIR)/*.cpp)
OBJS := $(SRCS:$(SRCDIR)/%.cpp=$(OBJDIR)/%.o)
DEPS := $(SRCS:$(SRCDIR)/%.cpp=$(DEPDIR)/%.d)
CPPFLAGS += -MMD -MP

.PHONY: clean run


# Default target.
$(BIN): $(OBJS) | $(BINDIR)
	$(CXX) $(LDFLAGS) -o $@ $^ $(LDLIBS)

# Handle dependencies.
$(OBJDIR)/%.o: $(SRCDIR)/%.cpp
	$(CXX) $(CPPFLAGS) $(INCL) -MT $@ -MF $(DEPDIR)/$*.d $(CXXFLAGS) -c -o $@ $<

# Create the necessary directories if they don't exist.
$(OBJS): | $(OBJDIR) $(DEPDIR)

$(BINDIR) $(OBJDIR) $(DEPDIR):
	mkdir $@

clean:
	$(RM) $(BIN) $(OBJS) $(DEPS)
	rmdir $(BINDIR) $(OBJDIR) $(DEPDIR)

run: $(BIN)
	$(BINDIR)/$(NAME)

-include $(DEPS)
