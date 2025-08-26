# Descrição
## Esse projeto é uma implementação do clássico jogo *Conway's Game of Life*, com uma interface gráfica e funcionalidades acrescidas.

# Regras
## A simulação segue as seguintes regras:
## - Espaços mortos com 3 vizinhos ganham vida.
## - Espaços vivos com 2 ou 3 vizinhos continuam vivos.
## - Espaços vivos com menos de 2 vizinhos morrem (morte por solidão).
## - Espaços vivos com mais de 3 vizinhos morrem (morte por superpopulação). 

# Funcionalidades extra
## - É possível editar a simulação enquanto ela está rodando: você pode criar ou destruir vida nos quadrados do tabuleiro.
## - Há um sistema de salvamento em arquivos, que permite que qualquer momento da simulação seja guardado e posteriormente retomado.

# Controles
## Space - Pausa a simulação.
## Backspace - Salva o estado atual em um novo arquivo.
## Mouse Esquerdo - Cria quadrados.
## Mouse Direito - Apaga Quadrados.
## Mouse Meio - Move o círculo que controla a velocidade da simulação (é preciso clicar dentro dele).

# Dependências
## Além das bibliotecas padrão de C++, o jogo usa a biblioteca *SDL2* para renderizar os gráficos e a *SDL2_ttf* para renderizar os textos da interface.
## Para que o texto funcione corretamente, é preciso que o arquivo "OpenSans.ttf" esteja na pasta do executável.