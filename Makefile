NAME        := generate_json
CC          := gcc

SRC         := src/main.c

CFLAGS      := -Wall -Wextra -MMD -MP
DBG_FLAGS   := -O0 -g -fsanitize=address -DDEBUG

all: $(NAME)
debug: $(NAME)_debug

$(NAME): $(SRC)
	$(CC) $(CFLAGS) $< -o $@ -lm

$(NAME)_debug: $(SRC)
	$(CC) $(CFLAGS) $(DBG_FLAGS) $< -o $@

-include $(NAME).d
-include $(NAME)_debug.d

clean:
	rm -rf $(NAME) $(NAME)_debug
	rm -rf *.d

re: clean all

.PHONY: all debug clean re