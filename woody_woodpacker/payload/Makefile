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

NAME	=	payload

SRCS   =	payload.c

OBJS   = $(SRCS:.c=.o)

RM 		=	rm -rf

all		:	$(NAME)

$(NAME)	:	$(SRCS) $(OBJS)
	ld -shared -fstack-protector $(OBJS) -o $(NAME)

clean	:
	$(RM) $(NAME).o

fclean	: clean
	$(RM) $(NAME)

re		: clean all

.PHONY	: all re clean


