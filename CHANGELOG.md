# Histórico de versões

## 1.3.3-rc.1 — TESTE / FIX36

- Pré-carga do Inside Cover Full e disco do jogo selecionado durante o repouso, uma imagem por etapa, antes da abertura com L3.
- Reutilização de buffers RGBA/ARGB e de blocos liberados de texturas, com pool limitado a 8 MiB/três blocos e contabilização da alocação real.
- Pré-carga por layout: três vizinhos na Lista, cinco no Clássico e até quatorze no Spine, priorizando a direção de navegação e preservando superfícies visíveis.
- USB passa a procurar em PS3COVERS na raiz de cada dispositivo, sem subpastas. Qualquer subconjunto de artes pode ser atualizado; tipos ausentes ou corrompidos preservam a arte anterior.
- Importação de artes para ISO pela ID reconhecida ou pelo nome exato sem extensão, inclusive sem ID conhecida. Invalidação somente dos tipos copiados.
- Fundo animado próprio inspirado no XMB, com opção Ligado/Desligado no menu START e preferência persistente separada. Desligado mantém a imagem estática.
- Mantidos caches independentes 15/15/15, geometria da caixa e disco, UVs, shaders, sequência de apresentação, montagem webMAN e otimizações da 1.3.2.
- Primeira tela v1.3.3 / SpeedVash; log FIX36 com PERF 1.3.3 e contadores de reutilização/ondas.
- Mesma identidade PGORBT301; APP_VER 01.06, acima de 01.05 da 1.3.2.
- Testes de arquivos, buffers, caches com 57 jogos, menu, persistência, pré-carga, loop nativo e fundo; compilação PowerPC64 e pacote verificados no computador. Teste físico no PS3 pendente.

## 1.3.2-rc.1 — TESTE / FIX35

- Atualização parcial da interface e upload das regiões alteradas, com textura persistente.
- PNG/JPEG carregados durante a execução; caminho ARGB direto para imagens sem rotação/redução.
- Log agrupado com buffer limitado e gravação em repouso/encerramento; tempos em ms de leitura, decode, upload e interface.
- Mantidos os caches independentes de 15/15/15 e seus limites em bytes.
- Versão e criador SpeedVash na primeira tela; SELECT só mostra controles.
- Arte do disco estendida até o furo central, usando as mesmas coordenadas radiais; 1.056 triângulos preservados.
- Menu START: renomear no teclado do PS3, recarregar artes do jogo, importar trio da raiz do pendrive e reescanear biblioteca.
- Nomes persistidos separadamente, sem alterar jogos ou IDs. Importação valida todo o trio e usa arquivos temporários/backup para substituição.
- Mesma identidade PGORBT301, APP_VER 01.05; instalação sobre a 1.3.1 (01.04).
- Compilação e verificações no computador; teste físico do PS3 pendente.

# Histórico

## 1.3.1-rc.1 — FIX34

- Três caches LRU independentes e estritos: 15 capas, 15 inside e 15 artes de disco; até 64 MiB por tipo. Cache codificado de capas com 15 entradas/16 MiB.
- Inside e disco ficam em memória após fechar ou trocar de jogo; reabertura reutiliza as texturas residentes. Arquivos ausentes/inválidos usam fallback sem ocupar uma textura.
- Prefetch de até sete vizinhos de cada lado, após estabilização da transição, sem recarregamento contínuo de um conjunto maior que o cache.
- Anel central do disco opaco e prateado; furo central e geometria preservados.
- Ajuda SELECT e README explicam capas para ISO: nome exato sem extensão, sufixos `_INSIDE`/`_DISC`, ID contígua no nome e ausência de leitura da ID interna. START com caixa fechada limpa todos os caches.
- Mesma identidade PGORBT301, layout e preferências. X monta pelo webMAN com a caixa aberta ou fechada.
- Compilação e testes no computador; teste físico da 1.3.1 pendente.

## 1.3.0-rc.1 — FIX33

- Cache de 10 capas recentes, protegendo as superfícies visíveis e em transição; arquivos codificados também limitados a 10 entradas.
- Prefetch reduzido a quatro vizinhos de cada lado, evitando descarte e recarregamento contínuos com o cache menor.
- Inside Cover Full e rótulo do disco por jogo, usando `_INSIDE` e `_DISC` em `/dev_hdd0/PS3COVERS`.
- Interior dividido em esquerda/centro/direita com orientação própria na tampa, centro contínuo e fallback neutro.
- Duas artes opcionais carregadas apenas ao abrir a caixa, mantidas para o jogo selecionado e liberadas ao trocar o título.
- X monta pelo webMAN com a caixa aberta; L3/Círculo fecham; Círculo cancela e sai durante a montagem.
- Ajuda, botões e identificação atualizados para 1.3; mesma identidade PGORBT301.
- Base 1.2 testada e aprovada pelo usuário. Teste físico da nova 1.3 pendente.

## 1.2.0-rc.1 — FIX32

- Layout Lista com seleção à esquerda e caixa 3D à direita; ciclo Clássico/Spine/Lista.
- Cache LRU de texturas prontas, até 64 MiB/32 capas, protegendo superfícies visíveis e em transição.
- Cache codificado de 16 MiB, prefetch em repouso e menor volume de logs.
- L3 aproxima e abre a caixa; disco 3D aprovado sai e retorna. L3/X/Círculo fecham sem montar/sair durante a inspeção.
- Modelo fechado e UVs aprovados preservados; tampas articuladas e disco com texturas independentes.
- Buffer do HUD reutilizado e fonte da Lista corrigida para o tamanho disponível no atlas.
- Testes com 57 jogos, validação de grupos nativos completos e previews exportados do código. Teste físico pendente.


## 1.1.0-rc.1 — 2026-10-03

Build de teste **FIX31**, com SFO `01.01` e o mesmo TITLE_ID `PGORBT301`.

- Caixas do Clássico reduzidas a `0,55`, o mínimo anterior; R3 também retorna a esse tamanho.
- Segunda caixa reposicionada para caber inteira na tela.
- Saída e entrada animadas, mantendo a textura anterior até o fim do desaparecimento; inversão da navegação preserva as poses já visíveis.
- Novo layout Spine, adaptado do Aurora/Phoenix, com até cinco lombadas de cada lado e a capa selecionada à frente.
- Quadrado alterna os layouts; a escolha é persistida em arquivo separado do estado compatível com a 1.0.
- Barra de controles e ajuda atualizadas.
- Limites de cache/texturas e de quadros ajustados para a fila de lombadas. Geometria aprovada, UVs, shaders, segmentos FIFO, branding e integração webMAN preservados.
- Compilação nativa, testes de fluxo/persistência/renderização e prévias concluídos no computador. **Teste físico da 1.1 pendente.**

## 1.0.0 — 2026-10-03

Primeira release pública do **PS3 Game Orbit**, baseada no FIX30 aprovado pelo usuário.

- Biblioteca de jogos do HDD e USB, com filtros e favoritos persistentes.
- Caixas Blu-ray 3D com plástico transparente sem pigmento azul.
- Capa completa com frente, lombada central e verso; giro, inclinação e zoom.
- Jogo selecionado e caixa vizinha com capas independentes.
- Montagem assíncrona pelo webMAN MOD e confirmação do TITLE_ID do disco quando conhecido.
- Novo ícone do XMB e tela de início com a identidade PS3 Game Orbit.
- Fundo com ondas inspirado no XMB, Noto Sans Light suavizada e barra de botões compacta.
- Ajuda em SELECT e orientação normal de capa como padrão.
- Inicialização e calibração sem exibir a antiga tela rosa.
- Compilação nativa e verificações de geometria, controles, persistência, montagem e fluxo de comandos concluídas no computador.

O PKG é byte a byte o mesmo instalador FIX30 aprovado, com versão SFO `01.00` e TITLE_ID `PGORBT301`. Os identificadores FIX presentes nos fontes registram a evolução interna do projeto.
