# Histórico de mudanças

## 1.6 — 2026-10-13 (FIX48)

- Substitui a montagem HTTP por um serviço próprio residente, incluído no PKG, que usa HEN/Cobra diretamente.
- Monta ISOs PS3 no HD interno e USB FAT32, incluindo `.iso.0`, `.iso.1`...; verifica sequência, tamanho e ID antes de substituir a montagem.
- Monta jogos em pasta em GAMES/GAMEZ com redirecionamentos do sistema e um disco virtual mínimo criado pelo Orbit.
- Acrescenta catálogo e leitura de ISOs em USB NTFS, com MBR, EBR e GPT; interpreta MFT, índices, atributos e mapas de setores em 64 bits, sem gravar no volume.
- Mantém o leitor NTFS no VSH para servir a ISO após fechar a interface. Valida tamanho, setor e número de série do USB ao montar.
- Adiciona START → Controle da ventoinha, modos Automático/Manual/SYSCON, temperaturas CPU/RSX, ajustes salvos e proteção térmica.
- Verifica a política aplicada, restaura instruções temporárias e suspende o controle quando há outro controlador ou durante a transição de processo no HEN.
- Carrega o plugin em slot livre sem editar boot_plugins.txt nem descarregar plugins de terceiros.
- Mantém o disco original, cache comum de artes, imagem de carregamento, gerenciador e modelo/shaders aprovados. PS1 permanece em hiato.
- Versão 1.6, APP_VER/VERSION 01.60 e identidade PGORBT301. Compilação e testes no computador; teste físico pendente.

## 1.5.7 — 2026-10-12 (FIX47)

- Adiciona o jogo PS3 disponível no leitor à biblioteca 3D, com nome e ID lidos do PARAM.SFO.
- Usa as artes da ID em PS3COVERS na capa, interior e disco; conserva a caixa articulada e os três layouts.
- Detecta inserção, troca e retirada em uma tarefa separada e aplica as mudanças entre quadros, preservando índices e texturas do HD/USB.
- Separa favoritos, nomes e estilos por ID do disco, mantendo o caminho real do leitor para a consulta dos arquivos.
- X confirma a ID e volta ao XMB sem enviar montagem ao webMAN; indica “Ir ao XMB” na ajuda e nos comandos do disco.
- Consulta o tipo de mídia, emulação e redirecionamentos Cobra/HEN sem montar, desmontar ou ejetar. Consultas indisponíveis usam a entrada genérica Disco.
- Mantém o cache compartilhado e a imagem de início durante a preparação das artes. PS1 permanece em hiato.
- Versão 1.5.7, APP_VER/VERSION 01.17 e PGORBT301 preservado. Teste físico desta versão pendente.

## 1.5.6 — 2026-10-11 (FIX46)

- Mantém a imagem “Carregando sua biblioteca...” visível até concluir as artes da janela inicial de até 15 jogos.
- A pré-carga reconhece os ciclos de upload sem limpar ou trocar os framebuffers; o primeiro quadro da biblioteca substitui a imagem inicial.
- Cede CPU ao worker durante a espera, com saída por Círculo e conclusão definida pela preparação real, sem atraso fixo.
- Mantém o cache comum e a navegação em segundo plano da v1.5.5, cuja troca de layouts foi confirmada pelo usuário sem a pequena pausa anterior.
- Validação do bloco real do main em 33 cenários, incluindo espera longa, catálogo vazio, falta de artes, cancelamento e falhas.
- Versão 1.5.6 e APP_VER/VERSION 01.16, PGORBT301 e configurações mantidos. A correção visual do início aguarda teste físico. PS1 permanece em hiato.

## 1.5.5 — 2026-10-11 (FIX45)

- Prepara as artes de uma janela inicial de até 15 jogos, com progresso antes de abrir a biblioteca.
- Os três layouts compartilham até 15 capas, 15 interiores e 15 discos, mantendo as texturas comuns ao alternar.
- Leitura, decodificação, orientação, redução e conversão ARGB executadas em um worker PPU; fila limitada a duas imagens prontas e uma transferência.
- Envio à GPU dividido em até 256 KiB por quadro, somente após reconhecer o quadro anterior; textura publicada apenas ao concluir.
- Prioriza capas visíveis e próximos jogos; evita preparar vizinhos que não cabem junto às caixas em transição.
- Recarga e novo catálogo invalidam resultados antigos; artes pendentes usam aparência neutra; arquivos inválidos terminam a pré-carga sem bloquear a biblioteca.
- Worker encerrado antes de descarregar PNG/JPEG; contadores e logs protegidos para uso concorrente.
- PS1 permanece em hiato. Modelos, shaders e geometria do gerenciador aprovado conservados.
- Versão 1.5.5 e APP_VER/VERSION 01.15, PGORBT301 mantido. Teste físico desta versão pendente.

## 1.5.4 — 2026-10-11 (FIX44)

- Preserva capas, interiores e discos recentes até a pressão de capacidade exigir liberação.
- Redução entre layouts prioriza o jogo selecionado, a janela do destino e as caixas que ainda desaparecem.
- Cache de arquivos JPEG/PNG já lidos preserva as imagens comuns ao layout de destino.
- Limites Clássico 7/7/7, Spine 15/15/15 e Lista 5/5/5, com a proteção transitória de caixas visíveis mantida.
- Regressão comparativa: quatro novas preparações na política anterior versus zero na nova, no cenário de ida e volta; 5/5 arquivos comuns mantidos na redução do cache.
- Modelo de PS1 preparado a partir do anexo CC0 de effe, com tampa, livreto e disco articulados, abertura de 160° e arquivos Blender/GLB de prévia.
- Gerenciador v1.5.3 confirmado funcional pelo usuário; fluidez da v1.5.4 ainda depende do teste físico.
- Versão 1.5.4 e APP_VER/VERSION 01.14, PGORBT301 mantido.

## 1.5.3 — 2026-10-11 (FIX43)

- Corrige o gerenciador dividido entre o topo e o rodapé da tela na v1.5.2.
- A imagem contínua 1280×512 usa uma geometria própria, centralizada no quadro 1280×720, com UVs contínuos e proporções preservadas.
- Retorno à biblioteca restaura a textura e seleciona novamente a geometria original.
- Regressão com as funções nativas reais: 11 telas do gerenciador, transições repetidas, falhas de alocação/mapeamento/upload e liberação da geometria ao encerrar.
- Prévia considera os vértices e UVs enviados ao desenho, além do bitmap.
- Versão pública 1.5.3 e APP_VER/VERSION 01.13; identidade PGORBT301 preservada.
- Teste físico no PS3 pendente; visualização de PS1 continua reservada para a próxima etapa.

## 1.5.2 — 2026-10-11 (FIX42)

- L1/R1 escolhem painéis; Triângulo abre ações; R3 escolhe dispositivo; SELECT mostra caminho completo.
- Marcações vinculadas a um painel e uma pasta de origem; navegação no destino não inverte a cópia.
- Menu para copiar, marcar todos, limpar seleção e atualizar dispositivos/pastas.
- Preparação por etapas, cancelável e sem gravações; confirmação com arquivos, pastas, tamanho total, origem e destino.
- Cópia recursiva de pastas e pastas vazias; combinação de pastas existentes com confirmação dos arquivos.
- Nomes maiores, foco por borda, seleção com caixas azuis e cópias concluídas com sinal verde.
- Progresso do arquivo e do lote, resumo de copiados/ignorados/erros e confirmação de saída durante operações.
- Temporários, preservação do destino anterior, rollback e cópias concluídas mantidos.
- Limites de preparação por lote e rejeição de destino dentro da origem ou conflito arquivo/pasta.
- Versão pública 1.5.2 e SFO APP_VER/VERSION 01.12; PGORBT301 mantido.
- Estilos de caixa, caches, ordenação, preferências e pausa/restauração do fundo preservados.
- Visualização de jogos PS1 reservada para a próxima etapa, após teste desta versão no PS3.

## 1.5.1 — 2026-10-10 (FIX41)

- Gerenciador de dois painéis independentes para HD e USB, com posições preservadas ao trocar de painel e seleção de dispositivos.
- Seleção múltipla por arquivo ou pasta atual; arquivos marcados em azul e cópias concluídas em verde na origem e no destino.
- Cópia para o outro painel com progresso, cancelamento e confirmação individual ou para todo o lote: sobrescrever, ignorar ou cancelar.
- API nativa de arquivos unificada; dados do caminho e do descritor verificados separadamente, espaço livre consultado no dispositivo de destino e erros por etapa/código.
- Temporário por arquivo, rollback na falha de conclusão, manutenção de cópias já concluídas e verificação do limite FAT32 por arquivo.
- Triângulo → Tipo de caixa: transparente, Platinum cinza opaco e Greatest Hits vermelha; plástico externo colorido e interior transparente, com escolha persistida por jogo.
- Mesmo modelo fechado/aberto; separação dos materiais externos/internos sem mudar a geometria ou as artes.
- Trabalho opcional de pré-carga, logs e gravações adiado durante comandos manuais e por 650 ms após o último comando; pré-carga também aguarda as transições.
- Caches por layout preservados: Clássico 7/7/7, Spine 15/15/15 e Lista 5/5/5.
- Versão 1.5.1 no início, cabeçalho e menus; PGORBT301 mantido e APP_VER/VERSION 01.11.
- Testes de cópia, persistência, loop, renderização e memória aprovados no computador; compilação nativa verificada. Teste físico no PS3 pendente.

## 1.5

- Ordenação A-Z, Z-A e Custom, com posições persistidas por caminho do jogo.
- Triângulo → Alterar posição no modo Custom; seleção preservada na mudança de ordem.
- START → Gerenciador de arquivos do HD: navegação, copiar/colar, progresso, cancelamento e confirmação de substituição.
- Cópia por blocos com tamanho de 64 bits, temporário e restauração do destino anterior em falha de conclusão.
- Aviso de entrada e pausa temporária do fundo, com retorno à preferência anterior ao sair.
- Caixa selecionada articula o mesmo conjunto completo em todas as fases, incluindo fechada; interior e disco preparados na entrada e durante a espera.
- Interface do gerenciador e retorno à biblioteca usam atualização controlada da textura de HUD.
- Identidade PGORBT301 preservada; versão pública 1.5 e APP_VER/VERSION 01.10.
- Configurações, USB/PS3COVERS parcial/em lote, arte normalizada e caches 7/7/7, 15/15/15 e 5/5/5 mantidos.

# Histórico de versões

## 1.4.2 — 2026-10-06 (FIX39)

- Plástico neutro na lombada e relevos; retirada a faixa reta de duas camadas que acinzentava a arte.
- Lombada feita das curvas do próprio invólucro, com topo e base arredondados e pontos de união idênticos às laterais.
- Transporte das extremidades por duas articulações, com normais suavizadas e buffers reutilizados durante a animação.
- Tela inicial sem TESTE; versão 1.4.2 centralizada também no cabeçalho e no menu START.
- Mantidos o modelo aprovado fechado/aberto, abertura de 160°, disco da 1.4.1, salvamento das configurações e caches por layout.
- PGORBT301 mantido, SFO APP_VER/VERSION 01.09, fontes e pacote atualizados.
- Testes de união do plástico em seis fases, geometria, fluxo/menu, PNG/USB, configurações e submissão nativa. Verificação física no PS3 pendente.

## 1.4.1 — TESTE (FIX38)

- Corrigida a peça cinza com triângulos duplicados que sobrescrevia a arte frontal do disco.
- Caixa fechada gerada da mesma geometria aprovada que abre, preservando seis draws para a biblioteca e a calibração.
- Abertura de 160°, disco deslocado para x=96 mm e z=24,7 mm na retirada completa.
- Bordas transparentes internas/externas de 0,5 mm, com furo físico de 15 mm e diâmetro de 120 mm.
- Verso Blu-ray prateado com textura procedural; PNG frontal composto sobre substrato opaco.
- Disco e inside selecionados carregados ao abrir; caches e pré-carga em repouso mantidos.
- Gravações sucessivas de configurações, nomes, favoritos, seleção, layout e fundo com transação compatível com rename do PS3, rollback e recuperação de backup.
- USRDIR é o destino persistente; migração dos dados antigos em /dev_hdd0/tmp.
- Splash usa a versão atual, exibindo v1.4.1 e SpeedVash. SFO 01.08 mantém PGORBT301.
- Novos testes de PNG→ARGB, ausência da sobreposição, equivalência do modelo fechado, gravações repetidas e falhas simuladas na troca de arquivo.


## 1.4-rc.1 — 2026-10-06

- Configuração unificada com checksum e gravação atômica, preservando preferências antigas.
- Triângulo para nome/USB/recarregar/favoritos; START para biblioteca/fundo/USB em lote/último jogo.
- Importação parcial ID/ISO em USB/PS3COVERS com saídas JPG 1000×550 e PNG 500×500.
- Cópia da biblioteca inteira com progresso, deduplicação por ID e cancelamento entre jogos.
- Caches por layout: Clássico 7/7/7, Lista 5/5/5, Spine 15/15/15; três vizinhos visíveis por lado no Spine.
- Filtro vazio sem caixa; pré-carga equilibrada entre capa e artes internas, mantendo buffers reutilizados.
- Interior procedural, arte interna completa, duas articulações com lombada contínua, topo transparente e borda externa do disco de 0,4 mm.
- PGORBT301 mantido; SFO APP_VER 01.07. Publicação inicial como TESTE, aguardando verificação física.


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
