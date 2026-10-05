# Publicar PS3 Game Orbit v1.3.3 no GitHub

Este guia usa o repositório existente e o ZIP **PS3_GAME_ORBIT_v1.3.3_TESTE_GITHUB_COMPLETO.zip**. O pacote não publica nada automaticamente.

## 1. Extrair

Extraia o ZIP no computador. O conteúdo está organizado assim:

| Item | Uso |
| --- | --- |
| PS3-Game-Orbit/ | Código e arquivos que devem ir para a raiz do repositório. |
| release/ | PKG instalável, notas, instruções e SHA-256. |
| validacao/ | Relatórios dos testes e da compilação desta versão. |
| LEIA_ME_1.3.3.md | Instruções gerais do pacote. |
| GUIA_GITHUB_1.3.3.md | Cópia deste passo a passo. |

## 2. Atualizar os fontes

Abra seu repositório no GitHub. Envie o **conteúdo de PS3-Game-Orbit**, mantendo os caminhos: README.md e Makefile na raiz, arquivos de src dentro de src, e assim por diante. Não crie outra pasta PS3-Game-Orbit dentro da raiz.

Pelo navegador, use **Add file → Upload files** (Adicionar arquivo → Carregar arquivos), arraste os arquivos/pastas e confirme a alteração. O envio web aceita até 100 arquivos por vez e 25 MiB por arquivo; faça lotes menores se necessário. Mensagem sugerida: `Atualiza PS3 Game Orbit para v1.3.3`.

Se o GitHub propuser uma nova branch, conclua o pull request para que a atualização entre na branch principal antes de criar a release. Se você usa GitHub Desktop, substitua os arquivos na cópia local do repositório, revise as mudanças, faça commit e envie-as.

Confira especialmente README.md, CHANGELOG.md, VERSION, Makefile, assets/PARAM_1_3_3.xml, include/, src/, scripts/, tests/, docs/ e o workflow de verificações que acompanha a pasta de fontes. O workflow faz testes; ele não publica o PKG.

O ZIP público já exclui OBJ/PSD e a malha editável derivada do JFX. A preparação para recompilar está em docs/BUILD.md. O PKG pronto não exige que o usuário faça essa preparação.

## 3. Criar a release

1. Abra **Releases → Draft a new release**.
2. Em **Choose a tag**, crie `v1.3.3-rc.1`. Selecione a branch que já recebeu todos os fontes desta versão.
3. Título: **PS3 Game Orbit v1.3.3 TESTE**.
4. Cole o texto de **release/RELEASE_NOTES.md** no campo de descrição.
5. Anexe **PS3_GAME_ORBIT_v1.3.3_TESTE.gnpdrm.pkg**, **SHA256SUMS.txt** e, se desejar, o ZIP completo e o guia de instruções.
6. Marque **This is a pre-release** enquanto esta versão aguarda teste físico no PS3.
7. Aguarde os anexos terminarem de enviar; use **Publish release**, ou **Save draft** para revisar depois.

Os anexos vêm de release/; o PKG deve ficar nos downloads da release. Os arquivos Source code (zip/tar.gz) que o GitHub oferece automaticamente refletem os fontes da tag e não incluem necessariamente o instalador anexado.

## 4. Conferir a publicação

Abra a release e confira o título/tag, descrição e o nome **v1.3.3** do PKG. Baixe o instalador anexado e compare seu SHA-256 com release/SHA256SUMS.txt.

No PowerShell, dentro da pasta do PKG:

```powershell
Get-FileHash .\PS3_GAME_ORBIT_v1.3.3_TESTE.gnpdrm.pkg -Algorithm SHA256
```

No Linux:

```bash
sha256sum PS3_GAME_ORBIT_v1.3.3_TESTE.gnpdrm.pkg
```

Após confirmar o funcionamento no PS3, atualize a indicação de teste nas notas. Preserve as tags e os pacotes antigos para permitir comparação entre versões.

## Referências

Procedimento conferido na documentação oficial do GitHub:

- [Adicionar arquivos pelo navegador](https://docs.github.com/pt/repositories/working-with-files/managing-files/adding-a-file-to-a-repository)
- [Criar e gerenciar releases](https://docs.github.com/pt/repositories/releasing-projects-on-github/managing-releases-in-a-repository)
