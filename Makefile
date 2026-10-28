CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -g
LDLIBS = -lraylib -lGL -lm -lpthread -ldl -lrt -lX11

FONTES = main.c salas.c render.c efeitos.c criatura.c regras.c \
         fotogramas.c ordenacao.c recursos.c menu.c
OBJETOS = $(FONTES:.c=.o)
HEADERS = $(wildcard *.h)

# Junta todos os .o no executável do jogo
rebobina: $(OBJETOS)
	$(CC) $(OBJETOS) -o rebobina $(LDLIBS)

# Compila cada .c; recompila se algum header mudar
%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

# Apaga o que foi gerado pela compilação
clean:
	rm -f $(OBJETOS) rebobina

.PHONY: clean
