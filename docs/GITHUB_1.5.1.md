# Postar PS3 Game Orbit v1.5.1 no GitHub

## Preparar

Extraia `PS3_GAME_ORBIT_v1.5.1_GITHUB_COMPLETO.zip`. O conteúdo da pasta `PS3-Game-Orbit` vai para a raiz do repositório. A pasta `instalar` contém o PKG e seu SHA-256; `validacao` contém os relatórios.

Use o repositório existente do PS3 Game Orbit. Em um primeiro envio, crie um repositório chamado `PS3-Game-Orbit`.

## Atualizar o código

1. Abra o repositório e clique em **Add file → Upload files**.
2. Arraste o conteúdo de `PS3-Game-Orbit`, preservando as pastas. `README.md` deve ficar na raiz, sem uma segunda pasta `PS3-Game-Orbit` envolvendo os arquivos.
3. O navegador aceita até 100 arquivos por envio e 25 MiB por arquivo. Envie em lotes ou use GitHub Desktop para atualizar a árvore inteira.
4. Confira os arquivos e faça o commit com a mensagem **PS3 Game Orbit v1.5.1**. Se o branch exigir pull request, conclua essa etapa antes de criar a tag.

Não envie o PKG para a árvore de fontes: ele será um anexo do release. O ZIP público já exclui o OBJ licenciado e o include da malha. Preserve créditos e avisos de licença.

## Criar o release

1. Abra **Releases → Draft a new release**.
2. Em **Choose a tag**, crie **v1.5.1** e escolha o branch com o código atualizado.
3. Título: **PS3 Game Orbit v1.5.1 — SpeedVash**.
4. Copie o texto de `NOTAS_RELEASE_1.5.1.md` ou `PS3-Game-Orbit/docs/RELEASE_NOTES_1.5.1.md` para a descrição.
5. Anexe `instalar/PS3_GAME_ORBIT_v1.5.1.gnpdrm.pkg` e `instalar/SHA256SUMS.txt`. Anexe também `PS3_GAME_ORBIT_v1.5.1_GITHUB_COMPLETO.zip` para oferecer a entrega completa.
6. Confira tag, título e anexos. **Save draft** guarda o rascunho; **Publish release** disponibiliza a versão.

O código enviado não compila um PKG automaticamente; o instalador incluído já foi compilado. O workflow de testes valida os fontes públicos.

Os testes no computador passaram, mas o teste físico desta versão no PS3 está pendente. Use `PS3-Game-Orbit/docs/TESTE_1.5.1.md` e mantenha essa informação nas notas até concluir o teste no console.

## Referências oficiais

- [Adicionar arquivos ao repositório](https://docs.github.com/pt/repositories/working-with-files/managing-files/adding-a-file-to-a-repository)
- [Gerenciar releases](https://docs.github.com/pt/repositories/releasing-projects-on-github/managing-releases-in-a-repository)
