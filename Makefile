# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: jinam <jinam@student.42seoul.kr>           +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2023/06/02 11:11:51 by jinam             #+#    #+#              #
#    Updated: 2023/07/28 21:10:30 by jinam            ###   ########.fr        #
#                                                                              #
# **************************************************************************** #


# Target
NAME = Server

# Cmd & Options
CXX			= c++
CXXFLAGS	= #-fsanitize=address -g#-Wall -Werror -Wextra -std=c++98 -g3
RM 			= rm
RMFLAGS		= -f
OUT_DIR		= objs
FILE		= main ConfigParser ConfigFunctions Location Server Client config action ServerManager RequestMessageReader Event
OBJECTS		= $(addprefix $(OUT_DIR)/, $(addsuffix .o, $(FILE)))

# Compile rules
$(OUT_DIR)/%.o	: %.cpp
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
