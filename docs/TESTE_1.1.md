# Teste no PS3 — 1.1 / FIX31

Instale `PS3_GAME_ORBIT_v1.1_TESTE.gnpdrm.pkg` por cima da 1.0, no mesmo Slim com CFW em que a base já funcionou. O ID permanece `PGORBT301` e o SFO indica `01.01`.

1. Confira a abertura: tela de início habitual, fundo com ondas e ausência da tela rosa de diagnóstico.
2. No Clássico, confira as duas caixas menores. A caixa selecionada inicia no zoom `0,55`; a vizinha aparece inteira.
3. Troque alguns jogos para a direita e para a esquerda, inclusive invertendo a direção antes do movimento terminar. A caixa anterior deve sair gradualmente e a nova entrar, sem trocar a arte da caixa anterior durante a saída.
4. Pressione Quadrado: devem aparecer lombadas dos dois lados e a caixa selecionada à frente. Com onze jogos ou mais, aparecem cinco lombadas por lado. Bibliotecas menores não repetem jogos.
5. Navegue e gire a caixa com o analógico direito. Confira lombada, frente e verso da mesma capa completa. L2/R2 continua controlando o zoom; R3 restaura o tamanho mínimo e a posição frontal do layout.
6. Marque um favorito, use L1/R1, abra/feche a ajuda em SELECT e releia a biblioteca com START.
7. Saia com Círculo e abra novamente. Confira os favoritos e a escolha de layout.
8. Com webMAN MOD ativo, teste X em um jogo com TITLE_ID conhecido e confirme o retorno ao XMB após a montagem do disco correto.

Se aparecer tela preta, retorno inesperado ao XMB ou travamento, use o log `/dev_hdd0/tmp/PS3_GAME_ORBIT_FIX31.log`; a alternativa fica em `/dev_hdd0/game/PGORBT301/USRDIR/PS3_GAME_ORBIT_FIX31.log`. Informe em qual layout ocorreu e qual ação vinha antes da falha.

As imagens e animações desta entrega são simulações no computador. Não certificam desempenho, amostragem de texturas, contagem de comandos do driver ou saída de vídeo real.
