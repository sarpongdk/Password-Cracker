all:
	clear
	gcc crack.c -o crack -lpthread -lcrypt -std=c99
