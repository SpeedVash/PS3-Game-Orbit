# Validação da v1.5.5 — FIX45

A alteração separa a preparação CPU das artes do desenho. O modelo PS3 aprovado, os shaders arquivados, a geometria do gerenciador e o controlador de comandos RSX são preservados.

## Cache e fila

- Janela comum de até 15 jogos: sete anteriores, selecionado, sete seguintes, seguindo filtro e direção da navegação.
- Até 15 capas, 15 interiores e 15 discos, com 64 MiB por tipo e imagens limitadas a 1024×1024. A memória de uma transferência em andamento também é reservada antes da alocação.
- As capas visíveis e em transição têm prioridade. Vizinhos que excedem o espaço disponível aguardam, evitando decodificações repetidas de imagens que não cabem juntas.
- Um worker PPU com prioridade 1500 e pilha de 1 MiB lê, decodifica, orienta, reduz e converte os pixels para ARGB. Nenhuma chamada RSX/GCM executa nesse worker.
- Fila limitada a 64 pedidos e duas imagens prontas; apenas uma imagem em transferência. As imagens prontas/transferência usam até 12 MiB, além do único trabalho de decodificação e do arquivo comprimido em processamento. Não se conserva uma cópia decodificada de toda a biblioteca na RAM.
- A rotina principal usa tentativas de acesso à fila sem esperar pelo worker. A leitura e a decodificação ocorrem fora do bloqueio da fila.
- A GPU recebe no máximo 256 KiB de arte por serviço de quadro, após o reconhecimento do quadro anterior. Uma textura só fica disponível para desenho quando todos os pixels e o preenchimento das linhas foram enviados.
- Catálogo, índice, caminho do jogo, caminho da arte, papel da textura, orientação e geração identificam o pedido. Recargas e novos catálogos descartam resultados antigos.
- Módulos PNG/JPEG são protegidos de uso concorrente com importações explícitas. O worker é unido antes de descarregar os módulos. Logs e métricas têm proteção própria.

## Verificações

`tests/run_fix45_tests.sh` usa o carregador e os caches reais, não apenas a política abstrata:

- Leitura bloqueada no worker enquanto a rotina principal muda a prioridade e consulta resultados.
- Limite de duas imagens prontas; cancelamento de um pedido em processamento; nova geração no mesmo índice; término do worker.
- Oito threads registram 80.000 medições sem perder incrementos.
- Pixels ARGB idênticos à preparação aprovada para capa completa, capa vertical, JPEG com EXIF, PNG transparente de disco e imagem grande. O disco preserva o fundo prateado opaco e seus recortes geométricos.
- Textura incompleta permanece indisponível; cada parte contém até 256 KiB.
- Preparação de 45/45 artes. Cem trocas de layout conservam os mesmos endereços GPU e não fazem novas leituras/decodificações, inclusive com os arquivos de origem temporariamente removidos.
- Duzentas alterações rápidas de seleção/layout, extremidades circulares e caixas em transição; a fila termina no jogo correto, respeitando os limites de memória.
- Pausa do gerenciador sem liberar o cache; recarga por papel; arquivo ausente; restauração após recarga; substituição de catálogo; biblioteca vazia e encerramento.

`scripts/check-library-loop-fix45.py` compila o bloco real do `main.cpp` com APIs observáveis. Seus 29 cenários verificam pré-carga antes da biblioteca, progresso sem caixas incompletas, Círculo durante a preparação, falhas de worker/interface/início/conclusão de quadro, biblioteca vazia, navegação, menus, caixa, montagem, recarga e retorno do gerenciador. O carregador é chamado entre o reconhecimento do quadro anterior e o início do seguinte. O prefetch síncrono antigo não executa em FIX45.

As regressões da v1.5.4, do gerenciador corrigido e da rotina anterior são mantidas. Os testes com AddressSanitizer/UndefinedBehaviorSanitizer verificam o novo fluxo; nesta infraestrutura, a detecção de vazamentos via ptrace precisa ser desativada. Isso não equivale a um teste de vazamentos no console.

## Compilação nativa

Use `PS3DEV` e `PSL1GHT` apontando para a mesma instalação aprovada, GCC 7.5.0, e execute `scripts/build-fix45.sh`. O script registra o ambiente e os hashes, confere o modelo JFX e os shaders congelados, compila FIX45 e verifica ELF PowerPC64 big endian, SELF, SFO e PKG finalizado.

A identidade é `PGORBT301`, com `APP_VER/VERSION 01.15`. Os símbolos nativos incluem a criação/saída do worker e os métodos de streaming; a inspeção dos objetos confirma que o worker não chama RSX/GCM. As chamadas de espera proibidas do pipeline continuam ausentes.

O ZIP completo inclui os registros de compilação e os testes. Os fontes públicos conservam o limite de redistribuição do modelo PS3: o OBJ e o include JFX privado não estão no repositório/ZIP público; seus hashes aparecem na validação do build.

## Limites da conclusão

Esses resultados verificam a implementação, a identidade do pacote e as condições testadas. Não medem FPS ou pausas no PS3 físico. O HUD, a gravação de configurações, a transferência entre CPU/GPU e o custo de desenhar as caixas continuam dependendo do hardware. Operações explícitas de copiar/importar/atualizar arquivos mantêm seu próprio custo. Veja `TESTE_1.5.5.md` para a comparação no console. PS1 permanece em hiato.
