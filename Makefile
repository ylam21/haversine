NAME_GEN    := generate_json
NAME_PARSER := parse_json
CC          := gcc

SRC_GEN     := src/generator/generator_main.c
SRC_PARSER  := src/parser/parser_main.c

CFLAGS      := -Wall -Wextra -MMD -MP
DBG_FLAGS   := -O0 -g -fsanitize=address -DDEBUG

all: $(NAME_GEN) $(NAME_PARSER)

debug: $(NAME_GEN)_debug $(NAME_PARSER)_debug

$(NAME_GEN): $(SRC_GEN)
	$(CC) $(CFLAGS) $< -o $@ -lm

$(NAME_GEN)_debug: $(SRC_GEN)
	$(CC) $(CFLAGS) $(DBG_FLAGS) $< -o $@

$(NAME_PARSER): $(SRC_PARSER)
	$(CC) $(CFLAGS) $< -o $@ -lm

$(NAME_PARSER)_debug: $(SRC_PARSER)
	$(CC) $(CFLAGS) $(DBG_FLAGS) $< -o $@

-include $(NAME_GEN).d
-include $(NAME_PARSER).d
-include $(NAME_GEN)_debug.d
-include $(NAME_PARSER)_debug.d

clean:
	rm -rf $(NAME_GEN) $(NAME_GEN)_debug
	rm -rf $(NAME_PARSER) $(NAME_PARSER)_debug
	rm -rf *.d

re: clean all

.PHONY: all debug clean re