##########TEMPLATE##########

TARGET=interpretador_rt interpretador_rr escalonador_rt escalonador_rr prog_io prog_cpu

BINDIR=bin
SRCDIR=src
OBJDIR=obj
DEPDIR=$(OBJDIR)/dep

CC=gcc
EXT=.c

CCFLAGS=-O0 -g -Wall -Wextra -pedantic -ansi
LDFLAGS=

##########AUTO##########

SRC=$(wildcard $(SRCDIR)/*$(EXT))
OBJ=$(SRC:$(SRCDIR)/%$(EXT)=$(OBJDIR)/%.o)
DEP=$(OBJ:$(OBJDIR)/%.o=$(DEPDIR)/%.d)
BIN=$(addprefix $(BINDIR)/, $(TARGET))

##########COMMANDS#########

MKDIR_P = mkdir -p
RM_R = rm -r

##########TARGETS##########

.PHONY: all clean

all: $(BIN)

# $(BINDIR)/interpretador_rt: $(OBJDIR)/main.o

$(BIN): | $(BINDIR)
	$(CC) -o $@ $^ $(LDFLAGS)

$(DEPDIR)/%.d: $(SRCDIR)/%$(EXT) | $(DEPDIR)
	@$(CC) $< -MM -MT $(@:$(DEPDIR)/%.d=$(OBJDIR)/%.o) >$@

-include $(DEP)

$(OBJDIR)/%.o: $(SRCDIR)/%$(EXT) | $(OBJDIR)
	$(CC) $(CCFLAGS) -o $@ -c $<

$(OBJDIR) $(DEPDIR) $(BINDIR):
	$(MKDIR_P) $@

##########CLEAN##########

clean:
	$(RM_R) $(DEPDIR) $(OBJDIR) $(TARGETDIR) $(BINDIR)
