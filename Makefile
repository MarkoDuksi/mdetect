CXX = g++
CXX_FLAGS = -std=c++17 -pthread -fdiagnostics-color=always -pedantic -Wall -Wextra -Wunreachable-code -Wfatal-errors
CXX_RELEASE_FLAGS = -DNDEBUG -DRELEASE
CXX_DEBUG_FLAGS = -g -DDEBUG
LD_FLAGS = -lX11

MAIN_BASENAME = main
SRC_DIR = src
OBJ_DIR = obj
LIB_DIRS = lib
HDR_DIRS = include
BIN_DIR = bin

SRCS = $(wildcard $(SRC_DIR)/*.cpp)
LIB_INCLUDE_DIRS_FLAGS = $(addprefix -L, $(LIB_DIRS))
HDR_INCLUDE_DIRS_FLAGS = $(addprefix -I, $(HDR_DIRS))

RELEASE_OBJ_DIR = $(OBJ_DIR)/release
RELEASE_BIN_DIR = $(BIN_DIR)/release
RELEASE_OBJS = $(SRCS:$(SRC_DIR)/%.cpp=$(RELEASE_OBJ_DIR)/%.o)
RELEASE_BIN = $(BIN_DIR)/release/$(MAIN_BASENAME).out

DEBUG_OBJ_DIR = $(OBJ_DIR)/debug
DEBUG_BIN_DIR = $(BIN_DIR)/debug
DEBUG_OBJS = $(SRCS:$(SRC_DIR)/%.cpp=$(DEBUG_OBJ_DIR)/%.o)
DEBUG_BIN = $(BIN_DIR)/debug/$(MAIN_BASENAME).out

.PHONY: all
all: release

.PHONY: release
release: release-dirs
release: $(RELEASE_BIN)

.PHONY: debug
debug: debug-dirs
debug: $(DEBUG_BIN)

.PHONY: release-dirs
release-dirs:
	mkdir -p $(RELEASE_OBJ_DIR) $(RELEASE_BIN_DIR)

.PHONY: debug-dirs
debug-dirs:
	mkdir -p $(DEBUG_OBJ_DIR) $(DEBUG_BIN_DIR)


$(RELEASE_BIN): $(RELEASE_OBJS)
	$(CXX) $(CXX_RELEASE_FLAGS) $(CXX_FLAGS) $^ -o $@ $(LIB_INCLUDE_DIRS_FLAGS) $(LD_FLAGS)

$(RELEASE_OBJ_DIR)/$(MAIN_BASENAME).o: $(SRC_DIR)/$(MAIN_BASENAME).cpp $(HDR_DIRS)/CImg.h $(SRC_DIR)/bbox.h $(SRC_DIR)/motion.h
	$(CXX) $(CXX_RELEASE_FLAGS) $(CXX_FLAGS) $(HDR_INCLUDE_DIRS_FLAGS) -c $< -o $@

$(RELEASE_OBJ_DIR)/bbox.o: $(SRC_DIR)/bbox.cpp $(SRC_DIR)/bbox.h
	$(CXX) $(CXX_RELEASE_FLAGS) $(CXX_FLAGS) $(HDR_INCLUDE_DIRS_FLAGS) -c $< -o $@

$(RELEASE_OBJ_DIR)/motion.o: $(SRC_DIR)/motion.cpp $(SRC_DIR)/motion.h $(SRC_DIR)/bbox.h $(SRC_DIR)/filters.h
	$(CXX) $(CXX_RELEASE_FLAGS) $(CXX_FLAGS) $(HDR_INCLUDE_DIRS_FLAGS) -c $< -o $@


$(DEBUG_BIN): $(DEBUG_OBJS)
	$(CXX) $(CXX_DEBUG_FLAGS) $(CXX_FLAGS) $^ -o $@ $(LIB_INCLUDE_DIRS_FLAGS) $(LD_FLAGS)

$(DEBUG_OBJ_DIR)/$(MAIN_BASENAME).o: $(SRC_DIR)/$(MAIN_BASENAME).cpp $(HDR_DIRS)/CImg.h $(SRC_DIR)/bbox.h $(SRC_DIR)/motion.h
	$(CXX) $(CXX_DEBUG_FLAGS) $(CXX_FLAGS) $(HDR_INCLUDE_DIRS_FLAGS) -c $< -o $@

$(DEBUG_OBJ_DIR)/bbox.o: $(SRC_DIR)/bbox.cpp $(SRC_DIR)/bbox.h
	$(CXX) $(CXX_DEBUG_FLAGS) $(CXX_FLAGS) $(HDR_INCLUDE_DIRS_FLAGS) -c $< -o $@

$(DEBUG_OBJ_DIR)/motion.o: $(SRC_DIR)/motion.cpp $(SRC_DIR)/motion.h $(SRC_DIR)/bbox.h $(SRC_DIR)/filters.h
	$(CXX) $(CXX_DEBUG_FLAGS) $(CXX_FLAGS) $(HDR_INCLUDE_DIRS_FLAGS) -c $< -o $@


.PHONY: clean-release
clean-release:
	rm -rf $(RELEASE_OBJ_DIR) $(RELEASE_BIN_DIR)

.PHONY: clean-debug
clean-debug:
	rm -rf $(DEBUG_OBJ_DIR) $(DEBUG_BIN_DIR)

.PHONY: clean-all
clean-all:
	rm -rf $(OBJ_DIR) $(BIN_DIR)
