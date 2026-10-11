# PS3 Game Orbit

![PS3 Game Orbit](pkgfiles/ICON0.PNG)

**v1.5.4 — criado por SpeedVash.** Biblioteca de jogos PS3 com capas completas, caixas articuladas, layouts Clássico, Spine e Lista, favoritos e montagem pelo webMAN MOD.

## Reaproveitamento entre layouts

As capas e artes recentes permanecem em memória enquanto houver espaço. Ao mudar para um layout menor, a redução do cache prioriza o jogo selecionado e os vizinhos que serão usados. Os arquivos de imagem já lidos seguem a mesma prioridade. Isso reduz recarregamentos ao trocar layouts e voltar a jogos recentes.

Mantidos os limites Clássico 7/7/7, Spine 15/15/15 e Lista 5/5/5. A preparação de imagens novas continua a consumir leitura, decodificação e upload. Veja [validação](docs/VALIDACAO_1.5.4.md) e [teste no PS3](docs/TESTE_1.5.4.md).

## Gerenciador corrigido

A geometria própria introduzida na v1.5.3 conserva a tela inteira do gerenciador. O usuário confirmou que ela funcionou bem no PS3. A biblioteca recupera seu desenho normal ao sair.

## Recursos do gerenciador

- **Controles mais claros:** L1/R1 escolhem o painel, Triângulo abre as ações e R3 escolhe HD ou USB.
- **Origem fixa:** marcar arquivos ou pastas vincula a seleção ao painel e à pasta de origem. Abrir o destino no outro painel preserva a direção da cópia.
- **Confirmação antes de gravar:** o app conta arquivos, subpastas e tamanho total, mostrando origem e destino para você conferir.
- **Cópia de pastas completas**, incluindo subpastas e pastas vazias. Pastas existentes são combinadas; arquivos existentes têm confirmação de substituição.
- **Listagem mais legível:** nomes maiores, borda para o item em foco, caixas de seleção azuis e indicação verde de cópia concluída. SELECT mostra o caminho completo.
- **Progresso e resumo:** arquivo atual, lote inteiro, arquivos copiados, ignorados e erros. START pede confirmação antes de cancelar uma operação em andamento.
- Mantidos estilos de caixa por jogo, A-Z/Z-A/Custom, configurações, webMAN, artes via USB, caches e pausa temporária do fundo.
- Versão **1.5.4** no início, cabeçalho e menu, com crédito a **SpeedVash**. Identidade PGORBT301 preservada.

Compilação nativa e testes no computador concluídos. **Teste desta versão no PS3 pendente.** A primeira carga das imagens necessárias ao desenho ainda é síncrona; as simulações não medem a fluidez real no console. A iluminação usa os shaders aprovados.

## Instalação

1. Copie `PS3_GAME_ORBIT_v1.5.4.gnpdrm.pkg` para a raiz de um pendrive FAT32.
2. Instale pelo gerenciador de pacotes do XMB, por cima da versão anterior. Identidade **PGORBT301**, `APP_VER 01.14`; preserve os arquivos de configuração existentes.
3. No Super Slim, habilite HEN antes de abrir. Para montar jogos, mantenha webMAN MOD ativo.
4. X monta o jogo com caixa aberta ou fechada. Após a confirmação, o app volta ao XMB; inicie pelo ícone do disco.

## Menus e ordenação

Prévia do [gerenciador corrigido](assets/previews/GERENCIADOR_1_5_3.png), calculada com os pixels e o mapeamento de tela usados pelo desenho nativo. É uma simulação no computador, não uma captura do PS3.

| Botão | Opções |
| --- | --- |
| Triângulo — jogo | Alterar nome · Copiar capas do USB · Recarregar capas · Adicionar/Remover dos favoritos · Tipo de caixa · Alterar posição, disponível em Custom |
| START — homebrew | Atualizar biblioteca · Fundo animado · Atualizar todas as capas via USB · Salvar último jogo visto · Listagem · Gerenciador de arquivos HD/USB |

Em START → Listagem, X alterna A até Z → Z até A → Custom. Em Custom, selecione o jogo e abra Triângulo → Alterar posição. O direcional escolhe a posição; L1/R1 saltam dez posições; X confirma e Círculo cancela. As posições mostradas são relativas ao filtro ativo. O jogo selecionado continua selecionado ao mudar a ordenação.

Os índices internos do catálogo e das texturas permanecem estáveis. Alterar nome muda apenas o título mostrado, sem renomear a ISO, pasta, ID ou arte; A-Z/Z-A são atualizados após salvar o nome.

## Gerenciador HD/USB

Abra START → Gerenciador de arquivos HD/USB. X confirma o aviso e abre os dois painéis. Cada painel guarda sua própria pasta e posição.

| Botão | Ação |
| --- | --- |
| Cima / baixo | Escolher arquivo ou pasta |
| L1 / R1 | Escolher painel esquerdo / direito |
| Esquerda / direita | Atalho para escolher o painel |
| L2 / R2 | Voltar / avançar sete entradas |
| X | Abrir pasta ou confirmar |
| Quadrado | Marcar/desmarcar arquivo ou pasta |
| Triângulo | Ações: copiar, marcar todos, limpar seleção, atualizar |
| R3 | Escolher HD ou USB para o painel ativo |
| SELECT | Ver o caminho completo; L2/R2 percorrem suas páginas |
| Círculo | Pasta anterior ou voltar; durante preparação/cópia, cancelar |
| START | Sair; durante operação, confirmar cancelamento e saída |

Marque a origem com Quadrado. Use L1/R1 para passar ao outro painel e abra a pasta de destino. Em qualquer painel, Triângulo → Copiar usa a origem marcada. Sem marcações, copia o item em foco no painel ativo. Confira quantidade, tamanho e caminhos; somente X na confirmação inicia a gravação.

As marcações pertencem a uma única pasta de origem. Abrir outra pasta nesse mesmo painel limpa as marcações; navegar no destino as preserva. Para escolher uma nova origem no outro painel, use Triângulo → Limpar seleção. Marcar todos inclui arquivos e pastas. O menu mostra a direção da cópia.

É possível copiar HD → HD, HD → USB, USB → HD e entre USBs montados pelo sistema. Pastas são copiadas com todo o conteúdo, incluindo pastas vazias. Se a pasta já existir, seus arquivos não relacionados são preservados. Cada conflito de arquivo permite sobrescrever, pular, sobrescrever todos, pular todos ou cancelar; a escolha para todos vale somente para o lote atual.

A cópia usa blocos de 256 KiB e um temporário por arquivo na pasta de destino. O arquivo anterior é mantido até a conclusão; falha ou cancelamento remove o temporário e preserva as cópias concluídas. Pastas já criadas também permanecem. Falha na troca final tenta restaurar o destino anterior; a mensagem informa o backup se a restauração falhar. A origem não é removida.

A preparação ocorre por etapas e pode ser cancelada sem gravar. Limites por lote: 4096 itens marcados, 16384 arquivos/pastas preparados, 8 MiB para seus caminhos e 1023 bytes por caminho. Se excedidos, copie subpastas em lotes menores. Copiar uma pasta para dentro dela mesma, itens não suportados ou conflitos entre arquivo e pasta é recusado antes da gravação.

Tamanhos usam 64 bits. Um arquivo acima de 4 GB pode ser copiado para HD se houver espaço; USB FAT32 aceita no máximo 4 GiB menos 1 byte por arquivo. NTFS/exFAT não têm suporte adicional nesta entrega. Os bytes são copiados sem alteração; para redimensionar artes, use a atualização de capas da biblioteca.

Durante o gerenciador, fundo animado, caixas e pré-carga ficam pausados. Ao sair, o fundo retorna à preferência anterior e a biblioteca volta a aparecer. Veja o [guia do gerenciador](docs/GERENCIADOR_1.5.2.md).

## Tipo de caixa

Selecione o jogo, abra Triângulo → Tipo de caixa e pressione X para alternar:

| Opção | Plástico externo | Interior |
| --- | --- | --- |
| Padrão | Transparente, sem cor | Transparente |
| Platinum | Cinza opaco | Transparente |
| Greatest Hits | Vermelho opaco | Transparente |

A geometria e as artes são as mesmas nos três estilos. A escolha acompanha o jogo nos layouts Clássico, Spine e Lista, com a caixa fechada ou aberta. Ela é salva com as preferências do jogo; dados das versões anteriores recebem o estilo padrão.

## Imagens no pendrive e no HDD

Crie **PS3COVERS na raiz do pendrive**. As imagens devem ficar diretamente nessa pasta, sem subpastas.

| Arte | Exemplo com ID | ISO sem ID reconhecida | Saída no HDD |
| --- | --- | --- | --- |
| Capa completa | `PS3COVERS/BLES01287.jpg` | `PS3COVERS/Bayonetta.jpg` | 1000 × 550, JPG |
| Inside Cover Full | `PS3COVERS/BLES01287_INSIDE.jpg` | `PS3COVERS/Bayonetta_INSIDE.jpg` | 1000 × 550, JPG |
| Disco | `PS3COVERS/BLES01287_DISC.png` | `PS3COVERS/Bayonetta_DISC.png` | 500 × 500, PNG |

Use **Triângulo → Copiar capas do USB** para o jogo atual, ou **START → Atualizar todas as capas via USB** para a biblioteca inteira, incluindo jogos fora do filtro atual.

**Uma imagem já basta.** Tipos ausentes ou inválidos preservam a arte anterior; os outros tipos válidos ainda são copiados. A busca percorre `/dev_usb000` até `/dev_usb007`, somente dentro de `PS3COVERS`. JPG, JPEG e PNG são aceitos como origem, inclusive extensões maiúsculas. `_INSIDE` e `_DISC` devem permanecer em maiúsculas. A origem no USB fica intacta.

O destino é **`/dev_hdd0/PS3COVERS`**. A cópia usa dimensões exatas, sem manter proporções diferentes: prepare a arte no aspecto 1000:550 para evitar distorção. A transparência do PNG do disco é preservada; capa/inside são convertidos para JPG. Arquivos antigos concorrentes são removidos apenas para o tipo atualizado, após gravação e troca com backup.

Para ISO, use o nome exato sem `.iso`/`.ISO`, preservando espaços e maiúsculas/minúsculas. Exemplo: `My ISO Game.ISO` → `My ISO Game_DISC.png`. Uma ID reconhecida tem prioridade; a origem pelo nome da ISO também é aceita e gravada no HDD com a ID. Sem ID, o destino mantém o nome da ISO. A ID interna do ISO ainda não é lida; o scanner reconhece uma ID de quatro letras e cinco números juntos no nome do arquivo.

Jogos em pasta usam a TITLE_ID. Renomear um jogo pelo menu não muda os nomes esperados das imagens. Cópias manuais no HDD não são redimensionadas: use Triângulo → Recarregar capas depois de copiá-las.

Modelos de arte: [Inside Cover Full 1000×550](assets/examples/INSIDE_COVER_FULL_MODELO_1000x550.png) e [disco 500×500](assets/examples/DISC_EXEMPLO_500.png).

A capa completa segue **verso · lombada · frente**. O inside segue **interior esquerdo · lombada · interior direito**, usando a mesma área e o mesmo recorte central. O disco é recortado pela geometria circular com furo de 15 mm. A arte cobre a região opaca de raio 8,0 até 59,5 mm. A borda de 7,5 a 8,0 mm e a de 59,5 a 60,0 mm ficam transparentes. Sem imagens opcionais, o app usa o interior neutro e o disco padrão.

## Controles

| Controle | Função |
| --- | --- |
| Esquerda/direita ou analógico esquerdo | Trocar de jogo |
| Cima/baixo no layout Lista | Navegar na lista |
| Analógico direito | Girar/inclinar a caixa |
| Quadrado | Alternar Clássico → Spine → Lista |
| L1 / R1 | Alternar Todos / Favoritos / USB / HD |
| L3 | Abrir/fechar a caixa e retirar/recolocar o disco |
| X | Montar o jogo pelo webMAN MOD |
| Triângulo | Opções do jogo |
| START | Opções do homebrew |
| L2 / R2 | Zoom |
| R3 | Restaurar a visualização |
| Cima em Clássico/Spine | Rotação automática |
| SELECT | Ajuda dos controles |
| Círculo | Fechar menu/caixa, cancelar operação ou sair |

## Caches e pré-carga

| Layout | Jogos anteriores | Atual | Jogos seguintes | Limite de cada cache |
| --- | ---: | ---: | ---: | ---: |
| Clássico | 3 | 1 | 3 | 7 capas, 7 inside, 7 discos |
| Lista | 2 | 1 | 2 | 5 capas, 5 inside, 5 discos |
| Spine | Recentes | Atual | Recentes | 15 capas, 15 inside, 15 discos |

Janelas circulares seguem o filtro ativo e não repetem jogos em bibliotecas curtas. A pré-carga considera a direção da navegação e aguarda 650 ms sem comandos manuais, além do término das transições. Após 900 ms de espera também prepara inside/disco. Girar ou inclinar a caixa, trocar de jogo, usar zoom ou pressionar botões reinicia a espera; rotação automática não impede a pré-carga. Na entrada do app, as três artes do jogo selecionado são preparadas. Ao navegar, inside/disco são preparados durante a espera; ao abrir a caixa, ambos são preparados antes do desenho. Uma imagem é preparada por etapa, alternando trabalho de capa e interior/disco. Spine mantém o cache de artes recentes e prepara inside/disco do jogo atual. Caixas que estão terminando a transição mantêm sua capa até o último quadro reconhecido pela GPU; esse período pode reter uma capa adicional ao limite de repouso.

As imagens que já estão no cache são reutilizadas. Se faltar uma imagem necessária ao desenho ou à abertura da caixa, seu carregamento ainda pode causar uma pausa inicial; o ajuste evita trabalho opcional concorrendo com os comandos manuais. Buffers de decodificação e de texturas liberadas continuam sendo reaproveitados. Os logs seguem agrupados e incluem tempo de leitura, decodificação, upload e interface, em milissegundos.

## Configurações e compatibilidade

Preferências unificadas: `/dev_hdd0/game/PGORBT301/USRDIR/PS3_GAME_ORBIT_SETTINGS.dat`. Arquivos de nomes, favoritos/seleção, layout, fundo e ordenação usam o mesmo diretório. A ordem fica em `PS3_GAME_ORBIT_ORDER.dat`, com checksum e backup `.bak`. Os dados antigos de `/dev_hdd0/tmp` continuam sendo lidos para migração. Cada gravação usa um temporário sincronizado e uma cópia `.bak` da versão anterior; o PS3 não precisa sobrescrever um arquivo via rename. Se o arquivo principal estiver ausente, a leitura usa o backup. Para restaurar padrões manualmente, remova o arquivo de configuração e sua cópia `.bak`. Arquivos de favoritos/seleção, nomes, layout e fundo das versões anteriores continuam compatíveis. As preferências antigas são migradas para USRDIR quando ainda não existe uma configuração atual válida.

“Salvar último jogo visto” vem ligado para preservar o comportamento anterior. Desligado, o app começa no primeiro jogo do filtro salvo. Se o último jogo não estiver disponível, começa no primeiro disponível. Alterações são gravadas após um breve intervalo e ao sair normalmente.

Diagnóstico: `/dev_hdd0/tmp/PS3_GAME_ORBIT_FIX42.log`; alternativa em `USRDIR`. Consulte [o roteiro de teste](docs/TESTE_1.5.3.md) ao reportar um problema.

## Fontes, compilação e publicação

- [Compilar e executar os testes](docs/BUILD.md)
- [Notas prontas para o release](docs/RELEASE_NOTES_1.5.3.md)
- [Teste no PS3](docs/TESTE_1.5.3.md)
- [Registro de validação](docs/VALIDACAO_1.5.3.md)
- [Histórico](CHANGELOG.md) e [créditos/licenças](CREDITS.md)

O PKG já está compilado. O ZIP público contém código, shaders arquivados, ativos próprios e relatórios. A malha JFX licenciada e sua representação editável não são redistribuídas. Para compilar, obtenha sua cópia na fonte indicada em `assets/jfx_bluray/README.md` e execute `scripts/prepare-jfx.py`. O código procedural do interior e das articulações acompanha o projeto. Prévias de versões anteriores mantidas em `assets/previews` são imagens de referência geradas no computador, não capturas desta versão no console.
