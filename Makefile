# Configuration: Set to 1 to use environment variables, 0 for hardcoded values
USE_ENV_VARS = 1

# Hardcoded values (used when USE_ENV_VARS = 0)
DEFAULT_MODEL = model_name
DEFAULT_API_URL = https://example.com/v1/chat/completions

CXX = c++
CXXFLAGS = -Wall -Wextra -Werror -std=c++98 -I./includes

CXXFLAGS += -DUSE_ENV_VARS=$(USE_ENV_VARS)
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
       $(SRCS_DIR)/HttpClient.cpp

OBJS = $(SRCS:$(SRCS_DIR)/%.cpp=$(OBJS_DIR)/%.o)

all: $(NAME)

$(NAME): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(NAME) $(OBJS)

$(OBJS_DIR)/%.o: $(SRCS_DIR)/%.cpp | $(OBJS_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJS_DIR):
	mkdir -p $(OBJS_DIR)

clean:
	rm -rf $(OBJS_DIR)

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re
