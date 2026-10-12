# Validação da v1.5.7

## Comportamento

O scanner acrescenta a mídia PS3 ao final do catálogo HD/USB. A ordem visível continua usando A-Z, Z-A ou Custom. Um monitor separado consulta a mídia e lê os metadados; a interface recebe somente o último resultado completo, usando uma tentativa de trava que não espera por leitura óptica.

As consultas Cobra/HEN são somente de leitura: tipo real/efetivo, emulação e redirecionamento de caminhos. O programa não monta, desmonta, autentica ou ejeta o disco nessa rotina. Quando a consulta não estiver disponível, a origem física não é confirmada e a entrada genérica representa o PS3 acessível em `/dev_bdvd`.

Nome e ID vêm do PARAM.SFO; a ID válida é normalizada para letras maiúsculas. Uma troca durante a leitura não publica dados misturados. Cada disco tem uma chave `/dev_bdvd/ID` para nomes, favoritos, estilo, ordenação e seleção, enquanto o caminho de consulta permanece `/dev_bdvd`.

Inserção e troca conservam os índices do catálogo existente. Retirada do último item também conserva esses índices; um catálogo inesperado com o disco no meio usa a invalidação completa para não associar texturas aos jogos errados. As mudanças são aplicadas entre quadros reconhecidos, fora de menus, inspeção, gerenciador, teclado e montagem. Resultados de artes anteriores são cancelados por geração.

## Testes no computador

- `tests/run_physical_disc_fix47.sh`: leitura de título/ID, artes pela ID, ícone frontal e caixa neutra, PSF inválido, ID inválida, conteúdo não PS3, emulação, retirada durante a leitura, índices estáveis, seleção e câmera, filtros, persistência separada entre discos, retorno direto ao XMB e monitor sem bloqueio da interface.
- `scripts/check-library-loop-fix47.py`: executa o bloco real do main em 35 cenários, com preparação inicial, falhas, cancelamento, navegação, menus, montagem, gerenciador e eventos de inserção/troca/retirada do disco.
- `tests/run_fix47_tests.sh`: cache, concorrência, equivalência de pixels e upload por partes, incluindo uma entrada de disco no catálogo. Verifica 100 mudanças de layout sem reler ou decodificar as artes comuns.

## Compilação

`scripts/build-fix47.sh` usa a pilha fixada ps3aqua B, GCC 7.5.0, PSL1GHT `af9d3d964c8faa69abce4961a269f9582e05a33f` e ps3toolchain `961fddac01337f18da08f4471d558a7a5b0d9af2`. Registra os inputs e verifica ELF PowerPC64, SELF, SFO e PKG finalizado. A versão pública é 1.5.7 e APP_VER/VERSION 01.17, PGORBT301.

A geometria aprovada e os shaders congelados são verificados. O modelo privado e o SDK não fazem parte dos fontes públicos. O pacote instalável contém o programa compilado.

## Limites

Os testes não reproduzem o leitor de um PS3. A detecção no HEN/CFW, o comportamento ao inserir/trocar discos e o retorno ao XMB devem ser confirmados no console conforme `DISCO_PS3_1.5.7.md`. PS1 continua em hiato.

Interfaces consultadas: [webMAN-MOD — storage](https://github.com/aldostools/webMAN-MOD/blob/master/cobra/storage_ext.h), [opcodes Cobra](https://github.com/aldostools/webMAN-MOD/blob/master/cobra/syscall8.h) e [PS3HEN — GET_MAP_PATH](https://github.com/PS3Xploit/PS3HEN/blob/master/payload/main.c).
