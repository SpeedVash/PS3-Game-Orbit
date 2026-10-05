# PS3 Game Orbit

![PS3 Game Orbit](pkgfiles/ICON0.PNG)

**v1.3.3-rc.1 — criado por SpeedVash.** Biblioteca de jogos PS3 com caixas 3D, capas completas, layouts Clássico/Spine/Lista, favoritos e montagem pelo webMAN MOD.

## Novidades da 1.3.3

- **Interior e disco preparados durante a espera:** após 900 ms no mesmo jogo, prepara primeiro o Inside Cover Full e depois o disco, uma imagem por etapa. Abrir com L3 reaproveita as texturas que já estiverem prontas.
- **Buffers reutilizados:** memória de decodificação e blocos liberados de texturas são reaproveitados, com limites de retenção. Texturas visíveis continuam protegidas.
- **Pré-carga conforme o layout:** Lista considera três vizinhos, Clássico cinco e Spine até quatorze. A direção da navegação recebe prioridade.
- **USB em `PS3COVERS`:** copia qualquer combinação de capa, interior e disco. Não exige as três imagens e preserva os tipos ausentes.
- **Importação para ISO:** aceita a ID reconhecida ou o nome exato do arquivo ISO, sem a extensão.
- **Fundo animado inspirado no XMB:** três ondas suaves, com opção de ligar/desligar no menu START. Desligado mantém a imagem estática; a escolha fica salva.
- **Caches mantidos em 15 capas / 15 interiores / 15 discos.** Mantidas as otimizações da 1.3.2: interface parcial, decodificadores carregados, logs em lotes e medições em milissegundos.

A compilação nativa e os testes automatizados no computador foram concluídos. **O teste físico desta v1.3.3 no PS3 está pendente.** A primeira leitura/decodificação de uma imagem continua síncrona; a pré-carga não usa uma thread separada nem garante ausência de pausas.

## Instalação

1. Copie `PS3_GAME_ORBIT_v1.3.3_TESTE.gnpdrm.pkg` para um pendrive FAT32 e instale pelo gerenciador de pacotes do XMB, por cima da versão anterior.
2. A identidade permanece `PGORBT301`; nomes personalizados, favoritos e layout continuam nos mesmos arquivos. O SFO usa `APP_VER 01.06` para avançar sobre a 1.3.2 (`01.05`).
3. No Super Slim, habilite HEN antes de abrir. Mantenha o webMAN MOD ativo para a montagem dos jogos.
4. Quadrado alterna os layouts; L3 abre/fecha a caixa. X monta pelo webMAN e volta ao XMB após a confirmação. Abra o jogo pelo ícone do disco no XMB.

## Menu START

| Opção | Comportamento |
| --- | --- |
| Alterar nome do jogo | Abre o teclado do PS3 e salva apenas o nome exibido no Game Orbit. Não modifica pasta, ISO, ID, PARAM.SFO ou nomes das imagens. |
| Recarregar capas | Relê as três artes do jogo selecionado, inclusive tentativas que falharam. Preserva os caches dos demais jogos. |
| Copiar capas do pendrive | Valida e copia as imagens encontradas em `USB/PS3COVERS`, depois invalida somente os tipos atualizados. |
| Fundo animado: Ligado/Desligado | Alterna as ondas animadas e o fundo estático. Ligado por padrão; preferência salva para a próxima abertura. |
| Atualizar biblioteca | Reescaneia os jogos e limpa os três caches, mantendo nomes, favoritos, layout e escolha do fundo. |

Cima/baixo selecionam, X confirma, Círculo ou START fecham o menu. Também funciona com a caixa aberta. Com a biblioteca vazia, START permite configurar o fundo e atualizar a biblioteca; ações de jogo informam que não há título selecionado.

## Atualizar imagens por pendrive

Crie uma pasta chamada **`PS3COVERS` na raiz do pendrive FAT32**. Coloque as imagens diretamente nessa pasta:

| Imagem | Com ID `BLES01287` | Para `Bayonetta.iso`, sem ID reconhecida |
| --- | --- | --- |
| Capa exterior completa | `PS3COVERS/BLES01287.jpg` | `PS3COVERS/Bayonetta.jpg` |
| Inside Cover Full | `PS3COVERS/BLES01287_INSIDE.jpg` | `PS3COVERS/Bayonetta_INSIDE.jpg` |
| Rótulo do disco | `PS3COVERS/BLES01287_DISC.png` | `PS3COVERS/Bayonetta_DISC.png` |

Selecione o jogo e use **START → Copiar capas do pendrive**. A busca ocorre em `/dev_usb000` até `/dev_usb007`, somente dentro de `PS3COVERS`, sem subpastas e sem procurar imagens soltas na raiz do USB. O PKG de instalação continua podendo ficar na raiz do pendrive.

**Uma imagem já basta.** Se houver somente a capa exterior, ela é copiada e as artes de interior/disco anteriores são preservadas. O mesmo vale para apenas inside, apenas disco ou qualquer par. As imagens também podem estar distribuídas entre os dispositivos USB reconhecidos.

Para ISO, use o nome exato sem `.iso`/`.ISO`, preservando espaços e maiúsculas/minúsculas. Exemplo: `My ISO Game.ISO` → `My ISO Game.jpg`, `My ISO Game_INSIDE.jpg`, `My ISO Game_DISC.png`. Quando a ID é conhecida, ela tem prioridade na busca; arquivos pelo nome da ISO também são aceitos e salvos no HDD com essa ID. Sem ID conhecida, o destino usa o nome da ISO.

Cada imagem é validada e substituída separadamente, com arquivo temporário e backup. Um tipo ausente ou corrompido mantém a arte anterior; os outros tipos válidos ainda podem ser copiados. O menu informa a quantidade atualizada e eventuais erros. Formatos concorrentes antigos são removidos somente do tipo atualizado.

## Imagens no HDD

Pasta principal: **`/dev_hdd0/PS3COVERS`**. Jogos em pasta usam a TITLE_ID conhecida. **A ID interna dos ISOs ainda não é lida:** o scanner reconhece uma ID no nome do ISO com quatro letras e cinco números juntos, como `BLES01287`, sem hífen. Para ISO sem ID reconhecida, use o nome exato do arquivo sem extensão.

Uma imagem nomeada apenas pela ID não é encontrada para `Bayonetta.iso` se o scanner não conhecer essa ID. Não é necessário renomear o ISO: use `Bayonetta` nos nomes das imagens. A resolução prioriza a ID na pasta global, depois o nome exato na pasta global e os fallbacks locais do jogo.

JPG, JPEG e PNG são aceitos, inclusive extensões maiúsculas. Os sufixos **`_INSIDE`** e **`_DISC`** devem estar em maiúsculas. Prefira ter apenas um formato para cada tipo; inside/disc priorizam PNG na resolução quando há vários.

A capa exterior inteira segue **verso · lombada · frente**. O inside segue **interior esquerdo · centro/lombada · interior direito**, sem espelhar a imagem. O mapeamento orienta a tampa ao abrir. O centro ocupa aproximadamente 46,22% a 53,79% da largura. Veja o [modelo de 1000 × 535](assets/examples/INSIDE_COVER_FULL_MODELO_1000x535.png) e o [exemplo identificado](assets/examples/INSIDE_COVER_FULL_EXEMPLO.png).

Use capas exterior/interior com aproximadamente **1000 pixels de largura** e discos de **512 × 512**. A geometria recorta o contorno circular e o furo central do disco. A arte chega até o furo físico; não há anel transparente artificial. Sem inside válido, o interior fica neutro; sem disco válido, aparece o rótulo padrão. Isso não impede montar o jogo.

Após uma cópia manual no HDD, use **START → Recarregar capas**. Nome personalizado no menu não muda o nome esperado dos arquivos de arte. SELECT continua mostrando somente controles.

## Controles

| Controle | Ação |
| --- | --- |
| Esquerda/direita ou analógico esquerdo | Trocar jogo |
| Cima/baixo ou analógico esquerdo vertical, na Lista | Trocar linha |
| Quadrado | Clássico / Spine / Lista |
| L3 | Abrir / fechar caixa e disco |
| X, caixa aberta ou fechada | Montar pelo webMAN MOD e voltar ao XMB após confirmação |
| Círculo, caixa aberta e sem montagem | Recolher disco e fechar caixa |
| Círculo, durante montagem | Cancelar operação e sair ao XMB |
| Círculo, caixa fechada | Sair ao XMB |
| Analógico direito | Girar / inclinar caixa |
| Triângulo | Marcar / remover favorito |
| L1 / R1 | Todos / HDD / USB / Favoritos |
| Cima, no Clássico ou Spine | Ativar / pausar autogiro |
| L2 / R2 | Zoom |
| R3 | Restaurar posição e zoom mínimo |
| SELECT | Abrir / fechar ajuda de controles |
| START | Abrir / fechar opções do jogo e fundo |

Durante a inspeção, navegação, filtros, layout e favoritos ficam bloqueados. X monta e START abre o menu do mesmo jogo. Dentro do menu, X confirma a opção. Durante montagem, um segundo X não cria outra solicitação.

## Pré-carga e memória

Os três caches guardam até **15 texturas válidas por tipo**, por ordem de uso recente. Cada cache tem limite de **64 MiB**. O cache de arquivos codificados das capas guarda 15 entradas / 16 MiB. Uma textura de 1024 × 1024 ocupa aproximadamente 4 MiB; texturas menores usam menos memória.

A pré-carga de capas começa após 400 ms de repouso, com intervalo mínimo de 200 ms entre etapas. A Lista considera dois vizinhos na direção de navegação e um no sentido oposto; Clássico considera três e dois; Spine até sete por lado. As caixas visíveis e em transição permanecem protegidas, e uma pré-carga já completa não entra em um ciclo de descarte/releitura.

Após 900 ms no mesmo jogo, interior e disco do **jogo selecionado** recebem prioridade. Cada etapa prepara no máximo uma imagem: primeiro inside, depois disco, depois continua com os vizinhos. A pré-carga em repouso pausa com o menu aberto ou a caixa aberta/em animação. Se L3 for pressionado antes de as artes estarem prontas, a preparação ocorre em quadros sucessivos, sem decodificar as duas no mesmo quadro.

Blocos liberados de texturas são reutilizados por tamanho, com até **8 MiB / três blocos livres**. Apenas memória que já foi liberada entra nesse pool. O cache contabiliza o tamanho real do bloco reutilizado, incluindo espaço excedente. Buffers RGBA/ARGB mantêm capacidade de até 4 MiB cada após preparações bem-sucedidas; alocações maiores são descartadas. Os decodificadores do SDK ainda podem alocar memória temporária internamente.

O limite somado dos três caches de texturas é 192 MiB. Pool, arquivos codificados, decodificação temporária, geometria, HUD e vídeo usam memória adicional. Texturas são limitadas a 1024 pixels por lado depois de decodificadas; imagens acima de 8192 pixels por lado ou 16 milhões de pixels são rejeitadas. Não há cache persistente de imagens ou miniaturas em disco.

Leitura, decode e upload continuam no fluxo principal, entre quadros confirmados pela GPU. Imagens residentes evitam esse trabalho. As medições no próprio PS3 permitem avaliar a melhora real com o mesmo conjunto de capas.

## Fundo animado

As ondas são uma implementação própria inspirada no XMB: três faixas neutras, 582 vértices e 576 triângulos, sobre um gradiente. As texturas são criadas e enviadas uma vez; a cada quadro, somente as posições dos vértices são atualizadas, entre quadros confirmados. Não há decodificação de uma imagem do fundo a cada quadro.

**START → Fundo animado → X** liga ou desliga. Desligado usa a mesma imagem estática da versão anterior e pausa o movimento. A preferência fica em `PS3_GAME_ORBIT_BACKGROUND.dat`, separada das preferências de jogos.

## Medições e diagnóstico

Log principal: **`/dev_hdd0/tmp/PS3_GAME_ORBIT_FIX36.log`**; fallback no `USRDIR` do aplicativo. Saia normalmente para gravar o último lote. O log continua limitado a 64 KiB pendentes, com gravação em repouso a intervalos mínimos de três segundos e ao encerrar.

As linhas `PERF 1.3.3` registram `count`, `total_ms`, `average_ms` e `max_ms`, acumulados desde a abertura:

| stage | Escopo |
| --- | --- |
| read | Leitura do arquivo e identificação do cabeçalho; acerto no cache evita nova leitura. |
| decode | PNG/JPEG e cópia do resultado do decodificador. |
| upload | Preparação, orientação/redução quando necessárias, aquisição do buffer, cópia e mapeamento da textura. Não mede a conclusão da GPU. |
| interface | Redesenho e cópia das regiões alteradas; quadros sem mudança não entram no contador. |

`OPT 1.3.3` registra fundo ligado/desligado, desenhos de ondas, alocações, reutilizações e bytes livres no pool. Esses contadores auxiliam a comparação; não são uma medição completa do tempo de quadro.

Arquivos de estado em `/dev_hdd0/tmp/`: `PS3_GAME_ORBIT_STATE.dat`, `PS3_GAME_ORBIT_LAYOUT.dat`, `PS3_GAME_ORBIT_NAMES.dat` e `PS3_GAME_ORBIT_BACKGROUND.dat`. Todos têm fallback no `USRDIR` de `PGORBT301`.

## Fontes, testes e créditos

Veja [teste físico da 1.3.3](docs/TESTE_1.3.3.md), [validação](docs/VALIDATION.md), [compilação](docs/BUILD.md), [guia do GitHub](docs/GITHUB_1.3.3.md), [histórico](CHANGELOG.md) e [créditos](CREDITS.md).

O scanner lê `GAMES`, `GAMEZ` e `PS3ISO` no HDD e USB 000–007. HTTP local do webMAN na porta 80; interface 1280 × 720. NTFS, rede e leitura do PARAM.SFO dentro de ISOs não foram adicionados nesta versão.

O ZIP completo inclui fontes públicos, ativos do aplicativo, PKG e relatórios. Obtenha o modelo JFX separadamente para compilar: OBJ/PSD e sua malha editável derivada são excluídos dos fontes públicos, como nas versões anteriores. Os testes públicos e o workflow usam uma malha de teste quando necessário e não publicam releases automaticamente.
