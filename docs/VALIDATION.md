# Validação — v1.3.3-rc.1 / FIX36

Compilação e testes no computador. **Teste físico desta versão no PS3 pendente.**

## Funcionalidades novas

- Importação real em diretórios temporários: somente USB/PS3COVERS, sem raiz/subpastas; imagem isolada ou conjunto, segundo USB, extensões maiúsculas, arquivo corrompido preservando o tipo anterior e conflito de backup com restauração.
- ISO pelo nome exato sem extensão, espaços e .ISO maiúsculo; com/sem ID, prioridade da ID e resolução das três artes após cópia.
- Pré-carga por layout com contagens 3/5/14, direção, wrap, candidatos únicos e catálogos vazios/curtos. Capas de Spine visíveis protegidas e convergência sem descarte/releitura contínuos.
- Interior antes do disco, no máximo uma tentativa de leitura/decode/upload por chamada de pré-carga; abertura de artes prontas sem nova decodificação; falhas suprimidas até recarga e invalidação seletiva por tipo.
- Três caches limitados a quinze entradas e ao orçamento de cada tipo em uma biblioteca de 57 jogos, com LRU atualizado em revisitas.
- Reutilização real de ponteiros de texturas e buffers, limite de 8 MiB/três blocos livres, tamanho real da alocação contado nos caches e liberação ao encerrar.
- Menu START com cinco linhas, mapeamento da releitura da biblioteca, on/off persistido e opções com catálogo vazio; atualização parcial da interface preservando o cabeçalho.
- Malha animada de 582 vértices/576 triângulos, índices válidos, coordenadas finitas e atualização sem realocar vetores. Modo desligado pausa o movimento e preserva a textura estática anterior.
- Funções atuais de desenho nativo do fundo com APIs simuladas: matrizes identidade, estados de profundidade/culling, texturas estática/animada corretas, três faixas com alpha e ordem corretos, reativação do fragment program, limite de comandos e geometria ausente.
- Bloco atual do main FIX36 em onze cenários: montagem, falha, confirmação, cancelamento, nomes, recarga, importação parcial, rescan e configuração do fundo. Preparações de arte, atualização de ondas, invalidações e gravações ocorrem apenas entre quadros confirmados.

AddressSanitizer e UndefinedBehaviorSanitizer também passaram na suíte pública FIX36, incluindo os novos buffers, pool, importação, caches e fundo. LeakSanitizer não foi executado (detect_leaks=0).

## Base preservada

Os testes herdados foram executados para geometria JFX, UVs da capa/interior, disco de 1.056 triângulos com furo físico de 15 mm, plástico neutro, layouts, nomes/teclado, interface parcial, decodificadores, log em lotes, montagem webMAN, apresentação e controlador FIFO. Simulações da base continuam identificadas como FIX35 nos relatórios; as novas funções têm relatórios FIX36 próprios.

Os hashes dos shaders arquivados, da capa FIX26 e do controlador de comandos permanecem verificados. O modelo editável JFX continua excluído dos fontes públicos.

## Compilação e pacote

GCC PPU 7.5.0 e SDK ps3aqua fixado. ELF PowerPC64 big endian, SELF/EBOOT e cabeçalhos do PKG verificados; PKG finalizado com identidade PGORBT301, APP_VER 01.06. Ativos do instalador e arquivos de shader correspondem aos fontes. Nenhuma chamada proibida de espera RSX foi introduzida.

Relatórios atuais ficam em validacao/ no ZIP completo. O SHA-256 do instalador fica em release/SHA256SUMS.txt e docs/RELEASE_1.3.3_TESTE.json. docs/SOURCE_SHA256.txt lista os arquivos públicos desta entrega, sem a própria lista.

## Limites da validação

Simulações de APIs não reproduzem GPU, driver, coerência de caches, contagem exata de palavras de comandos ou saída de TV. Testes de arquivos no computador não medem o HD do PS3. A pré-carga continua síncrona e não cria uma thread de carregamento. Não foi medida uma melhora percentual de desempenho no console. Veja [TESTE_1.3.3.md](TESTE_1.3.3.md).
