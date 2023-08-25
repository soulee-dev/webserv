# Target
NAME = webserv

# Cmd & Options
CXX			= c++
# CXXFLAGS	= -fsanitize=address -g -Wall -Werror -Wextra -std=c++98 -g3
CXXFLAGS	= -g3
RM 			= rm
RMFLAGS		= -f
OUT_DIR		= objs
SRC_DIR		= srcs
FILE		= main Server Webserv ConfigFunctions Servers Location Utils Event Client
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
