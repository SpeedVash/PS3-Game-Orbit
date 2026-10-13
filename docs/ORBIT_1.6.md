# PS3 Game Orbit v1.6 — instalação e teste

## Instalação

Instale `PS3_GAME_ORBIT_v1.6.gnpdrm.pkg` sobre a versão anterior. A identidade PGORBT301 e as preferências continuam as mesmas. O instalador inclui a interface, o plugin próprio e o pequeno disco virtual para jogos em pasta.

Habilite HEN ou use CFW com Cobra ativo antes de abrir. O Orbit carrega seu plugin em um slot VSH livre. Não modifica boot_plugins.txt, não remove outros plugins e não exige webMAN/multiMAN. Após reiniciar o console, habilite HEN e abra o Orbit para carregar novamente o serviço.

Selecione o jogo e pressione X. A preparação e o pedido de montagem ocorrem em uma tarefa separada. Com a ID confirmada em `/dev_bdvd`, o Orbit volta ao XMB; inicie pelo ícone do disco. Sem ID conhecida, o pedido aceito pede que você confira no XMB. Círculo cancela a espera. Dados inválidos são recusados antes de ejetar a montagem atual; se a chamada do sistema falhar depois disso, o serviço limpa a montagem nova e reinserta o disco físico, quando presente. A ISO montada anteriormente por outro gerenciador não é restaurada automaticamente.

## Organização dos jogos

| Dispositivo | Pasta | Formatos |
| --- | --- | --- |
| `/dev_hdd0` | `PS3ISO` | `.iso` ou `.iso.0`, `.iso.1`... |
| `/dev_usb000` até `/dev_usb007` | `PS3ISO` | `.iso` que caiba no FAT32 ou partes sequenciais |
| HD/USB FAT32 | `GAMES` ou `GAMEZ` | Pasta com `PS3_GAME/PARAM.SFO`, arquivos inteiros |
| USB NTFS | `PS3ISO` | ISOs inteiras ou divididas; subpastas até quatro níveis |

O leitor NTFS é próprio e somente leitura. Verifica MBR/EBR/GPT, número de série do volume, tamanho dos setores e limites dos mapas. O plugin permanece carregado enquanto a ISO NTFS estiver montada, pois o jogo depende dele para ler seus setores. Mantenha o USB conectado. Se o USB for retirado ou trocado, as leituras falham, sem continuar usando setores de outro dispositivo.

As partes devem ser consecutivas, sem lacunas, e ter tamanho múltiplo de 512 bytes; o conjunto deve formar uma ISO válida. O limite do Orbit é 64 partes e 4096 extensões de setores. Firmware/payload antigos podem impor limites menores. Para FAT32, cada arquivo deve ter menos de 4 GiB. ISOs NTFS esparsas, comprimidas, cifradas ou em links especiais são recusadas. Não há suporte a pastas NTFS, exFAT nem união de arquivos `.666xx` de jogos em pasta.

As capas, interiores e discos continuam em PS3COVERS, associados à ID ou ao nome da ISO. Não é necessário colocar as artes dentro do volume NTFS; use o fluxo existente de atualização de artes por USB FAT32 para gravá-las no HD interno. O gerenciador de arquivos continua em HD/USB FAT32.

## Ventoinha

Abra START → Controle da ventoinha. Cima/baixo escolhem o ajuste; esquerda/direita ou X alteram um ponto; L1/R1 alteram cinco. Círculo volta ao menu.

| Modo | Comportamento |
| --- | --- |
| Automático | Usa a maior temperatura entre CPU e RSX, alvo de 60–75°C, mínimo 35–70%, máximo até 100% |
| Manual | Velocidade de 35–100%; proteção térmica continua ativa |
| Padrão do PS3 | Devolve a política ao SYSCON |

Padrão inicial: Automático, alvo 68°C, mínimo 35%, máximo 85%, manual 40%. A partir de 80°C, a proteção solicita pelo menos 85%; a partir de 83°C, solicita 100%. Três falhas seguidas dos sensores solicitam SYSCON. Não há desligamento automático do console nesta versão.

O adaptador reconhece os pontos de acesso de ventoinha CEX/DEX de 4.55 a 4.92 e verifica o código antes de usar permissões temporárias. A velocidade aplicada é lida de volta; código não reconhecido ou consulta indisponível desativa o adaptador, mantendo a montagem separada. No HEN, aguarda 15 segundos após mudança de processo antes de acessar a política da ventoinha.

Se webMAN, sMAN ou fan_control estiver carregado, o Orbit informa outro controlador ativo e não assume a ventoinha. Para testar o controle próprio, desative a carga desse controlador pelo seu fluxo habitual e reinicie o console. O Orbit não faz essa alteração automaticamente.

## Teste no console

1. Confira versão 1.6, preferências, gerenciador e alternância entre layouts.
2. Monte uma ISO do HD e uma ISO dividida do FAT32, retornando ao XMB e iniciando o jogo.
3. Monte uma pasta em GAMES/GAMEZ e confirme a ID correta no XMB.
4. Conecte NTFS com PS3ISO, atualize a biblioteca, monte a ISO e confirme que o jogo lê depois de fechar o Orbit.
5. Confira CPU/RSX e velocidade em START → Controle da ventoinha; teste primeiro o automático e confirme o funcionamento depois de voltar ao XMB.
6. Confira suspensão com outro controlador, ausência do USB e cancelamento durante a preparação.

Os testes no computador e a compilação nativa não substituem essa confirmação em hardware. PS1 permanece em hiato.
