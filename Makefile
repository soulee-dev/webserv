# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: gyopark <gyopark@student.42.fr>            +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2023/06/02 11:11:51 by jinam             #+#    #+#              #
#    Updated: 2023/08/13 14:33:14 by gyopark          ###   ########.fr        #
#                                                                              #
# **************************************************************************** #


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
FILE		= Client ClientManager ConfigFunctions ConfigParser Event Location Message RequestMessage Http/Handler/Handler \
				Http/Handler/ErrorHandler ResponseMessage Server ServerManager Http/HttpRequestManager Http/Handler/StaticHandler Http/Handler/DynamicHandler main
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
