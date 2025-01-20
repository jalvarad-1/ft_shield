# COLORS #
#
GREEN 	= @echo "\033[0;32m"
BLUE 	= @echo "\033[0;34m"
PURPLE 	= @echo "\033[0;35m"
CYAN 	= @echo "\033[0;36m"
RESET 	= "\033[1;0m"

# VARIABLES #
#
NAME 		= ft_shield
BONUS_NAME 	= ft_shield_bonus
CC 			= gcc
CFLAGS 		= -Wall -Wextra -g -fsanitize=address -MMD -MP -Werror $(INCLUDE)
LDFLAGS		= -fsanitize=address
#LDFLAGS 	= -L/opt/homebrew/Cellar/oath-toolkit/2.6.11/lib -fsanitize=address
INCLUDE 	= -I/usr/include/liboath
#FLags for mac
#LDFLAGS 	= -L/opt/homebrew/Cellar/oath-toolkit/2.6.11/lib -fsanitize=address
#INCLUDE 	= -I/opt/homebrew/Cellar/oath-toolkit/2.6.11/include/liboath
LDLIBS		= -loath -lutil


# PATHS #
#
SRC_PATH    	= srcs
# SUBFILE1_PATH   = reporter
# SUBFILE2_PATH   = daemon
OBJ_PATH    	= objects
TOOLS_OBJ_PATH	= tools/objects

# SOURCES #
#
# SUBFILE1_SRC = reporter.c
# SUBFILE2_SRC = daemon.c


SRC =	main.c \
		daemon.c \
		socket.c \
		authentication.c


UPX_VERSION = 4.2.4
UPX_EXECUTABLE = upx-$(UPX_VERSION)-amd64_linux/upx

# RULES #
#
all: $(NAME)

SRCS 	  = $(addprefix $(SRC_PATH)/, $(SRC))
OBJS 	  = $(addprefix $(OBJ_PATH)/, $(SRC:%.c=%.o))

$(OBJ_PATH):
	mkdir -p $(OBJ_PATH)

$(TOOLS_OBJ_PATH):
	mkdir -p $(TOOLS_OBJ_PATH)

TOOLS_SRC = tools/secret_generator.c
TOOLS_OBJ = $(addprefix $(TOOLS_OBJ_PATH)/, $(notdir $(TOOLS_SRC:.c=.o)))
# Generar la clave secreta aleatoria en Base32

$(TOOLS_OBJ_PATH)/%.o: tools/%.c | $(TOOLS_OBJ_PATH)
	$(CC) $(CFLAGS) -c $< -o $@

tools/generate_secret: $(TOOLS_OBJ)
	$(CC) $(CFLAGS) $(LDFLAGS) $(LDLIBS) $^ -o $@

define get_secret
SECRET := $(shell ./tools/generate_secret)
export SECRET
endef

.PHONY: gen_secret
gen_secret: tools/generate_secret
	$(eval $(get_secret))

$(OBJ_PATH)/%.o: $(SRC_PATH)/%.c gen_secret | $(OBJ_PATH)
	echo $(get_secret)
	$(CC) $(CFLAGS) -DSECRET=\"$(SECRET)\" -c $< -o $@

$(NAME): $(OBJS) Makefile
	$(CC) $(OBJS) $(LDFLAGS) $(LDLIBS) -o $(NAME)
	echo $(SECRET)
	qrencode -t ANSI "otpauth://totp/ft_shield:jalvarodro@example.com?secret=$(SECRET)&issuer=ft_shield"
	$(GREEN) Program asembled $(RESET)
	@echo "⠀⠀⠀	    ⣠⣴⣶⣿⣿⣷⣶⣄⣀⣀\n\
⠀⠀⠀⠀⠀⠀⠀⠀⠀⣰⣾⣿⣿⡿⢿⣿⣿⣿⣿⣿⣿⣿⣷⣦⡀⠀⠀⠀⠀⠀\n\
⠀⠀⠀⠀⠀⠀⠀⢀⣾⣿⣿⡟⠁⣰⣿⣿⣿⡿⠿⠻⠿⣿⣿⣿⣿⣧⠀⠀⠀⠀\n\
⠀⠀⠀⠀⠀⠀⠀⣾⣿⣿⠏⠀⣴⣿⣿⣿⠉⠀⠀⠀⠀⠀⠈⢻⣿⣿⣇⠀⠀⠀\n\
⠀⠀⠀⠀⢀⣠⣼⣿⣿⡏⠀⢠⣿⣿⣿⠇⠀⠀⠀⠀⠀⠀⠀⠈⣿⣿⣿⡀⠀⠀\n\
⠀⠀⠀⣰⣿⣿⣿⣿⣿⡇⠀⢸⣿⣿⣿⡀⠀⠀⠀⠀⠀⠀⠀⠀⣿⣿⣿⡇⠀⠀\n\
⠀⠀⢰⣿⣿⡿⣿⣿⣿⡇⠀⠘⣿⣿⣿⣧⠀⠀⠀⠀⠀⠀⢀⣸⣿⣿⣿⠁⠀⠀\n\
⠀⠀⣿⣿⣿⠁⣿⣿⣿⡇⠀⠀⠻⣿⣿⣿⣷⣶⣶⣶⣶⣶⣿⣿⣿⣿⠃⠀⠀⠀\n\
⠀⢰⣿⣿⡇⠀⣿⣿⣿⠀⠀⠀⠀⠈⠻⣿⣿⣿⣿⣿⣿⣿⣿⣿⠟⠁⠀⠀⠀⠀\n\
⠀⢸⣿⣿⡇⠀⣿⣿⣿⠀⠀⠀⠀⠀⠀⠀⠉⠛⠛⠛⠉⢉⣿⣿⠀⠀⠀⠀⠀⠀\n\
⠀⢸⣿⣿⣇⠀⣿⣿⣿⠀⠀⠀⠀⠀⢀⣤⣤⣤⡀⠀⠀⢸⣿⣿⣿⣷⣦⠀⠀⠀\n\
⠀⠀⢻⣿⣿⣶⣿⣿⣿⠀⠀⠀⠀⠀⠈⠻⣿⣿⣿⣦⡀⠀⠉⠉⠻⣿⣿⡇⠀⠀\n\
⠀⠀⠀⠛⠿⣿⣿⣿⣿⣷⣤⡀⠀⠀⠀⠀⠈⠹⣿⣿⣇⣀⠀⣠⣾⣿⣿⡇⠀⠀\n\
⠀⠀⠀⠀⠀⠀⠀⠹⣿⣿⣿⣿⣦⣤⣤⣤⣤⣾⣿⣿⣿⣿⣿⣿⣿⣿⡟⠀⠀⠀\n\
⠀⠀⠀⠀⠀⠀⠀⠀⠀⠉⠻⢿⣿⣿⣿⣿⣿⣿⠿⠋⠉⠛⠋⠉⠉⠁⠀⠀⠀⠀\n\
⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠈⠉⠉⠉⠁\n"

bonus: $(BONUS_NAME)

$(BONUS_NAME):
	$(MAKE) -C ./bonus/

-include $(OBJS:.o=.d)
clean:
	$(PURPLE) CLEANING OBJECTS $(RESET)
	rm -rf $(OBJ_PATH) $(TOOLS_OBJ_PATH)
	-$(MAKE) -C ./srcs/modules/ clean
	$(MAKE) -C ./bonus/ clean

fclean: clean
	$(PURPLE) CLEANING OBJECTS AND EXEC $(RESET)
	rm -rf $(NAME) tools/generate_secret tools/ft_shield_qr.png
	rm -rf /etc/systemd/system/ft_shield.service 
	rm -rf /bin/ft_shield
	rm -rf /home/ubuntu/ft_shield/ft_shield.log
	rm -rf /home/ubuntu/ft_shield/new_ft_shield.log
	$(MAKE) -C ./bonus/ fclean

re: fclean all

.PHONY: all clean fclean re
