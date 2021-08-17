CC=gcc

CWARNINGS=-Wall -Wextra -Wpedantic -Wformat=2 -Wswitch-default -Wpointer-arith -Wbad-function-cast -Wstrict-overflow=5 -Wstrict-prototypes -Winline -Wundef -Wnested-externs -Wcast-qual -Wshadow -Wunreachable-code -Wlogical-op -Wfloat-equal -Wstrict-aliasing=2 -Wredundant-decls -Wold-style-definition 

COPTIMISING=-O0 -ggdb3 -fno-omit-frame-pointer -ffloat-store -fno-common -fstrict-aliasing -g

CFLAGS=-std=c11 $(CWARNINGS) $(COPTIMISING) -lm

INSTALL_DIR=$(HOME)/psu/bin

# Auxillary Variables

RED=\033[0;31m
BLUE=\033[0;34m
NC=\033[0m


all: main.o 
	@echo "--------------------------------------------------"
	@echo -e "$(RED)LINKING$(NC): Producing Executable"
	@$(CC) $(CFLAGS) main.o utils.o terminal.o globals.o editor.o -o aurum
	@echo "--------------------------------------------------"

main.o: main.c utils.o terminal.o globals.o editor.o
	@echo "--------------------------------------------------"
	@echo -e "$(RED)COMPILING$(NC): $(BLUE)main.c$(NC)"
	@$(CC) $(CFLAGS) -c main.c 

terminal.o: terminal.c terminal.h utils.o
	@echo "--------------------------------------------------"
	@echo -e "$(RED)COMPILING$(NC): $(BLUE)terminal.c$(NC)"
	@$(CC) $(CFLAGS) -c terminal.c 

utils.o: utils.c utils.h
	@echo "--------------------------------------------------"
	@echo -e "$(RED)COMPILING$(NC): $(BLUE)utils.c$(NC)"
	@$(CC) $(CFLAGS) -c utils.c 

globals.o: globals.h globals.c utils.o
	@echo "--------------------------------------------------"
	@echo -e "$(RED)COMPILING$(NC): $(BLUE)globals.c$(NC)"
	@$(CC) $(CFLAGS) -c globals.c 

editor.o: editor.h editor.c globals.o utils.o
	@echo "--------------------------------------------------"
	@echo -e "$(RED)COMPILING$(NC): $(BLUE)editor.c$(NC)"
	@$(CC) $(CFLAGS) -c editor.c 

clean:
	rm -f editor.o terminal.o utils.o main.o globals.o aurum
