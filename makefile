target:bin/main
SRCS := $(wildcard src/*.c)
OBJS := $(patsubst src/%.c,obj/%.o,$(SRCS))
BUILD_TYPE ?= debug
CFLAGS= #-D_DEBUG    #  -fsanitize=address,undefined -g  
LINK= # -fsanitize=address,undefined -g
ifeq  ($(BUILD_TYPE),debug)
	CFLAGS += -D_DEBUG  -fsanitize=address,undefined -g
	LINK += -fsanitize=address,undefined -g
else ifeq ($(BUILD_TYPE),release)
	
endif

# 创建目录
obj bin:
	mkdir -p $@

obj/%.o: src/%.c | obj
	gcc -c $^ -o $@   $(CFLAGS)

bin/main: $(OBJS) | bin
	gcc $(OBJS) -o $@ $(LINK)

clean:
	rm -rf obj bin
run:bin/main
	./bin/main
.PHONY: clean run  target
