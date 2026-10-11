# Postar PS3 Game Orbit v1.4.1 no GitHub

Este pacote contém o PKG, os fontes públicos, as instruções e os relatórios de validação. Você pode atualizar o mesmo repositório das versões anteriores.

## 1. Extrair e conferir

1. Extraia `PS3_GAME_ORBIT_v1.4.1_TESTE_GITHUB_COMPLETO.zip` no computador.
2. Em `instalar`, localize `PS3_GAME_ORBIT_v1.4.1_TESTE.gnpdrm.pkg` e `SHA256SUMS.txt`.
3. A pasta `PS3-Game-Orbit` contém o conteúdo que será colocado na raiz do repositório. As pastas `instalar` e `validacao` são materiais da entrega; o PKG pode ser anexado ao release.
4. Teste esse PKG no seu PS3 conforme `PS3-Game-Orbit/docs/TESTE_1.4.1.md`. A compilação e os testes no computador não substituem esse teste físico.

## 2. Atualizar os fontes

Para este projeto com muitas pastas, o caminho mais simples é usar uma cópia local do repositório pelo GitHub Desktop ou Git. Copie o conteúdo de `PS3-Game-Orbit` sobre essa cópia, preservando a pasta `.git` do repositório. Inclua a pasta `.github` do pacote. Confira as mudanças e faça commit com a mensagem `PS3 Game Orbit v1.4.1`, depois envie com Push.

Não copie OBJ, PSD, FBX nem `src/jfx_case_data_fix29.inc` de uma pasta de trabalho sua: o modelo editável é licenciado e está excluído desta entrega pública. Os scripts permitem gerar a malha localmente usando a cópia obtida por cada desenvolvedor.

Se preferir o navegador, use **Add file → Upload files** para enviar os arquivos extraídos, mantendo a estrutura de pastas. Faça lotes de até 100 arquivos; o limite é 25 MiB por arquivo nesse caminho. Não coloque a pasta externa `PS3-Game-Orbit` como uma subpasta acidental do repositório. [Documentação oficial: envio de arquivos](https://docs.github.com/pt/repositories/working-with-files/managing-files/adding-a-file-to-a-repository).

Depois do envio, a página principal deve mostrar `README.md`, `Makefile`, `src`, `include`, `assets`, `pkgfiles`, `scripts`, `tests` e `docs`. Os testes automáticos disponíveis na aba Actions cobrem a parte pública; não compilam o PKG nem publicam releases.

## 3. Criar o release

1. No repositório atualizado, abra **Releases → Draft a new release**.
2. Crie a tag **v1.4.1-rc.1**, apontando para o branch que recebeu os fontes.
3. Use o título **PS3 Game Orbit v1.4.1 — TESTE** e cole o conteúdo de `docs/RELEASE_NOTES_1.4.1.md` na descrição.
4. Anexe o PKG de `instalar` e seu `SHA256SUMS.txt`. Você também pode anexar o ZIP completo que baixou nesta conversa.
5. Marque **This is a pre-release**, pois o teste físico desta entrega ainda precisa ser confirmado por você.
6. Confira os anexos e use **Publish release**, ou **Save draft** para revisar antes.

[Documentação oficial: gerenciamento de releases](https://docs.github.com/pt/repositories/releasing-projects-on-github/managing-releases-in-a-repository).

Após sua validação física, você pode atualizar as notas com os modelos/firmwares testados e publicar uma versão estável com a tag `v1.4.1`. Se houver qualquer alteração no binário, gere novos hashes; não reutilize os hashes desta entrega para outro arquivo.

## 4. Conferir o download

Abra o release publicado e baixe novamente o PKG. O arquivo deve ter o mesmo SHA-256 indicado em `instalar/SHA256SUMS.txt`. O download instalável é o PKG anexado: os arquivos automáticos “Source code” do GitHub contêm apenas os fontes registrados no repositório.

No Windows, você pode conferir com PowerShell:

```powershell
Get-FileHash .\PS3_GAME_ORBIT_v1.4.1_TESTE.gnpdrm.pkg -Algorithm SHA256
```

No Linux:

```bash
sha256sum -c SHA256SUMS.txt
```
