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
CC 			= gcc
CFLAGS 		= -Wall -Wextra -g -fsanitize=address -MMD -MP -Werror $(INCLUDE)
LDFLAGS 	= -L/opt/homebrew/Cellar/oath-toolkit/2.6.11/lib -fsanitize=address
LDLIBS		= -loath
INCLUDE 	= -I/opt/homebrew/Cellar/oath-toolkit/2.6.11/include/liboath


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
		reporter.c \
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

$(NAME): modules $(OBJS) Makefile
	$(CC) $(OBJS) $(LDFLAGS) $(LDLIBS) -o $(NAME)
	echo $(SECRET)
	qrencode -o tools/ft_shield_qr.png "otpauth://totp/ft_shield:jalvarodro@example.com?secret=$(SECRET)&issuer=ft_shield"
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

modules:
	$(MAKE) -C ./srcs/modules/

$(UPX_EXECUTABLE): 
	wget https://github.com/upx/upx/releases/download/v$(UPX_VERSION)/upx-$(UPX_VERSION)-amd64_linux.tar.xz
	tar -xvf  upx-$(UPX_VERSION)-amd64_linux.tar.xz
	rm -rf upx-$(UPX_VERSION)-amd64_linux.tar.xz

pack: $(UPX_EXECUTABLE) $(NAME)
	./$(UPX_EXECUTABLE) --best $(NAME)

-include $(OBJS:.o=.d)
clean:
	$(PURPLE) CLEANING OBJECTS $(RESET)
	rm -rf $(OBJ_PATH) $(TOOLS_OBJ_PATH)

fclean: clean
	$(PURPLE) CLEANING OBJECTS AND EXEC $(RESET)
	rm -rf $(NAME) tools/generate_secret tools/ft_shield_qr.png
re: fclean all

.PHONY: all clean fclean re
