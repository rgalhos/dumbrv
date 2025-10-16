CC = gcc

CFLAGS := -pedantic
CFLAGS += -Wall -Wextra -fPIC
CFLAGS += -std=gnu23

CFLAGS += -fanalyzer
CFLAGS += -fsanitize=address -fsanitize=undefined -fno-sanitize-recover=all
CFLAGS += -fsanitize=float-divide-by-zero -fsanitize=float-cast-overflow
CFLAGS += -fsanitize=null -fsanitize=alignment

INCLUDES := 
LIBS := -L./lib

SRC_DIR = src
OBJ_DIR = build
OUT_DIR = build
TESTS_DIR = test
TESTS_OUT_DIR = build/test
ENTRYPOINT = $(SRC_DIR)/main.c

SRCS := $(filter-out $(ENTRYPOINT) , $(wildcard $(SRC_DIR)/*.c))
OBJS := $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SRCS))
TESTS = $(wildcard $(TESTS_DIR)/*.c)
TESTS_OBJS := $(patsubst $(TESTS_DIR)/%.c, $(TESTS_OUT_DIR)/%.o, $(TESTS))
TESTS_BINS := $(patsubst $(TESTS_DIR)/%.c, $(TESTS_OUT_DIR)/%.bin, $(TESTS))

.PHONY: all vm tests clean

all: vm tests

vm: $(OBJS)
	$(CC) $(CFLAGS) $(INCLUDES) -o $(OUT_DIR)/dumb-vm $(SRCS) $(ENTRYPOINT) $(LIBS)

tests: $(TESTS_OBJS) #$(TESTS_BINS)
	$(CC) $(CFLAGS) -I./src $(INCLUDES) -o $(OUT_DIR)/test-dumb-vm $(SRCS) $(TESTS) $(LIBS) -lcriterion

$(TESTS_OUT_DIR)/%.o: $(TESTS_DIR)/%.c
	$(CC) $(CFLAGS) -I$(SRC_DIR) -c -o $@ $<

#$(TESTS_OUT_DIR)/%.bin: $(TESTS_OUT_DIR)/%.o
#	objcopy -O binary --only-section=.program $< $@
#	objcopy -O binary --only-section=.sdata $< /dev/stdout | tee -a $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) $(INCLUDES) -c -o $@ $<

$(TESTS_OUT_DIR)/%.o: $(TESTS_DIR)/%.c
	@mkdir -p $(TESTS_OUT_DIR)
	$(CC) $(CFLAGS) $(INCLUDES) -c -o $@ $<

clean:
	rm -rf $(OUT_DIR)
