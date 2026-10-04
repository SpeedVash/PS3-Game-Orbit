# Teste no PS3 — 1.2 / FIX32

Instale por cima da versão anterior. Habilite HEN quando aplicável. O pacote mantém `PGORBT301`, com versão SFO `01.02`.

1. Confira splash, fundo e ausência da tela rosa. No Clássico devem continuar as duas caixas menores.
2. Com Quadrado, confira Spine e Lista. A Lista mostra títulos à esquerda, destaque na seleção e a caixa à direita. Teste cima/baixo, analógico e início/fim da lista.
3. Troque entre alguns jogos com capa e volte aos anteriores. Depois de preparados, os residentes devem carregar sem repetir a decodificação. Espere um instante em um jogo e teste os vizinhos pré-carregados.
4. Navegue pelos 57 jogos, incluindo títulos sem capa. Confira que a arte acompanha cada caixa durante a transição. O cache deve permanecer dentro do limite e descartar antigas quando necessário.
5. Pressione L3. A caixa vem à frente, a tampa abre e o disco sai. Use L3 de novo ou Círculo/X: o disco deve retornar antes da tampa fechar. Inverta L3 no meio da animação e gire com o analógico direito.
6. Enquanto abre/fecha, X e Círculo só retornam a caixa; não montam o jogo nem saem do aplicativo. Depois de fechada, X monta e Círculo sai normalmente.
7. Teste favoritos, filtros, SELECT e START. START limpa o cache e relê a biblioteca. Feche e reabra para confirmar favoritos e o layout salvo.
8. Com webMAN MOD ativo, monte um jogo com TITLE_ID conhecido e confira o retorno ao XMB após confirmar o disco.

Use as mesmas capas reduzidas a 1000 pixels que melhoraram o teste anterior. Compare as primeiras leituras com retornos a capas recentes. Cache em memória não elimina o custo da primeira decodificação nem persiste após fechar o aplicativo.

Em caso de falha, consulte `/dev_hdd0/tmp/PS3_GAME_ORBIT_FIX32.log` ou `/dev_hdd0/game/PGORBT301/USRDIR/PS3_GAME_ORBIT_FIX32.log`. Informe layout, ação anterior e se foi primeira leitura ou retorno a uma capa.

Prévias e testes no computador não substituem este teste físico, especialmente no Super Slim/HEN.
