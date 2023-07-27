NAME = web
CXX = c++
CXXFLAGS = -Wall -Wextra -Werror -std=c++98 -pedantic-errors -g
SRCS = srcs/main.cpp srcs/Server.cpp srcs/kq.cpp
OBJS = $(SRCS:.cpp=.o)

%.o : %.cpp
	@echo "$(YELLOW) *** Generating object files ... *** $(DEFAULT)"
	@$(CXX) $(CXXFLAGS) $< -c -o $@

$(NAME) : $(OBJS)
	@$(CXX) $(OBJS) $(CXXFLAGS) -o $(NAME)
	@echo "$(GREEN) *** [$(NAME)] Linking Success *** $(DEFAULT)"

all : $(NAME)

clean : 
	@echo "$(RED) *** Deleting objects ... *** $(DEFAULT)"
	@rm -rf $(OBJS)

fclean : clean
	@echo "$(RED) *** Deleting executable ... *** $(DEFAULT)"
	@rm -rf $(NAME)

re : fclean
	@make all
	@echo "$(GREEN)[$(NAME)] Cleaned and rebuilt everything *** $(DEFAULT)"

.PHONY = all clean fclean re

# ---------------------------------------------------------------------------#
#                             Define the colors                              #
# ---------------------------------------------------------------------------#
GRAY		=	\033[1;30m
RED			=	\033[1;31m
GREEN		=	\033[1;32m
YELLOW		=	\033[1;33m
BLUE		=	\033[1;34m
MAGENTA		=	\033[1;35m
CYAN		=	\033[1;36m
WHITE		=	\033[1;37m
DEFAULT		=	\033[1;0m