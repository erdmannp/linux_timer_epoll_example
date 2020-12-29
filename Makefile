src = $(wildcard src/*.c)
obj = $(src:.c=.o)

LDFLAGS = -lrt

timer_epoll_example: $(obj)
	$(CC) -o $@ $^ $(LDFLAGS)

.PHONY: clean
clean:
	rm -f $(obj) timer_epoll_example

