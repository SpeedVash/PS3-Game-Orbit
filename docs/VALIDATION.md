# Validação — v1.3.2-rc.1 / FIX35

Compilação, testes e previews feitos no computador. **Teste físico da v1.3.2 no PS3 pendente.**

## Interface, decodificação e log

- Atualização parcial do rodapé e das linhas da Lista comparada byte a byte à rasterização completa. Cabeçalho sem alteração preservado; mesma alocação da textura HUD mantida. Cópia RGBA→ARGB conferida.
- Menu START, limpeza ao fechar, seleção de opções e ajuda sem painel Imagens e cache. Tela de início com versão e SpeedVash conferida visualmente.
- Código nativo real de PNG/JPEG com respostas simuladas do SDK: módulos carregados uma vez, ARGB com pitch/alpha corretos, conversão RGBA de fallback, erro de decode, tentativa após falha de load e unload único.
- Log real: nenhuma gravação durante atividade, intervalo mínimo do lote em repouso, teto de 64 KiB e gravação final ao encerrar. Contadores das quatro etapas ativos; unidades do relógio nativo convertidas para ms.

## Menu e arquivos

- START abre opções em vez de reescanear imediatamente; o menu bloqueia montagem/navegação/favoritos/layout e bloqueia ações enquanto ocupado. Círculo fecha sem sair ao XMB.
- Recarga de um jogo invalida apenas suas três artes; ponteiros das outras entradas permanecem os mesmos.
- Conversão UTF-8/UTF-16 e persistência de nomes por caminho, com ID e arquivo de jogo preservados; validação de limites/texto.
- Código nativo real do OSK com SDK simulado: buffers vivos até UNLOADED; confirmação, cancelamento, texto inválido, falhas de container/callback/load, repetição de unload e aborto. Slot 1 independente do lifecycle em slot 0.
- Importação real de trio completo, busca em mais de uma raiz, rejeição de imagem corrompida sem substituir imagens existentes, remoção de formatos concorrentes, resolução das novas imagens e rejeição de ID com caminho.
- Corpo real do main em dez cenários: seis de montagem/falha/confirmação/cancelamento e quatro de renomear/recarregar/importar/reescanear. Operações de arquivos, recursos, teclado e log só entre quadros confirmados.

## Caches, geometria e renderização

- Caches de 15/15/15 sobre biblioteca de 57 jogos, com limites independentes em bytes, reaproveitamento real de ponteiros e descarte LRU. Cache codificado limitado a 15 entradas/16 MiB.
- Onze capas de Spine e quatorze em transição protegidas no teto de quinze; prefetch converge sem repetidas decodificações. Ausentes/corrompidas usam fallback sem ocupar textura.
- Cinco partes fechadas da caixa aprovadas preservadas byte a byte. Juntas conservam área/UVs/pivô. Disco conserva 1.056 triângulos e furo de 15 mm; centro frontal e traseiro recebem arte com as mesmas UVs radiais do restante do disco.
- Renderer real com APIs simuladas em sete ângulos e cinco fases de abertura: texturas por jogo, normais/matrizes, culling, transparência do plástico, visibilidade, ordenação, fallback, fundo e HUD. Regressões deliberadas são rejeitadas.
- Shaders arquivados e controlador FIFO preservados. PKG PowerPC64 big endian, GCC 7.5.0/SDK fixado, identidade PGORBT301 e APP_VER 01.05.
- AddressSanitizer e UndefinedBehaviorSanitizer nos testes de fluxo/cache/menu/arquivos. LeakSanitizer desabilitado pelo limite de ptrace do ambiente.
- Previews exportam geometria, matrizes, ordem, shader, UVs e HUD do código nativo. Três layouts, caixa aberta/centro do disco, menu e ajuda conferidos visualmente.

Relatórios em `validacao/` no ZIP. APIs simuladas e previews não medem tempo de leitura no PS3, RSX real ou saída de TV. O caminho inicial continua síncrono. Veja [TESTE_1.3.2.md](TESTE_1.3.2.md).
