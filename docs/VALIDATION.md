# Validação — 1.1.0-rc.1 / FIX31

A atualização foi compilada para PowerPC64 big endian com a pilha ps3aqua B, GCC 7.5.0, e empacotada como PKG NPDRM finalizado. TITLE_ID: `PGORBT301`; APP_VER: `01.01`.

## Verificações concluídas no computador

- Geometria aprovada: os mesmos 5.597 triângulos, normais, UVs e cinco partes do OBJ JFX original. Shaders VPO/FPO arquivados e FIFO da 1.0 preservados.
- Clássico em 0,55; R3 no mínimo; Spine com cinco lombadas por lado, capa frontal selecionada e deduplicação de bibliotecas pequenas/vazias.
- Animação da caixa anterior e da próxima; reversão durante o movimento sem saltos de posição/opacidade dos jogos já visíveis.
- 1.200 solicitações rápidas de navegação sem exceder o limite de quatorze caixas, com estabilização posterior.
- Sincronização real das texturas no caminho host: a textura da caixa anterior permanece até o fim do fade e é liberada depois; onze capas do Spine carregam.
- Persistência atômica do layout, rejeição de dados inválidos e independência do arquivo de favoritos da 1.0.
- Cache de imagens codificadas limitado a 32 MiB e texturas reduzidas proporcionalmente a 1024 pixels por lado. Capas acima de 16 milhões de pixels são rejeitadas antes da decodificação.
- Corpo real da função nativa de desenho exercitado com APIs simuladas em sete ângulos e 1/2/3/11/14 caixas: textura e matriz por jogo, UVs de verso, capas frontais/ausentes, fade de papel/logo, plástico transparente neutro em duas passagens, ordem por opacidade/profundidade, fundo e HUD.
- Regressões deliberadas de plástico, culling, textura, opacidade do papel, visibilidade, restauração da profundidade e recarga do fragment program foram rejeitadas.
- O fluxo real de main mantém I/O, gravação de estado, montagem, releitura e alterações de texturas entre quadros confirmados. Falha de rede, montagem confirmada e saída pelo Círculo passaram.
- Inicialização com 61 flips limitados e calibração fora da tela preservadas. Simulação do controlador FIFO completou 10.000 segmentos alternados; GET/REF/backend label ausentes bloqueiam a reutilização.
- Regressões herdadas de scanner, favoritos, orientação, UVs, montagem HTTP, interface UTF-8 e empacotamento passaram.
- Prévias dos dois layouts e das transições foram renderizadas com geometria, matrizes e posições exportadas do código nativo.

Os segmentos FIFO continuam com 64 KiB. Esta build aceita até quatorze caixas por quadro, com orçamento máximo de 57.344 bytes e guarda inicial de 61.440 bytes. Os testes de API verificam estados, ordem e guardas; não reproduzem o custo exato das chamadas RSX do driver.

## O que falta

**Teste físico da 1.1 no PS3.** A aprovação anterior em Slim com CFW foi para a 1.0/FIX30. Não há aprovação de hardware, desempenho ou HEN para esta build.

As prévias de `docs/images` usam capas demonstrativas e simulação gráfica no computador. Consulte [TESTE_1.1.md](TESTE_1.1.md) para verificar a atualização no console.
