# PS3 Game Orbit

![PS3 Game Orbit](pkgfiles/ICON0.PNG)

**v1.5.1 — criado por SpeedVash.** Biblioteca de jogos PS3 com capas completas, caixas articuladas, layouts Clássico, Spine e Lista, favoritos e montagem pelo webMAN MOD.

## Novidades da v1.5.1

- **Gerenciador com dois painéis independentes**, para HD interno e USB. Escolha a origem em um painel e o destino no outro; copie um ou vários arquivos, com progresso e cancelamento.
- **Seleção múltipla e destaques**: Quadrado marca arquivos, SELECT marca/desmarca todos na pasta. Marcados aparecem em azul; cópias concluídas ficam em verde na origem e no destino.
- **Substituição confirmada**, com opções para sobrescrever ou ignorar um arquivo, aplicar a decisão ao lote inteiro ou cancelar.
- **Acesso aos arquivos revisado**: abertura, leitura e consulta do arquivo usam a mesma API nativa. A verificação separa os dados do caminho dos dados do descritor; erros mostram a etapa e o código retornado pelo sistema.
- **Tipo de caixa por jogo**, em Triângulo: padrão transparente, Platinum cinza opaco ou Greatest Hits vermelha. A cor muda o plástico externo do mesmo modelo; interior transparente, encarte e disco são preservados. A escolha fica salva.
- **Pré-carga adiada durante navegação e rotação manual**, incluindo um intervalo de 650 ms após o último comando e o término das transições. Logs e gravações de preferências também aguardam repouso; caches por layout mantidos.
- Mantidos A-Z/Z-A/Custom, ordem salva, caixa selecionada completa fechada/aberta com abertura de 160°, configurações, webMAN e atualização parcial/em lote das artes via USB.
- Versão **1.5.1** no início, cabeçalho e menu, com crédito a **SpeedVash**. Identidade PGORBT301 preservada.

Compilação nativa e testes no computador concluídos. **Teste desta versão no PS3 pendente.** A primeira carga das imagens necessárias ao desenho ainda é síncrona; as simulações não medem a fluidez real no console. A iluminação usa os shaders aprovados.

## Instalação

1. Copie `PS3_GAME_ORBIT_v1.5.1.gnpdrm.pkg` para a raiz de um pendrive FAT32.
2. Instale pelo gerenciador de pacotes do XMB, por cima da versão anterior. Identidade **PGORBT301**, `APP_VER 01.11`; preserve os arquivos de configuração existentes.
3. No Super Slim, habilite HEN antes de abrir. Para montar jogos, mantenha webMAN MOD ativo.
4. X monta o jogo com caixa aberta ou fechada. Após a confirmação, o app volta ao XMB; inicie pelo ícone do disco.

## Menus e ordenação

Prévias geradas da interface real no computador: [tela inicial](assets/previews/TELA_INICIAL_1_5_1.png), [opções](assets/previews/MENU_HOME_1_5_1.png), [jogo](assets/previews/MENU_JOGO_1_5_1.png) e [gerenciador](assets/previews/GERENCIADOR_1_5_1.png). Não são capturas de um PS3.

| Botão | Opções |
| --- | --- |
| Triângulo — jogo | Alterar nome · Copiar capas do USB · Recarregar capas · Adicionar/Remover dos favoritos · Tipo de caixa · Alterar posição, disponível em Custom |
| START — homebrew | Atualizar biblioteca · Fundo animado · Atualizar todas as capas via USB · Salvar último jogo visto · Listagem · Gerenciador de arquivos HD/USB |

Em START → Listagem, X alterna A até Z → Z até A → Custom. Em Custom, selecione o jogo e abra Triângulo → Alterar posição. O direcional escolhe a posição; L1/R1 saltam dez posições; X confirma e Círculo cancela. As posições mostradas são relativas ao filtro ativo. O jogo selecionado continua selecionado ao mudar a ordenação.

Os índices internos do catálogo e das texturas permanecem estáveis. Alterar nome muda apenas o título mostrado, sem renomear a ISO, pasta, ID ou arte; A-Z/Z-A são atualizados após salvar o nome.

## Gerenciador HD/USB

Abra START → Gerenciador de arquivos HD/USB. O aviso informa a pausa do fundo animado; X continua e Círculo volta à biblioteca. Cada painel guarda sua própria pasta e posição. Na raiz de um dispositivo, Círculo abre a lista de HD e USBs disponíveis; R3 atualiza a lista após conectar um pendrive.

| Botão | Ação |
| --- | --- |
| Cima / baixo | Selecionar arquivo ou pasta |
| Esquerda / direita ou L2 / R2 | Trocar o painel ativo |
| L1 / R1 | Voltar ou avançar oito entradas |
| X | Abrir pasta ou dispositivo |
| Quadrado | Marcar/desmarcar um arquivo |
| SELECT | Marcar/desmarcar todos os arquivos da pasta ativa |
| Triângulo | Copiar os marcados para a pasta do outro painel; sem marcações, copiar o arquivo destacado |
| Círculo | Pasta anterior; na raiz do dispositivo, escolher dispositivo; na lista de dispositivos, sair |
| R3 | Atualizar pastas e dispositivos |
| START | Sair do gerenciador |
| Círculo durante cópia | Cancelar o arquivo atual e o restante do lote |
| START durante cópia | Cancelar e sair |

É possível copiar HD → HD, HD → USB, USB → HD e entre USBs reconhecidos pelo sistema. A seleção pode reunir arquivos de pastas diferentes. Arquivos do lote com o mesmo nome devem ser copiados separadamente. Pastas servem para navegar; esta versão não faz cópia recursiva de pastas.

Arquivos marcados aparecem em azul e os já copiados em verde, até sair do gerenciador. Em conflitos, escolha sobrescrever um, ignorar um, sobrescrever todos, ignorar todos ou cancelar. A decisão para todos vale somente para aquele lote. O progresso mostra o arquivo atual e o total do lote.

A cópia usa blocos de 256 KiB e um temporário na pasta de destino. O destino anterior é mantido até a conclusão; falha ou cancelamento remove o temporário e preserva os arquivos já concluídos. Falha na troca final tenta restaurar o destino anterior; se isso também falhar, a mensagem informa o caminho do backup. A origem não é removida. Arquivos acima de 4 GB são aceitos no HD; USB FAT32 não permite um arquivo maior que 4 GiB menos 1 byte. NTFS/exFAT não têm suporte adicional nesta entrega.

Os bytes são copiados sem alteração. Para redimensionar artes, use a atualização de capas via USB da biblioteca. Durante o gerenciador, as caixas e a pré-carga ficam pausadas; ao sair, a preferência anterior do fundo e a visualização da biblioteca retornam.

Veja o [guia do gerenciador](docs/GERENCIADOR_1.5.1.md) para um exemplo completo.

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

Diagnóstico: `/dev_hdd0/tmp/PS3_GAME_ORBIT_FIX41.log`; alternativa em `USRDIR`. Consulte [o roteiro de teste](docs/TESTE_1.5.1.md) ao reportar um problema.

## Fontes, compilação e publicação

- [Compilar e executar os testes](docs/BUILD.md)
- [Guia para postar no GitHub](docs/GITHUB_1.5.1.md)
- [Notas prontas para o release](docs/RELEASE_NOTES_1.5.1.md)
- [Validação e teste no PS3](docs/TESTE_1.5.1.md)
- [Histórico](CHANGELOG.md) e [créditos/licenças](CREDITS.md)

O PKG já está compilado. O ZIP público contém código, shaders arquivados, ativos próprios e relatórios. A malha JFX licenciada e sua representação editável não são redistribuídas. Para compilar, obtenha sua cópia na fonte indicada em `assets/jfx_bluray/README.md` e execute `scripts/prepare-jfx.py`. O código procedural do interior e das articulações acompanha o projeto. Prévias de versões anteriores mantidas em `assets/previews` são imagens de referência geradas no computador, não capturas desta versão no console.
