PROG := generate_tetris_ai
SRCS := main.c tetris.c ga.c
OBJS := $(SRCS:%.c=%.o)
DEPS := $(SRCS:%.c=%.d)

CC := gcc

all: $(PROG)

-include $(DEPS)

$(PROG): $(OBJS)
	$(CC) -o $@ $^

%.o: %.c
	$(CC) -c -Wall -g -MMD -MP $<

clean:
	rm -f $(PROG) $(OBJS) $(DEPS)