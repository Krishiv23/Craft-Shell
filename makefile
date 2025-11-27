CC = clang
CFLAGS = -Wall -Wextra -g

SRC = main.c prompt.c lexer.c parser.c execution_engine.c buildins.c
DEPS = prompt.h lexer.h parser.h execution_engine.h buildins.h
OBJ = $(SRC:.c=.o)
TGT = craft_shell

%.o: %.c $(DEPS)
	$(CC) $(CFLAGS) -c -o $@ $<

$(TGT) : $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^

.PHONY : clean
clean:
	@rm -f $(OBJ) $(TGT)