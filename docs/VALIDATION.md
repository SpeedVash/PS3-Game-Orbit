# Validação — v1.5.2 / FIX42

Compilação e testes no computador aprovados. **Teste físico desta versão no PS3 pendente.**

## Gerenciador

Os testes usam arquivos reais em diretórios temporários representando HD e USB: origem fixa ao navegar no destino, confirmação sem gravações, cópia nas duas direções, arquivos vazios, seleção por painel/pasta, dispositivos e caminho completo.

A cópia recursiva foi verificada com arquivos, subpastas, pastas vazias e combinação com uma pasta existente. Decisões de sobrescrever/pular por arquivo ou por lote preservam os arquivos não relacionados e são reinicializadas a cada lote.

Os testes de falha cobrem destino dentro da origem, conflito arquivo/pasta, item não suportado, limite de preparação, cancelamento, pausa e confirmação de saída, falha na troca final, mudanças na origem/destino, tamanhos de 64 bits e limite FAT32. O arquivo anterior e as cópias concluídas são conferidos após falhas e cancelamentos.

O ramo nativo foi executado com funções sysFs simuladas, incluindo mkdir, baselines independentes do caminho/descritor, espaço livre no dispositivo de destino, limpeza de descritores e erro 0x80010009.

## Integração

O corpo real do loop passou em 22 cenários. Durante o gerenciador não ocorre desenho de caixas, decodificação de capas ou pré-carga; o fundo fica pausado e volta à preferência anterior ao sair.

A suíte completa verifica menus, ordenação, configurações, estilos de caixa, artes via USB, caches por layout, interface real, geometria e restauração do HUD. A suíte pública passa sem o modelo licenciado. Os testes de desenho simulado cobrem três estilos, fases da abertura, texturas e transparência. Hashes da geometria aprovada, shaders e controlador FIFO permanecem preservados.

AddressSanitizer e UndefinedBehaviorSanitizer passaram no gerenciador e na suíte completa, com detecção de vazamentos desativada.

## Compilação e limites

ELF PowerPC64 big endian, SELF/EBOOT e cabeçalhos do PKG foram verificados. Identidade PGORBT301; versão pública 1.5.2; APP_VER/VERSION 01.12. Os ativos do instalador correspondem aos fontes. Nenhuma chamada proibida de espera RSX foi introduzida.

Relatórios ficam em validacao/ no ZIP completo. BUILD_INPUT_SHA256.txt registra o inventário da compilação; COMPILED_CODE_SHA256.txt confere código e ativos usados; PUBLIC_SOURCE_SHA256.txt lista os fontes distribuídos. O hash do instalador fica em instalar/SHA256SUMS.txt e docs/RELEASE_1.5.2.json.

Simulações não reproduzem driver RSX, TV, acesso ao HD/USB ou desempenho do console. A primeira carga de imagens necessárias ao desenho ainda é síncrona. Siga [TESTE_1.5.2.md](TESTE_1.5.2.md) para a verificação física.
