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

NAME	=	payload

SRCS	=	srcs/payload.c \
			srcs/encrypt.c

INC		=	includes/payload.h

OBJS	=	$(SRCS:.c=.o)

CFLAGS	=	-pthread -Wall -Wextra -Werror -s

RM 		= 	rm -rf

all		:	$(NAME)

$(NAME)	:	$(OBJS) $(INC)
	gcc $(CFLAGS) $(OBJS) -o $(NAME)

.FORCE	:

clean	:
	$(RM) $(OBJS)

fclean	:	clean
	$(RM) $(NAME)

re		:	fclean all

.PHONY	: all re clean fclean
