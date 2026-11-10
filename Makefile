CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -g

# O raylib usa bibliotecas diferentes em cada sistema.
# No Windows, use o w64devkit que vem no instalador do raylib: o make dele
# roda os comandos num terminal estilo Linux, então rm e ./ funcionam igual.
ifeq ($(OS),Windows_NT)
    LDLIBS = -lraylib -lopengl32 -lgdi32 -lwinmm
    EXTENSAO = .exe
else
    LDLIBS = -lraylib -lGL -lm -lpthread -ldl -lrt -lX11
    EXTENSAO =
endif
APAGAR = rm -f
RODAR = ./

JOGO = rebobina$(EXTENSAO)
TESTE_SALAS = teste_salas$(EXTENSAO)
TESTE_CRIATURA = teste_criatura$(EXTENSAO)
TESTE_REGRAS = teste_regras$(EXTENSAO)

FONTES = main.c salas.c render.c efeitos.c criatura.c regras.c          fotogramas.c ordenacao.c recursos.c menu.c
OBJETOS = $(FONTES:.c=.o)
HEADERS = $(wildcard *.h)

# Junta todos os .o no executável do jogo
$(JOGO): $(OBJETOS)
	$(CC) $(OBJETOS) -o $(JOGO) $(LDLIBS)

# Compila cada .c; recompila se algum header mudar
%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

# Compila e roda os testes de terminal (salas, criatura e regras), sem raylib
teste: teste_salas.c teste_criatura.c teste_regras.c salas.c criatura.c regras.c fotogramas.c ordenacao.c $(HEADERS)
	$(CC) $(CFLAGS) teste_salas.c salas.c -o $(TESTE_SALAS)
	$(CC) $(CFLAGS) teste_criatura.c criatura.c salas.c -o $(TESTE_CRIATURA)
	$(CC) $(CFLAGS) teste_regras.c regras.c criatura.c salas.c fotogramas.c ordenacao.c -o $(TESTE_REGRAS)
	$(RODAR)$(TESTE_SALAS)
	$(RODAR)$(TESTE_CRIATURA)
	$(RODAR)$(TESTE_REGRAS)

# Apaga o que foi gerado pela compilação
clean:
	$(APAGAR) $(OBJETOS) $(JOGO) $(TESTE_SALAS) $(TESTE_CRIATURA) $(TESTE_REGRAS)

.PHONY: clean teste
