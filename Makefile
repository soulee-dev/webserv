# Target
NAME = webserv

# Cmd & Options
CXX			= c++
#CXXFLAGS	= # -fsanitize=address -g#-Wall -Werror -Wextra -std=c++98 -g3
CXXFLAGS	= -g
RM 			= rm
RMFLAGS		= -f
OUT_DIR		= objs
SRC_DIR		= srcs
FILE		= Client ClientManager ConfigFunctions ConfigParser Event Location Message RequestMessage \
				Http/Handler/ErrorHandler ResponseMessage Server ServerManager Http/HttpRequestManager Http/Handler/StaticHandler Http/Handler/DynamicHandler Http/HttpParser main
OBJECTS		= $(addprefix $(OUT_DIR)/, $(addsuffix .o, $(FILE)))

# Compile rules
$(OUT_DIR)/%.o : $(SRC_DIR)/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) -c $(CXXFLAGS) $< -o $@

.PHONY	: all no clean fclean re

all		: $(NAME)

$(NAME)	: $(OBJECTS)
	$(CXX) $(CXXFLAGS) $(OBJECTS) -o $(NAME)

clean	:
	@$(RM) $(RMFLAGS) -r $(OUT_DIR)

fclean	: clean
	@$(RM) $(RMFLAGS) $(NAME)

re		:
	@make fclean
	@make all 
