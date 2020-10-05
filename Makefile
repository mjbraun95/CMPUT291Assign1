default:
	gcc shell.c process.c cli.c -o shell379 -Werror -Wall -g

clean:
	rm shell379