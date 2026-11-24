CC      = cc
CXX     = c++

CFLAGS   = -Wall -Wextra -Werror
CXXFLAGS = -Wall -Wextra -Werror -std=c++98

BIN_DIR = bin

C_SRCS   = $(wildcard *.c)
CPP_SRCS = $(wildcard *.cpp)

C_BINS   = $(patsubst %.c,$(BIN_DIR)/%,$(C_SRCS))
CPP_BINS = $(patsubst %.cpp,$(BIN_DIR)/%,$(CPP_SRCS))

all: $(C_BINS) $(CPP_BINS)

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

$(BIN_DIR)/%: %.c | $(BIN_DIR)
	$(CC) $(CFLAGS) $< -o $@

$(BIN_DIR)/%: %.cpp | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $< -o $@

clean:
	rm -rf $(BIN_DIR)

re:
	$(MAKE) clean
	$(MAKE) all

.PHONY: all clean re


#実行方法
# make
# 例）./bin/07_experiment_accept