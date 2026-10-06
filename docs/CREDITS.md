# Créditos e licenças

- **PS3 Game Orbit:** projeto de SpeedVash.
- **Modelo exterior:** JFX DVD/Bluray, obtido pelo usuário no TurboSquid. O modelo editável e a malha convertida não acompanham o repositório público. Consulte `assets/jfx_bluray/README.md` para origem e preparo local.
- **Interior, articulações e disco da v1.4:** construção procedural em `src/case_model_fix37.cpp`, integrada à malha exterior preparada localmente. Usa os shaders RSX arquivados do projeto.
- **PSL1GHT/ps3aqua:** SDK e ferramentas nos commits fixados descritos em `BUILD.md`, sujeitos às respectivas licenças upstream.
- **Fonte Noto Sans:** licença SIL Open Font License em `assets/fonts/OFL.txt`, também distribuída no PKG.
- **stb_image_write v1.16:** Sean Barrett e colaboradores; cabeçalho upstream do commit `2c980bb59875b0d32144a71867fbdebb2f77cd20`. Licença MIT ou domínio público em `assets/licenses/STB_LICENSE.txt`; cópia incluída em `USRDIR` do PKG. Usado para codificar as imagens importadas.

Não há jogos comerciais nem suas capas incluídos nesta entrega. As artes de exemplo são demonstrativas. Nenhuma licença geral nova é atribuída aos fontes históricos do projeto; as licenças de terceiros permanecem próprias.

## v1.4.1

O fechamento usa a mesma malha aprovada em repouso, agrupada para o RSX. A borda dupla do disco, a correção da sobreposição e o armazenamento com backup são código do projeto. ORBIT_DISC_BACK.png é uma textura procedural reproduzível por scripts/bake-disc-back-fix38.py; a iluminação continua nos shaders originais e não é reflexão de ambiente em tempo real. A arte da marca e a fonte mantêm os créditos anteriores.
