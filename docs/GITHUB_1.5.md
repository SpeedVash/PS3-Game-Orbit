# Postar PS3 Game Orbit v1.5 no GitHub

## Preparar os arquivos

Extraia `PS3_GAME_ORBIT_v1.5_GITHUB_COMPLETO.zip`. O conteúdo de `PS3-Game-Orbit` é o repositório; `instalar` contém o PKG e seu SHA-256; `validacao` contém os relatórios desta compilação.

Use o repositório que já contém o PS3 Game Orbit. Para um primeiro envio, crie um repositório chamado `PS3-Game-Orbit`. Não é necessário instalar plugins.

## Atualizar o código

1. Abra a página do repositório e use **Add file → Upload files**.
2. Envie o conteúdo da pasta `PS3-Game-Orbit` mantendo a estrutura de pastas. README.md deve ficar na raiz do repositório, não dentro de uma segunda pasta PS3-Game-Orbit.
3. O upload pelo navegador aceita até 100 arquivos por envio e 25 MiB por arquivo. Divida as pastas maiores em lotes ou use GitHub Desktop para atualizar o repositório inteiro.
4. Use a mensagem `PS3 Game Orbit v1.5`, confira as alterações e conclua o commit. Se a página exigir um branch e pull request, conclua essa etapa antes de criar a tag da versão.

Use a pasta de fontes extraída; o ZIP completo é um anexo do release. Ela já exclui o OBJ licenciado e a representação editável da malha. Preserve os arquivos de licença e créditos.

## Criar o release

1. Abra **Releases** e escolha **Draft a new release**.
2. Crie a tag **v1.5**, apontando para o branch com os arquivos atualizados.
3. Título: **PS3 Game Orbit v1.5 — SpeedVash**.
4. Copie o conteúdo de `docs/RELEASE_NOTES_1.5.md` para a descrição.
5. Anexe `PS3_GAME_ORBIT_v1.5.gnpdrm.pkg`, `SHA256SUMS.txt` da pasta `instalar` e, se quiser oferecer a entrega completa, este ZIP.
6. Confira título, tag e anexos. Use **Save draft** para guardar ou **Publish release** quando decidir disponibilizar.

Esta compilação foi verificada no computador; o teste físico da v1.5 no PS3 permanece pendente. O roteiro está em `docs/TESTE_1.5.md`. Mantenha essa informação nas notas até testar no console.

## Documentação oficial

- [Adicionar arquivos](https://docs.github.com/pt/repositories/working-with-files/managing-files/adding-a-file-to-a-repository)
- [Gerenciar releases](https://docs.github.com/pt/repositories/releasing-projects-on-github/managing-releases-in-a-repository)
