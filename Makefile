NAME_GEN    := generate_json
NAME_HAVER  := haversine
NAME_REPTEST:= reptest
CC          := gcc

SRC_GEN     := src/generator/generator_main.c
SRC_HAVER   := src/main_haversine.c
SRC_REPTEST := src/reptest/reptest_main.c

CFLAGS      := -Wall -Wextra -MMD -MP
DBG_FLAGS   := -O0 -g -fsanitize=address -DDEBUG
DFLAGS_PROF := -DENABLE_PROFILER

all: $(NAME_GEN) $(NAME_HAVER) $(NAME_REPTEST)

reptest: $(NAME_REPTEST)

debug: $(NAME_GEN)_debug $(NAME_HAVER)_debug

prof: $(NAME_GEN)_prof $(NAME_HAVER)_prof

# Generator_json binary
$(NAME_GEN): $(SRC_GEN) # Profiler OFF
	$(CC) $(CFLAGS) $< -o $@ -lm

$(NAME_GEN)_debug: $(SRC_GEN)
	$(CC) $(CFLAGS) $(DBG_FLAGS) $< -o $@ -lm

$(NAME_GEN)_prof: $(SRC_GEN)
	$(CC) $(CFLAGS) $(DFLAGS_PROF)=1 $< -o $@ -lm

# Haversine binary
$(NAME_HAVER): $(SRC_HAVER) # Profiler OFF
	$(CC) $(CFLAGS) $< -o $@ -lm

$(NAME_HAVER)_debug: $(SRC_HAVER)
	$(CC) $(CFLAGS) $(DBG_FLAGS) $< -o $@ -lm

$(NAME_HAVER)_prof: $(SRC_HAVER)
	$(CC) $(CFLAGS) $(DFLAGS_PROF)=1 $< -o $@ -lm

# Repetition Tester Binary
$(NAME_REPTEST): $(SRC_REPTEST)
	$(CC) $(CFLAGS) $< -o $@ -lm

-include $(NAME_GEN).d
-include $(NAME_HAVER).d
-include $(NAME_REPTEST).d
-include $(NAME_GEN)_debug.d
-include $(NAME_HAVER)_debug.d
-include $(NAME_GEN)_prof.d
-include $(NAME_HAVER)_prof.d

clean:
	rm -rf $(NAME_GEN) $(NAME_GEN)_debug $(NAME_GEN)_prof
	rm -rf $(NAME_HAVER) $(NAME_HAVER)_debug $(NAME_HAVER)_prof
	rm -rf $(NAME_REPTEST)
	rm -rf *.d

cleanf:
	rm -rf data.json answers.f64

re: clean cleanf all

.PHONY: all debug prof clean re cleanf
