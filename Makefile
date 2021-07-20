PROJECT := GenRun
SRCDIR = Src
OBJDIR = Int
CC := gcc
CFLAGS := -std=c99 -O2 -Wall -Wextra -Wcast-align -Wlogical-op -Wmissing-include-dirs -Wredundant-decls -Wundef -fmessage-length=0

OBJS := $(patsubst $(SRCDIR)%.c, $(OBJDIR)%.o, $(wildcard $(SRCDIR)/*.c))

POSTCMD := ./GenRun.exe --name $(PROJECT) -e --output=rungen.bat
deps := $(patsubst, %.o, %.d, $(OBJS))
-include $(deps)
DEPFLAGS = -MMD -MF $(@:.o=.d)

.PHONY: dirs clean

all: dirs $(PROJECT).exe

$(PROJECT).exe: $(OBJS)
	@echo Linking
	$(CC) -o $@ $^
	@echo Post Build Command
	$(POSTCMD)

$(OBJDIR)/%.o: $(SRCDIR)/%.c
	@echo Compiling $<
	$(CC) $(CFLAGS) -E -w -o $(@:.o=_e.c) $< $(DEPFLAGS)
	$(CC) $(CFLAGS) -S -w -o $(@:.o=.s) $< $(DEPFLAGS)
	$(CC) $(CFLAGS) -c $< -o $@ $(DEPFLAGS)


dirs: $(OBJDIR)

$(OBJDIR):
	mkdir -p $(OBJDIR)

clean: 
	rm -f $(OBJDIR)/*_e.c $(OBJDIR)/*.s $(OBJDIR)/*.o $(PROJECT).exe 