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

# Compila e roda o teste da lista circular, sem raylib
teste: teste_salas.c salas.c salas.h jogo.h
	$(CC) $(CFLAGS) teste_salas.c salas.c -o teste_salas
	./teste_salas

# Apaga o que foi gerado pela compilação
clean:
	rm -f $(OBJETOS) rebobina teste_salas

.PHONY: clean teste
