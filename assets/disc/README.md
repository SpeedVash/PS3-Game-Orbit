# Disco original do PS3 Game Orbit — 1.3.1

120 mm de diâmetro, furo real de 15 mm, espessura de 1,2 mm e origem central. A variante nativa leve usa 48 segmentos, 1.056 triângulos e cinco materiais.

Na 1.3.1, o anel central usa material **opaco e prateado**, mantendo o furo e todos os vértices, índices, normais e UVs aprovados. O JSON `DISCO_DADOS_NATIVOS_MM.json` contém as duas variantes em milímetros, com esse material atualizado. A representação usada pelo aplicativo está em `src/disc_data_fix32.inc`, com a variante opaca habilitada por FIX34. Os ramos anteriores preservam o material antigo para testes históricos.

O rótulo personalizado do jogo vem de `_DISC.png`/JPG/JPEG. Na ausência dessa arte, usa `pkgfiles/USRDIR/ORBIT_DISC_LABEL.png`; o verso continua usando `ORBIT_DISC_BACK.png`, ambos com 512 × 512. A arte não é gerada da capa nem baixada automaticamente. Bordas, chanfros e centro usam o shader original do aplicativo, sem reflexos PBR ou refração.

A geometria do disco foi criada para este projeto e acompanha os fontes. A caixa JFX continua sendo um ativo separado sob os termos do autor e não acompanha o repositório em formato editável.
