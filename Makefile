##########TEMPLATE##########

TARGET=interpretador_rr escalonador_rr interpretador_rt escalonador_rt prog_cpu prog_io P1 P2 P3 P4 P5 P6
DOCS=Relatorio.pdf Relatorio-LAB.pdf

BINDIR=bin
SRCDIR=src
OBJDIR=obj
DEPDIR=$(OBJDIR)/dep

CC=gcc
EXT=.c

CCFLAGS=-O0 -g -Wall -Wextra -pedantic
LDFLAGS=-lrt -pthread

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

docs: $(DOCS)

Relatorio.pdf: README.md
	pandoc -V geometry:margin=1in $^ -o $@

Relatorio-LAB.pdf: LAB.md
	pandoc -V geometry:margin=1in $^ -o $@

$(BINDIR)/escalonador_rr: $(OBJDIR)/escalonador_rr.o $(OBJDIR)/queue.o $(OBJDIR)/slist.o
$(BINDIR)/interpretador_rr: $(OBJDIR)/interpretador_rr.o
$(BINDIR)/escalonador_rt: $(OBJDIR)/escalonador_rt.o $(OBJDIR)/queue.o $(OBJDIR)/slist.o
$(BINDIR)/interpretador_rt: $(OBJDIR)/interpretador_rt.o
$(BINDIR)/prog_cpu: $(OBJDIR)/prog_cpu.o
$(BINDIR)/prog_io: $(OBJDIR)/prog_io.o
$(BINDIR)/P1: $(OBJDIR)/prog_cpu.o
$(BINDIR)/P2: $(OBJDIR)/prog_cpu.o
$(BINDIR)/P3: $(OBJDIR)/prog_io.o
$(BINDIR)/P4: $(OBJDIR)/prog_cpu.o
$(BINDIR)/P5: $(OBJDIR)/prog_cpu.o
$(BINDIR)/P6: $(OBJDIR)/prog_cpu.o

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
	$(RM_R) $(DEPDIR) $(OBJDIR) $(TARGETDIR) $(BINDIR) $(DOCS)
