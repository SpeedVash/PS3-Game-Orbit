# Testar a v1.5.2 no PS3

Instale o PKG sobre a versão anterior. Habilite HEN quando necessário e confira “1.5.2” no início, cabeçalho e menu. Use arquivos de teste e uma pasta de destino separada para verificar o gerenciador.

## Navegação e origem

1. Abra START → Gerenciador e confirme o aviso.
2. Confira L1/R1 para painéis, X para abrir pastas, Círculo para voltar, R3 para HD/USB e L2/R2 para páginas.
3. Marque dois arquivos com Quadrado no HD. Troque ao painel USB, navegue até uma subpasta e abra Triângulo → Copiar.
4. Confira que a direção continua HD → USB, com quantidade e tamanho corretos. Cancele a confirmação: nada deve ser gravado.
5. Repita, confirme e compare os arquivos. Eles devem aparecer como copiados na origem e no destino.
6. Confira Marcar todos e Limpar seleção. Marcar no outro painel deve pedir que a seleção anterior seja limpa. Mudar a pasta da origem deve limpar as marcações.

## Pastas e conflitos

1. Prepare uma pasta com dois arquivos, uma subpasta com outro arquivo e uma pasta vazia.
2. Marque a pasta e copie. Confira a contagem, a estrutura completa e a pasta vazia no destino.
3. Repita em uma pasta de destino já existente que tenha um arquivo adicional. Ele deve permanecer.
4. Teste sobrescrever um, pular um, sobrescrever todos e pular todos. Em um novo lote, o diálogo deve aparecer novamente.
5. Tente copiar uma pasta para dentro dela mesma e tente conflito entre arquivo e pasta. Deve ser recusado durante a preparação.

## Cancelamento e retorno

1. Marque um arquivo pequeno e outro grande. Cancele durante o segundo: o primeiro deve permanecer concluído, e o segundo destino anterior deve permanecer intacto.
2. Use START durante a cópia. “Continuar” deve retomar; “Cancelar e sair” deve voltar à biblioteca preservando o que já terminou.
3. Cancele durante a preparação de uma pasta grande. O destino deve permanecer sem novos arquivos/pastas.
4. Teste com fundo animado ligado e depois desligado. Ele deve ficar pausado no gerenciador e voltar à preferência anterior ao sair.
5. Confira SELECT com um caminho longo: todas as partes devem ser acessíveis em páginas.

## Recursos preservados

Confira layouts Clássico, Spine e Lista, favoritos, A-Z/Z-A/Custom, cores de caixa por jogo, abertura da caixa, artes via USB, salvamento das configurações e montagem pelo webMAN.

Ao relatar um problema, informe modelo do PS3, HEN/CFW, origem e destino, ação escolhida, texto/código do erro e, se possível, o log. O teste do computador não substitui esta verificação de controles, acesso aos dispositivos e fluidez no console.
