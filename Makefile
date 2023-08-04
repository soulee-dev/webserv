# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: gyopark <gyopark@student.42.fr>            +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2023/06/02 11:11:51 by jinam             #+#    #+#              #
#    Updated: 2023/08/04 20:00:28 by gyopark          ###   ########.fr        #
#                                                                              #
# **************************************************************************** #


# Target
NAME = webserv

# Cmd & Options
CXX			= c++
#CXXFLAGS	= # -fsanitize=address -g#-Wall -Werror -Wextra -std=c++98 -g3
CXXFLAGS	=
RM 			= rm
RMFLAGS		= -f
OUT_DIR		= objs
SRC_DIR		= srcs
FILE		= Client ClientManager ConfigFunctions ConfigParser Event Location Message \
				RequestMessage ResponseMessage Server ServerManager ParseRequest main
OBJECTS		= $(addprefix $(OUT_DIR)/, $(addsuffix .o, $(FILE)))

# Compile rules
$(OUT_DIR)/%.o : $(SRC_DIR)/%.cpp
	@mkdir -p $(OUT_DIR)
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
