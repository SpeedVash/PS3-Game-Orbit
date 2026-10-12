# Teste da v1.5.6 no PS3

Instale `PS3_GAME_ORBIT_v1.5.6.gnpdrm.pkg` por cima da versão anterior. A identidade PGORBT301 e as configurações continuam as mesmas; APP_VER/VERSION é 01.16. Habilite HEN antes de abrir, quando necessário.

1. **Inicialização:** a imagem com “Carregando sua biblioteca...” deve permanecer durante a leitura do catálogo e a preparação das capas, interiores e discos iniciais. A biblioteca deve aparecer ao concluir essa preparação, sem um intervalo mostrando somente o fundo animado.
2. **Layouts salvos:** repita a abertura com Clássico, Spine e Lista. Confira o jogo e o filtro restaurados e alterne os layouts depois de abrir.
3. **HD/USB e imagens grandes:** confira a mesma espera com as imagens no HD ou USB. A duração acompanha o trabalho real; não há um atraso fixo acrescentado ao início.
4. **Biblioteca vazia ou sem artes:** deve abrir a biblioteca vazia ou as caixas neutras, sem esperar por imagens inexistentes. Um arquivo inválido também não deve impedir a conclusão.
5. **Saída durante o carregamento:** Círculo pode encerrar a preparação. Abra novamente e confira que a biblioteca inicia normalmente.
6. **Regressão:** navegue rapidamente, alterne os três layouts, abra a caixa com L3 e visite o gerenciador. O cache comum da v1.5.5 continua em uso.

O pré-carregamento continua limitado à janela inicial de até 15 jogos. Os demais jogos são preparados durante a navegação. O usuário confirmou que a v1.5.5 eliminou a pequena pausa ao trocar de layout; esta versão corrige a apresentação durante a espera inicial.

O log fica em `/dev_hdd0/tmp/PS3_GAME_ORBIT_FIX46.log`, com alternativa em `/dev_hdd0/game/PGORBT301/USRDIR/PS3_GAME_ORBIT_FIX46.log`. A linha `ARTWORK 1.5.6 preload: splash_retained=1` registra conclusão, total de artes e tempo de preparação. A continuidade visual desta correção precisa ser confirmada no PS3.
