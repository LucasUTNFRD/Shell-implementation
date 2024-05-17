CFLAGS := -g -Wall -Wextra
CC := gcc
EXEC = sh
SRC = $(wildcard *.c)
OBJ = $(patsubst %.c, $(OBJDIR)/%.o, $(SRC))
OBJDIR = build

$(OBJDIR)/%.o: %.c
	@mkdir -p $(OBJDIR)
	$(CC) $(CFLAGS) -c $< -o $@

all: $(OBJDIR)/$(EXEC)

$(OBJDIR)/$(EXEC): $(OBJ)
	$(CC) $(CFLAGS) $^ -o $@

run: all
	./$(OBJDIR)/$(EXEC)

clean:
	rm -rf $(OBJDIR) *.o *.asm

.PHONY: clean run

