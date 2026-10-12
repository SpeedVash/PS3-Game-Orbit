# Teste da v1.5.5 no PS3

Instale `PS3_GAME_ORBIT_v1.5.5.gnpdrm.pkg` por cima da versão anterior. PGORBT301 e os arquivos de configuração são preservados. Habilite HEN antes de abrir, quando necessário.

1. **Inicialização:** confira o contador “imagens preparadas”. A biblioteca abre depois de preparar as artes disponíveis de até 15 jogos próximos ao selecionado. Imagens ausentes ou inválidas usam a aparência neutra. O tempo depende do HD/USB e das imagens; não há duração fixa. Círculo pode encerrar durante a preparação.
2. **Layouts no mesmo jogo:** alterne Clássico → Spine → Lista → Clássico várias vezes. As três visualizações conservam o mesmo cache. Compare a resposta com a v1.5.4, incluindo as primeiras trocas após abrir.
3. **Navegação rápida:** passe por vários jogos e circule nas extremidades da lista. Uma arte ainda pendente pode aparecer neutra e ser substituída ao terminar. Parar num jogo deve concluir sua capa, interior e disco; a fila não deve travar os comandos de navegação.
4. **Caixa e disco:** abra/feche com L3. Confira a arte do jogo selecionado no interior e no disco, sem misturar jogos depois de navegar rapidamente.
5. **Gerenciador:** abra pelo menu do homebrew, confira a tela inteira e volte à biblioteca. A preparação de novos pedidos pausa; as imagens já residentes continuam disponíveis ao voltar. Copiar e importar arquivos têm o custo próprio dessas operações.
6. **Recarga:** use “Recarregar capas” ou importe as artes de um jogo. Confira que a nova imagem substitui a anterior e que resultados antigos da fila não reaparecem. Atualizar a biblioteca deve invalidar pedidos do catálogo anterior.
7. **Biblioteca pequena/vazia:** um filtro com poucos jogos prepara apenas esses jogos, sem duplicações. Um filtro vazio não deve esperar por imagens inexistentes.
8. **Saída:** saia normalmente, ao navegar, e durante a preparação inicial. O worker termina antes de descarregar os módulos de imagem.

Se ainda houver pausas, descreva a ação, layout, quantidade aproximada de jogos, origem HD/USB e se ocorre só na primeira vez ou também ao repetir. O log fica em `/dev_hdd0/tmp/PS3_GAME_ORBIT_FIX45.log`, com alternativa em `/dev_hdd0/game/PGORBT301/USRDIR/PS3_GAME_ORBIT_FIX45.log`. A linha `ARTWORK 1.5.5 preload` registra o total e o tempo real da preparação.

Os testes no computador verificam cache, concorrência, pixels e ordem de preparação dos quadros. A melhora perceptível e o tempo de inicialização ainda precisam do teste físico. PS1 permanece em hiato nesta versão.
