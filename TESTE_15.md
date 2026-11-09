# PS3 Game Orbit 1.6 — Teste 15

Esta etapa adiciona catálogo e montagem própria de **ISOs PS3 em USB NTFS e exFAT**, incluindo subpastas de `PS3ISO`. Preserva a preparação automática de permissões do Teste 14 e a interface aprovada. O controle da ventoinha continua desabilitado.

## Estado da validação

O usuário confirmou o Teste 14 com Odin Sphere Leifthrasir em pasta no HD interno e no USB; confirmou que as três montagens registradas no log também iniciaram o jogo. A base anterior também teve DiRT 3 em pasta interna no CFW e uma ISO USB no HEN confirmados. Esses resultados não validam o novo leitor USB do Teste 15: **a validação física deste PKG ainda está pendente**.

## Instalar e testar

1. Reinicie o PS3 para liberar o serviço residente da versão anterior. No HEN, ative o HEN antes de abrir o Orbit.
2. Instale `PS3_GAME_ORBIT_TESTE15.pkg` sobre o aplicativo anterior, com a mesma ID `PGORBT301`.
3. Coloque uma ISO de jogo PS3 já conhecida por funcionar na pasta `PS3ISO` do USB NTFS ou exFAT. Partes devem ser consecutivas: `Jogo.iso.0`, `Jogo.iso.1` etc., sem lacunas. Não é necessário gerar arquivos com outro gerenciador.
4. Conecte o USB antes de abrir o Orbit. Aguarde a biblioteca e pressione X no jogo. O aplicativo confere a ID do disco montado antes de voltar ao XMB; inicie pelo ícone do disco.
5. Confirme também uma segunda montagem e a troca para um jogo em pasta no HD interno. Faça o teste em CFW e depois em HEN.
6. Se falhar, informe firmware, sistema do USB, nome do jogo, ISO inteira/dividida e mensagem. Envie `/dev_hdd0/tmp/PS3_GAME_ORBIT_TESTE15.log` (fallback no `USRDIR` do Orbit). O diagnóstico manual do serviço deve mostrar `D15-OK: serviço pronto para montagem.` Ele comprova comunicação; o lançamento da ISO é a verificação seguinte.

Não retire o USB enquanto o jogo estiver montado ou em execução. Reiniciar é também a forma de encerrar o serviço residente desta etapa.

## Alcance

| Local | Disponível nesta etapa |
| --- | --- |
| HD interno | Pastas `GAMES`/`GAMEZ`, ISO e ISO dividida em `PS3ISO` |
| USB FAT32 | Montagem nativa já existente de pastas e ISOs divididas |
| USB NTFS | Leitura e montagem de ISO inteira ou dividida em `PS3ISO` |
| USB exFAT | Leitura e montagem de ISO inteira ou dividida em `PS3ISO` |

NTFS e exFAT são acessados somente para leitura de ISOs. A integração não adiciona cópia ou gravação nesses volumes pelo gerenciador de arquivos, nem montagem de jogos em pasta nesses sistemas. Pastas com arquivos `.666xx` continuam fora desta etapa. NTFS comprimido, cifrado, residente ou esparso não é aceito para a ISO. exFAT com dados não inicializados, mais de uma FAT, checksum inválido ou geometria incompatível é recusado.

Limites: 8 endereços USB sondados, 128 partições por dispositivo, setores de 512 a 4096 bytes, até 64 partes e 4096 extensões físicas por ISO. O catálogo busca até quatro níveis abaixo de `PS3ISO`, com limites de 512 diretórios e 4096 candidatos por volume. O caminho completo tem limite de 1024 bytes. exFAT usa FatFs com nomes UTF-8 e buffer de nome de 255 bytes; clusters devem caber no limite de 32768 setores e 32 MiB. Estes são limites da implementação desta etapa.

## Serviço e leitores

- `raw_usb_catalog15` detecta NTFS/exFAT em volume sem tabela de partições, MBR/EBR ou GPT. Reconfere serial do volume, partição, conjunto de partes e TITLE_ID ao preparar a montagem. Não utiliza arquivos de índice externos.
- O leitor NTFS existente é reutilizado. O leitor exFAT usa **FatFs R0.16**, configurado somente para leitura, LBA64, UTF-8 e fast seek. A procedência, configuração, hashes e duas pequenas proteções estão em `vendor/fatfs/ORIGIN.json` e `ORBIT_READONLY_GUARDS.patch`; a licença original permanece nos arquivos.
- A geometria e o checksum da região de boot exFAT são validados antes da montagem do filesystem. Arquivos com `ValidDataLength` diferente do tamanho são recusados. O percurso da FAT ao gerar o mapa é limitado pelo tamanho do arquivo, impedindo ciclos infinitos em metadados danificados.
- O novo `orbit_service15.sprx` mantém a entrada e a ponte de IPC do Teste 13. O canal e os tokens usam `SERVICE15`/`ORBIT15`, isolados de versões anteriores. O serviço carrega somente após dois quadros da biblioteca efetivamente apresentados e reconhecidos.
- Apenas uma montagem raw inicia a tarefa de leitura USB, com ponte assembly própria e TOC explícito, prioridade 1000 e pilha de 64 KiB. O worker de IPC conserva prioridade 0 e pilha de 64 KiB. Ambos permanecem residentes ao voltar ao XMB.
- As interfaces 600/601/602/609 abrem, leem e fecham dispositivos USB. A montagem usa o proxy Cobra/HEN `0x6808` e responde aos eventos com até 256 KiB. Não há escrita de setores, chamadas de fan ou sondagem de temperatura no novo módulo.
- O tempo de espera da fila usa **ETIMEDOUT `0x8001000B`**, confirmado no SDK fixado; `0x80010005` é ESRCH. Falhas de leitura respondem EIO `0x8001002B`; comandos inválidos respondem EINVAL `0x80010002`. O fim da ISO recebe preenchimento com zeros.
- A mídia candidata é conferida em buffer separado do utilizado pela ISO ainda montada. Ao trocar uma montagem raw, o serviço desmonta o proxy enquanto a tarefa ainda atende leituras; depois pede parada, aguarda a tarefa, destrói os eventos e fecha o handle. Uma falha de desmontagem preserva a tarefa existente.
- A tarefa de montagem do aplicativo continua preparando permissões apenas na pasta interna selecionada, antes do bootstrap/RPC. ISO, FAT32 externo, NTFS e exFAT não recebem chmod. A conferência do TITLE_ID realmente montado e a rolagem de mensagens longas permanecem.

## Verificações locais executadas

- `tests/run_raw_usb15.sh`: volume NTFS real; exFAT formatado com FatFs independente da configuração de produção; setores 512/4096; nomes UTF-8; ISO contínua, fragmentada, dividida e rejeição de lacunas; volume de 6 GiB com ISO de 5 GiB e leitura exata acima de 4 GiB; MBR/GPT; checksum, dados não inicializados e ciclo de FAT; serial/ID/caminho/mídia indisponível.
- `tests/run_service15.sh`: C real do serviço e C++ real do cliente em threads, com a fronteira LV2 simulada; bytes de leitura, 350 ms de ociosidade antes de ler novamente, códigos de erro independentes, troca de montagem e limpeza em falhas de filas, portas, threads e proxy; cancelamento, IPC, tokens e diagnósticos antigos.
- `tests/execute_native_service15.py`: código PPC64 real executado por Unicorn 2.1.4, com LV2/Cobra simulados; r2 inicial zerado/incorreto/correto; OPD32/TOC; três retornos de timeout anteriores à primeira leitura; eventos devolvidos em r4/r5/r6/r7 conforme o SDK; bytes, EOF, EBUSY, EINVAL e EIO.
- `tests/check_raw_lv2_errors15.py <SDK>/ppu/librt/lv2errno.c`: constantes comparadas à fonte do SDK, sem depender dos valores do mock.
- `tests/run_permissions15.sh`: modos de arquivos reais conferidos antes do bootstrap, preservação de conteúdo, repetição, falha, cancelamento e ID incorreta, usando as flags do Teste 15.
- `scripts/check-library-loop-service15.py`: 37 cenários do bloco real de `main`; pré-carga, quadros apresentados, inicialização e checagem manual do serviço, cancelamento, disco físico e fluxos existentes.
- `tests/run_service_core_fix48.sh` e `tests/run_status_scroll12.sh`: limites do leitor raw, LBAs acima de 2 TiB, NTFS/MBR/GPT malformados e pixels reais da rolagem de mensagens. Os testes puros de política de fan não habilitam a ventoinha no aplicativo.
- GCC 7.5.0 com `-Werror`; verificações de PRX/relocações/descritores; decodificação SCETool comparada byte a byte ao PRX e verificação independente OpenSSL/HMAC dos três segmentos; PKG final extraído e comparado ao staging e ao Teste 14.

Essas verificações não executam o kernel, os drivers ou o jogo em um PS3. O teste físico em cada console continua necessário.

## Recompilar

Use o checkpoint de toolchain GCC 7.5.0 e PSL1GHT `af9d3d964c8faa69abce4961a269f9582e05a33f`. Prepare o modelo JFX original com `python3 scripts/prepare-jfx.py --source /caminho/JFX_DVDBlu.zip`. O arquivo editável e seu include gerado não são redistribuídos.

O script `scripts/build-test15.sh` recompila o serviço e o aplicativo. Defina `PS3DEV`, `PSL1GHT`, `ORBIT_SCETOOL` e `ORBIT_SCETOOL_DATA` como no fluxo do Teste 13. SCETool 0.2.9 foi compilado da revisão `d78d8986c9ab831f084516f21ecdf87080ac7b78`; chaves e curvas permanecem dependências externas. FatFs já está incluído no diretório vendor. `PS3_GAME_ORBIT_TESTE15.pkg` é o pacote finalizado `.gnpdrm.pkg` renomeado para entrega.

Para a verificação PPC64, instale `unicorn==2.1.4` em ambiente separado e execute `python3 tests/execute_native_service15.py build/service15/service.linked.elf build/service15/service.prx`. O ZIP inclui os dois arquivos correspondentes ao módulo entregue em `validacao/teste15/native/`.

Fontes técnicas: [especificação exFAT da Microsoft](https://learn.microsoft.com/en-us/windows/win32/fileio/exfat-specification), [FatFs R0.16](https://elm-chan.org/fsw/ff/arc/ff16.zip), [fast seek do FatFs](https://elm-chan.org/fsw/ff/doc/lseek.html), código do SDK fixado e ABI de armazenamento/proxy Cobra/HEN. Nenhum plugin binário de outro gerenciador é usado.
