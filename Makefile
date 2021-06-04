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

LIBS	=	libft/libft.a

PAYLOAD =	payload/payload

CLIENT =	client/client

CFLAGS	=	-Wall -Wextra -Werror -g3

RM 		= 	rm -rf

all		:	$(LIBS) $(NAME) $(PAYLOAD) $(CLIENT)

$(NAME)	:	$(OBJS) $(INC)
	gcc $(CFLAGS) $(OBJS) $(LIBS) -o $(NAME)

$(LIBS)	: 	.FORCE
	@$(MAKE) -C libft

$(PAYLOAD) : .FORCE
	@$(MAKE) -C payload

$(CLIENT) : .FORCE
	@$(MAKE) -C client

.FORCE	:

clean	:
	$(RM) $(OBJS)
	$(MAKE) clean -C libft
	$(MAKE) clean -C payload
	$(MAKE) clean -C client

fclean	:	clean
	-sudo systemctl stop Durex.service
	-sudo systemctl disable Durex.service
	sudo $(RM) $(NAME) woody $(LIBS) $(PAYLOAD) $(CLIENT) /usr/bin/Durex /etc/systemd/system/Durex.service $2>/dev/null

re		:	fclean all

.PHONY	: all re clean fclean
