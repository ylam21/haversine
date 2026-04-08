NAME_GEN    := generate_json
NAME_HAVER  := haversine
CC          := gcc

SRC_GEN     := src/generator/generator_main.c
SRC_HAVER   := src/main_haversine.c

CFLAGS      := -Wall -Wextra -MMD -MP
DBG_FLAGS   := -O0 -g -fsanitize=address -DDEBUG

all: $(NAME_GEN) $(NAME_HAVER)

debug: $(NAME_GEN)_debug $(NAME_HAVER)_debug

$(NAME_GEN): $(SRC_GEN)
	$(CC) $(CFLAGS) $< -o $@ -lm

$(NAME_GEN)_debug: $(SRC_GEN)
	$(CC) $(CFLAGS) $(DBG_FLAGS) $< -o $@ -lm

$(NAME_HAVER): $(SRC_HAVER)
	$(CC) $(CFLAGS) $< -o $@ -lm

$(NAME_HAVER)_debug: $(SRC_HAVER)
	$(CC) $(CFLAGS) $(DBG_FLAGS) $< -o $@ -lm

-include $(NAME_GEN).d
-include $(NAME_HAVER).d
-include $(NAME_GEN)_debug.d
-include $(NAME_HAVER)_debug.d

clean:
	rm -rf $(NAME_GEN) $(NAME_GEN)_debug answers.f64 data.json
	rm -rf $(NAME_HAVER) $(NAME_HAVER)_debug
	rm -rf *.d

re: clean all

.PHONY: all debug clean re
