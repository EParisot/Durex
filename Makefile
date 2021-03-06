# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: eparisot <eparisot@student.42.fr>          +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2021/05/25 14:55:42 by eparisot          #+#    #+#              #
#    Updated: 2021/05/25 14:55:42 by eparisot         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

NAME	=	Durex

SRCS	=	srcs/durex.c 

INC		=	includes/durex.h

OBJS	=	$(SRCS:.c=.o)

PAYLOAD =	payload/payload

PACKER  =	woody_woodpacker/woody_woodpacker

CLIENT  =	client/client

CFLAGS	=	-Wall -Wextra -Werror

RM 		= 	rm -rf

all		:	$(NAME) $(PAYLOAD) $(PACKER) $(CLIENT)

$(NAME)	:	$(OBJS) $(INC)
	gcc $(CFLAGS) $(OBJS) -o $(NAME)

$(PAYLOAD) : .FORCE
	@$(MAKE) -C payload

$(PACKER) : .FORCE
	@$(MAKE) -C woody_woodpacker

$(CLIENT) : .FORCE
	@$(MAKE) -C client

.FORCE	:

clean	:
	$(RM) $(OBJS)
	$(MAKE) clean -C payload
	$(MAKE) clean -C woody_woodpacker
	$(MAKE) clean -C client

fclean	:	clean
	-sudo systemctl stop Durex.service
	-sudo systemctl disable Durex.service
	-sudo systemctl daemon-reload
	-sudo /etc/init.d/Durex stop
	-sudo update-rc.d -f Durex remove
	-sudo pkill Durex
	sudo $(RM) $(NAME) $(PAYLOAD) $(PACKER) $(CLIENT) /bin/Durex /etc/systemd/system/Durex.service /etc/init.d/Durex.service $2>/dev/null

re		:	fclean all

.PHONY	: all re clean fclean
