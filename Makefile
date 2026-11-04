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

# Compila e roda os testes de terminal (salas e criatura), sem raylib
teste: teste_salas.c teste_criatura.c salas.c criatura.c salas.h criatura.h jogo.h
	$(CC) $(CFLAGS) teste_salas.c salas.c -o teste_salas
	$(CC) $(CFLAGS) teste_criatura.c criatura.c salas.c -o teste_criatura
	./teste_salas
	./teste_criatura

# Apaga o que foi gerado pela compilação
clean:
	rm -f $(OBJETOS) rebobina teste_salas teste_criatura

.PHONY: clean teste
