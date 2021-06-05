# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: eparisot <eparisot@student.42.fr>          +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2021/02/26 14:55:42 by eparisot          #+#    #+#              #
#    Updated: 2021/02/26 14:55:42 by eparisot         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

NAME	=	woody_woodpacker

SRCS	=	srcs/woody_woodpacker.c \
			srcs/payload_builder.c \
			srcs/tools.c \
			srcs/encrypt.c

INC		=	includes/woody_woodpacker.h

OBJS	=	$(SRCS:.c=.o)

LIBS	=	libft/libft.a

PAYLOAD =	payload/payload

CFLAGS	=	-Wall -Wextra -Werror -g3

RM 		= 	rm -rf

all		:	$(LIBS) $(NAME) $(PAYLOAD)

$(NAME)	:	$(OBJS) $(INC)
	gcc $(CFLAGS) $(OBJS) $(LIBS) -o $(NAME)

$(LIBS)	: 	.FORCE
	@$(MAKE) -C libft

$(PAYLOAD): .FORCE
	@$(MAKE) -C payload

.FORCE	:

clean	:
	$(RM) $(OBJS)
	$(MAKE) clean -C libft
	$(MAKE) clean -C payload 

fclean	:	clean
	$(RM) $(NAME) woody $(LIBS) $(PAYLOAD)

re		:	fclean all

.PHONY	: all re clean fclean
