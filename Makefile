# Configuration: Set to 1 to use environment variables, 0 for hardcoded values
USE_ENV_VARS = 1
USE_RESPONSE_FORMAT = 0

# Hardcoded values (used when USE_ENV_VARS = 0)
DEFAULT_MODEL = model_name
DEFAULT_API_URL = https://example.com/v1/chat/completions

CXX = c++
CXXFLAGS = -Wall -Wextra -Werror -std=c++98 -I./includes

CXXFLAGS += -DUSE_ENV_VARS=$(USE_ENV_VARS)
CXXFLAGS += -DUSE_RESPONSE_FORMAT=$(USE_RESPONSE_FORMAT)
ifeq ($(USE_ENV_VARS),0)
CXXFLAGS += -DDEFAULT_MODEL=\"$(DEFAULT_MODEL)\" \
            -DDEFAULT_API_URL=\"$(DEFAULT_API_URL)\"
endif

NAME = examcli
SRCS_DIR = srcs
OBJS_DIR = objs

SRCS = $(SRCS_DIR)/main.cpp \
       $(SRCS_DIR)/ArgParser.cpp \
       $(SRCS_DIR)/SubjectLoader.cpp \
       $(SRCS_DIR)/LLMClient.cpp \
       $(SRCS_DIR)/Submission.cpp \
       $(SRCS_DIR)/Result.cpp \
       $(SRCS_DIR)/FileReader.cpp \
       $(SRCS_DIR)/HttpClient.cpp \
       $(SRCS_DIR)/Generator.cpp \
       $(SRCS_DIR)/Validator.cpp \
       $(SRCS_DIR)/QualityScorer.cpp \
       $(SRCS_DIR)/ExerciseSaver.cpp \
       $(SRCS_DIR)/ExerciseExporter.cpp \
       $(SRCS_DIR)/ExerciseImporter.cpp

OBJS = $(SRCS:$(SRCS_DIR)/%.cpp=$(OBJS_DIR)/%.o)

all: $(NAME)

$(NAME): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(NAME) $(OBJS)

$(OBJS_DIR)/%.o: $(SRCS_DIR)/%.cpp | $(OBJS_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJS_DIR):
	mkdir -p $(OBJS_DIR)

clean:
	rm -rf $(OBJS_DIR) $(TESTS_OBJS_DIR) test_validator test_qualityscorer test_integration

fclean: clean
	rm -f $(NAME)

re: fclean all

# Test targets
TESTS_DIR = tests
TESTS_OBJS_DIR = tests_objs

TEST_SRCS = $(TESTS_DIR)/test_generator.cpp \
            $(TESTS_DIR)/test_validator.cpp \
            $(TESTS_DIR)/test_qualityscorer.cpp \
            $(TESTS_DIR)/test_integration.cpp

TEST_BINS = $(TEST_SRCS:$(TESTS_DIR)/%.cpp=$(TESTS_OBJS_DIR)/%)

# Test binaries
test_validator: $(TESTS_OBJS_DIR)/test_validator.o $(filter-out $(OBJS_DIR)/main.o,$(OBJS))
	$(CXX) $(CXXFLAGS) -o $@ $^

test_qualityscorer: $(TESTS_OBJS_DIR)/test_qualityscorer.o $(filter-out $(OBJS_DIR)/main.o,$(OBJS))
	$(CXX) $(CXXFLAGS) -o $@ $^

test_integration: $(TESTS_OBJS_DIR)/test_integration.o $(filter-out $(OBJS_DIR)/main.o,$(OBJS))
	$(CXX) $(CXXFLAGS) -o $@ $^

$(TESTS_OBJS_DIR)/%.o: $(TESTS_DIR)/%.cpp | $(TESTS_OBJS_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(TESTS_OBJS_DIR):
	mkdir -p $(TESTS_OBJS_DIR)

# Run all tests
test: all test_validator test_qualityscorer
	@echo "Running unit tests..."
	@./test_validator
	@./test_qualityscorer
	@echo "All tests passed!"

# Run integration test
test-integration: all test_integration
	@echo "Running integration test..."
	@./test_integration

.PHONY: all clean fclean re test test-integration
