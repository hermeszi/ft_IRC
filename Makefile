NAME = ircserv
CXX = c++
CXXFLAGS = -Wall -Wextra -Werror -MMD -Iinc
CXXFLAGS += -std=c++98
SRC = src/Channel.cpp src/Client.cpp src/main.cpp src/Server.cpp
OBJ = $(SRC:%.cpp=%.o)
DEPS = $(SRC:%.cpp=%.d)

all: $(NAME)

%.o: %.cpp
	@echo "Converting $< to $@"
	@$(CXX) $(CXXFLAGS) -c $< -o $@

$(NAME): $(OBJ)
	@echo "Building..."
	@$(CXX) $(CXXFLAGS) $(OBJ) -o $(NAME)
	@echo "Built successfully"

clean:
	@echo "Cleaning..."
	@rm -f $(OBJ) $(DEPS)
	@echo "Clean up successfully!"

fclean: clean
	@echo "Full cleaning..."
	@rm -f $(NAME)
	@echo "Full clean up successfully"

re: fclean all

-include $(DEPS)

.PHONY: all clean fclean re