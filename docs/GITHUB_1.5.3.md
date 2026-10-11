# Postar PS3 Game Orbit v1.5.3 no GitHub

## Preparar

Os fontes da v1.5.3 estão no repositório. O instalador compilado é `PS3_GAME_ORBIT_v1.5.3.gnpdrm.pkg`; confira seu SHA-256 em `docs/RELEASE_1.5.3.json`.

## Fontes atualizados

O repositório `SpeedVash/PS3-Game-Orbit` contém a correção, os testes e os ativos públicos. O workflow verifica os fontes; não compila nem publica o instalador automaticamente.

O PKG deve ser anexo do release. Preserve créditos e avisos de licença; o OBJ licenciado e o include da malha permanecem locais.

## Criar o release

1. Abra **Releases → Draft a new release**.
2. Em **Choose a tag**, crie **v1.5.3** e escolha o branch com o código atualizado.
3. Título: **PS3 Game Orbit v1.5.3 — SpeedVash**.
4. Copie o texto de `docs/RELEASE_NOTES_1.5.3.md` para a descrição.
5. Anexe `PS3_GAME_ORBIT_v1.5.3.gnpdrm.pkg` e, se desejar, as notas e o relatório de validação. O GitHub oferece os fontes da tag em ZIP automaticamente.
6. Confira tag, título e anexos. **Save draft** guarda o rascunho; **Publish release** disponibiliza a versão.

Os testes no computador passaram, mas o teste físico desta versão no PS3 está pendente. Use `PS3-Game-Orbit/docs/TESTE_1.5.3.md` e mantenha essa informação nas notas até concluir o teste no console.

## Referências oficiais

- [Adicionar arquivos ao repositório](https://docs.github.com/pt/repositories/working-with-files/managing-files/adding-a-file-to-a-repository)
- [Gerenciar releases](https://docs.github.com/pt/repositories/releasing-projects-on-github/managing-releases-in-a-repository)
