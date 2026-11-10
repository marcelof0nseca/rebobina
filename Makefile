CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -g

# O raylib usa bibliotecas diferentes em cada sistema
ifeq ($(OS),Windows_NT)
    LDLIBS = -lraylib -lopengl32 -lgdi32 -lwinmm
    EXTENSAO = .exe
    APAGAR = del /Q /F
    RODAR =
else
    LDLIBS = -lraylib -lGL -lm -lpthread -ldl -lrt -lX11
    EXTENSAO =
    APAGAR = rm -f
    RODAR = ./
endif

JOGO = rebobina$(EXTENSAO)
TESTE_SALAS = teste_salas$(EXTENSAO)
TESTE_CRIATURA = teste_criatura$(EXTENSAO)

FONTES = main.c salas.c render.c efeitos.c criatura.c regras.c          fotogramas.c ordenacao.c recursos.c menu.c
OBJETOS = $(FONTES:.c=.o)
HEADERS = $(wildcard *.h)

# Junta todos os .o no executável do jogo
$(JOGO): $(OBJETOS)
	$(CC) $(OBJETOS) -o $(JOGO) $(LDLIBS)

# Compila cada .c; recompila se algum header mudar
%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

# Compila e roda os testes de terminal (salas e criatura), sem raylib
teste: teste_salas.c teste_criatura.c salas.c criatura.c salas.h criatura.h jogo.h
	$(CC) $(CFLAGS) teste_salas.c salas.c -o $(TESTE_SALAS)
	$(CC) $(CFLAGS) teste_criatura.c criatura.c salas.c -o $(TESTE_CRIATURA)
	$(RODAR)$(TESTE_SALAS)
	$(RODAR)$(TESTE_CRIATURA)

# Apaga o que foi gerado pela compilação
clean:
	$(APAGAR) $(OBJETOS) $(JOGO) $(TESTE_SALAS) $(TESTE_CRIATURA)

.PHONY: clean teste
