# PS3 Game Orbit v1.5.1

Criado por **SpeedVash**.

- **Gerenciador HD/USB com dois painéis**: navegação independente, seleção múltipla, cópia entre dispositivos, progresso e cancelamento. Arquivos marcados aparecem em azul e cópias concluídas em verde.
- **Confirmação de sobrescrita** por arquivo ou para todo o lote, com opções para ignorar e cancelar.
- **Acesso aos arquivos revisado** para corrigir falhas de abertura/verificação: API nativa consistente, dados do caminho e descritor separados, espaço livre no destino e erros com etapa/código.
- **Tipo de caixa por jogo**, em Triângulo: padrão transparente, Platinum cinza opaco ou Greatest Hits vermelha. A cor afeta somente o plástico externo; interior transparente e artes preservados. A opção fica salva e funciona nos três layouts.
- **Pré-carga adiada durante navegação e rotação manual**, com 650 ms de repouso e transições concluídas antes de retomar. Mantidos os caches Clássico 7/7/7, Spine 15/15/15 e Lista 5/5/5.
- Mantidos ordenação A-Z/Z-A/Custom, configurações, caixa selecionada completa fechada/aberta, webMAN MOD e importação parcial/em lote das artes via `USB/PS3COVERS`.
- Versão **1.5.1** no início e menus, identidade **PGORBT301** e APP_VER **01.11**.

Instale o PKG por cima da versão anterior. No Super Slim, habilite HEN antes de abrir; mantenha webMAN MOD ativo para montar os jogos.

O gerenciador copia arquivos individuais; pastas são usadas para navegar. USB precisa ser reconhecido pelo sistema: esta entrega não acrescenta NTFS/exFAT. Arquivos no FAT32 estão sujeitos ao limite de 4 GiB menos 1 byte. A atualização de capas usa `PS3COVERS` na raiz do pendrive e aceita apenas uma das três artes.

**Compilação e testes no computador concluídos; teste físico da v1.5.1 no PS3 pendente.** A carga inicial de imagens necessárias ainda é síncrona; a melhoria de fluidez precisa ser conferida no console.

O ZIP inclui PKG, fontes públicos, guia do GitHub, notas, roteiro de teste e relatórios com hashes. A malha de terceiros em formato editável não acompanha a distribuição pública; as instruções de compilação e os créditos indicam como obtê-la.
