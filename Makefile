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

PAYLOAD =	payload/payload.c

CFLAGS	=	-Wall -Wextra -Werror -g3

RM 		= 	rm -rf

all		:	$(LIBS) $(NAME) $(PAYLOAD)

$(NAME)	:	$(OBJS) $(INC)
	gcc $(CFLAGS) $(OBJS) $(LIBS) -o $(NAME)

$(LIBS)	: 	.FORCE
	@$(MAKE) -C libft

$(PAYLOAD) : .FORCE
	@$(MAKE) -C payload

.FORCE	:

clean	:
	$(RM) $(OBJS)
	$(MAKE) clean -C libft
	$(MAKE) clean -C payload

fclean	:	clean
	$(RM) $(NAME) woody $(LIBS) $(PAYLOAD) _Durex

re		:	fclean all

.PHONY	: all re clean fclean
