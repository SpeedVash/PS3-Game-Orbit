# Gerenciador de arquivos — v1.5.2

Abra START → Gerenciador de arquivos HD/USB e confirme o aviso com X. O fundo animado é pausado durante o uso e volta à preferência anterior ao sair.

## Copiar do HD para USB

1. Use L1 para escolher o painel esquerdo. R3 abre a escolha de HD/USB.
2. Abra a pasta de origem com X. Quadrado marca arquivos ou pastas; Triângulo → Marcar todos marca o conteúdo da pasta atual.
3. Use R1 para escolher o painel direito. Escolha o USB com R3 e abra a pasta de destino com X.
4. Abra Triângulo → Copiar. O menu indica o painel de destino, mesmo quando você está nele.
5. Aguarde a contagem e confira quantidade de arquivos/pastas, tamanho total, origem e destino.
6. X inicia a cópia. Círculo volta sem gravar.

Para USB → HD, comece marcando os itens no painel do USB e escolha o HD no outro painel. Também é possível copiar entre pastas do HD e entre USBs disponíveis.

## Seleção e navegação

As marcações pertencem ao painel e à pasta de origem. Trocar de painel ou navegar no destino mantém a seleção. Abrir outra pasta na origem limpa a seleção anterior. Para marcar no outro painel, primeiro use Triângulo → Limpar seleção.

L1/R1 escolhem os painéis; L2/R2 percorrem sete entradas por vez. R3 escolhe um dispositivo para o painel ativo. Círculo volta à pasta anterior; na raiz do dispositivo abre a lista de dispositivos. START sai.

A borda indica o item em foco. Caixas azuis indicam marcações; o sinal verde e “Copiado” indicam cópias concluídas durante a sessão. SELECT abre o caminho completo do item em foco; use L2/R2 para ver suas páginas e Círculo para voltar.

## Pastas e arquivos existentes

Uma pasta marcada é copiada com suas subpastas, arquivos e pastas vazias. A pasta é criada dentro do destino com o mesmo nome. Se já existir, os conteúdos são combinados; arquivos não relacionados no destino permanecem.

Conflitos de arquivo permitem sobrescrever um, pular um, sobrescrever todos, pular todos ou cancelar. A opção inicial é pular. A decisão para todos vale somente para o lote atual. Um arquivo não substitui uma pasta, nem uma pasta substitui um arquivo.

## Progresso, cancelamento e erros

O progresso mostra o arquivo atual e o lote inteiro. O resumo separa arquivos copiados, ignorados, erros e arquivos não concluídos. O tamanho copiado contabiliza somente arquivos concluídos.

Círculo cancela a preparação ou a cópia. START durante a operação pausa o trabalho e pergunta se você quer continuar ou cancelar e sair. Cancelar preserva os arquivos concluídos, o destino anterior do arquivo incompleto e as pastas já criadas. A preparação não cria arquivos ou pastas.

Cada arquivo é gravado primeiro em um temporário no destino. Na substituição, o destino anterior é preservado até a troca final. Se a troca falhar, o app tenta restaurá-lo. Quando não consegue restaurar ou remover o backup, a mensagem e o log indicam seu caminho.

Se um dispositivo for desconectado, ocorrer falta de espaço ou a origem/destino mudar durante a operação, a cópia é interrompida e o resumo informa a etapa. Confira os destaques de itens concluídos antes de repetir o lote.

## Limites

- 4096 itens marcados por pasta de origem.
- 16384 arquivos e pastas preparados por lote.
- 8 MiB para os caminhos do lote; 1023 bytes por caminho.
- Blocos de cópia de 256 KiB, sem carregar o arquivo inteiro em memória.
- Tamanhos de 64 bits. USB FAT32: até 4 GiB menos 1 byte por arquivo.

Para pastas acima desses limites, copie suas subpastas em lotes menores. Destino dentro da própria origem e itens não suportados são recusados antes da gravação. O acesso é aos dispositivos montados pelo sistema; esta versão não acrescenta suporte a NTFS/exFAT.

A cópia do gerenciador preserva os bytes. Para redimensionar capas, inside e disco, use as opções de artes via USB na biblioteca.
