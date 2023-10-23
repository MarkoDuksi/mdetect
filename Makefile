MAIN_BASENAME = main
SRC_DIR = src
LIB_INCLUDE_DIRS = lib
HDR_INCLUDE_DIRS = include
OBJ_DIR = obj
BIN_DIR = bin
DEP_DIR = .deps
TESTS_DIR = test_imgs

CXX = g++
CXX_FLAGS = -std=c++17 -fdiagnostics-color=always -pedantic -Wall -Wextra -Wunreachable-code -Wfatal-errors
CXX_DEBUG_FLAGS = -g -DDEBUG
CXX_RELEASE_FLAGS = -O3 -DNDEBUG -DRELEASE
DEP_FLAGS = -MT $@ -MMD -MP -MF $(DEP_DIR)/$(*D)/$(*F).$(BUILD_TYPE).d.tmp
LIB_INCLUDE_DIRS_FLAGS = $(addprefix -L, $(LIB_INCLUDE_DIRS))
HDR_INCLUDE_DIRS_FLAGS = $(addprefix -I, $(HDR_INCLUDE_DIRS))
LD_FLAGS = -lmdjpeg

PRECOMPILE = @mkdir -p $(@D) $(@D:$(OBJ_DIR)/$(BUILD_TYPE)%=$(DEP_DIR)%)
POSTCOMPILE = @mv -f $(DEP_DIR)/$(*D)/$(*F).$(BUILD_TYPE).d.tmp $(DEP_DIR)/$(*D)/$(*F).$(BUILD_TYPE).d && touch $@

SRCS = $(wildcard $(shell find $(SRC_DIR) -name '*.cpp'))
SRCS_SUBDIRS = $(wildcard $(shell find $(SRC_DIR)/* -type d))
DEPS = $(SRCS:$(SRC_DIR)/%.cpp=$(DEP_DIR)/%.debug.d) $(SRCS:$(SRC_DIR)/%.cpp=$(DEP_DIR)/%.release.d)
DEP_SUBDIRS = $(SRCS_SUBDIRS:$(SRC_DIR)/%=$(DEP_DIR)/%)
DEP_TREE = $(DEP_DIR) $(DEP_SUBDIRS)

DEBUG_OBJ_DIR = $(OBJ_DIR)/debug
DEBUG_BIN_DIR = $(BIN_DIR)/debug
DEBUG_OBJS = $(SRCS:$(SRC_DIR)/%.cpp=$(DEBUG_OBJ_DIR)/%.o)
DEBUG_BIN = $(DEBUG_BIN_DIR)/$(MAIN_BASENAME).out

RELEASE_OBJ_DIR = $(OBJ_DIR)/release
RELEASE_BIN_DIR = $(BIN_DIR)/release
RELEASE_OBJS = $(SRCS:$(SRC_DIR)/%.cpp=$(RELEASE_OBJ_DIR)/%.o)
RELEASE_BIN = $(RELEASE_BIN_DIR)/$(MAIN_BASENAME).out

.PHONY: all
all: debug release

.PHONY: debug
debug: $(DEBUG_BIN)

.PHONY: release
release: $(RELEASE_BIN)


$(DEBUG_BIN): $(DEBUG_OBJS) | $(DEBUG_BIN_DIR)
	$(CXX) $(CXX_DEBUG_FLAGS) $(CXX_FLAGS) $^ -o $@ $(LIB_INCLUDE_DIRS_FLAGS) $(LD_FLAGS)

$(DEBUG_OBJ_DIR)/%.o: BUILD_TYPE = debug
.SECONDEXPANSION:
$(DEBUG_OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp $(DEP_DIR)/%.$$(BUILD_TYPE).d
	$(PRECOMPILE)
	$(CXX) $(DEP_FLAGS) $(CXX_DEBUG_FLAGS) $(CXX_FLAGS) $(HDR_INCLUDE_DIRS_FLAGS) -c $< -o $@
	$(POSTCOMPILE)


$(RELEASE_BIN): $(RELEASE_OBJS) | $(RELEASE_BIN_DIR)
	$(CXX) $(CXX_RELEASE_FLAGS) $(CXX_FLAGS) $^ -o $@ $(LIB_INCLUDE_DIRS_FLAGS) $(LD_FLAGS)

$(RELEASE_OBJ_DIR)/%.o: BUILD_TYPE = release
.SECONDEXPANSION:
$(RELEASE_OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp $(DEP_DIR)/%.$$(BUILD_TYPE).d
	$(PRECOMPILE)
	$(CXX) $(DEP_FLAGS) $(CXX_RELEASE_FLAGS) $(CXX_FLAGS) $(HDR_INCLUDE_DIRS_FLAGS) -c $< -o $@
	$(POSTCOMPILE)


$(DEBUG_BIN_DIR) $(RELEASE_BIN_DIR):
	mkdir -p $@


.PHONY: clean-debug
clean-debug:
	rm -rf $(DEBUG_OBJ_DIR) $(DEBUG_BIN_DIR) $(addsuffix /*debug.d, $(DEP_TREE))
	find . -type d -name $(DEP_DIR) -empty -delete
	find . -type d -name $(OBJ_DIR) -empty -delete
	find . -type d -name $(BIN_DIR) -empty -delete

.PHONY: clean-release
clean-release:
	rm -rf $(RELEASE_OBJ_DIR) $(RELEASE_BIN_DIR) $(addsuffix /*release.d, $(DEP_TREE))
	find . -type d -name $(DEP_DIR) -empty -delete
	find . -type d -name $(OBJ_DIR) -empty -delete
	find . -type d -name $(BIN_DIR) -empty -delete

.PHONY: clean-tests
clean-tests:
	find $(TESTS_DIR) -type f -name '*.pgm' ! -regex '.+_ref\/.+' -delete
	find $(TESTS_DIR) -type d -empty -delete

.PHONY: clean
clean: clean-tests
	rm -rf $(OBJ_DIR) $(BIN_DIR) $(DEP_DIR)


$(DEPS):
include $(wildcard $(DEPS))
