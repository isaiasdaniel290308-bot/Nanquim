# Roteiro do Nanquim

| Item | Estado | Patch |
| --- | --- | --- |
| Build Android na nuvem, APK assinado | Feito | workflow |
| Tirar plugins pouco usados do Android | Feito | 0001 |
| Cronômetro da abertura no log | Feito | 0002 |
| Nome "Nanquim" no Android | Feito | 0003 |
| Abertura em ~15 s (sem o painel QML de texto) | Feito | 0004 |
| Abrir `.kpp` e `.bundle` pelo "Abrir com" | Feito (testado no Tab S10 Lite) | 0005, 0007 |
| Abertura animada nativa, ícone da gota, nome Nanquim | Feito | 0006, 0008, 0009 |
| Padrões leves no Android (filtro trilinear, prévia instantânea, 100 desfazer, teto de memória 35%) | Em teste | 0010, 0012 |
| Medir os ~10 s restantes da janela: KisViewManager ~4 s e ações das ferramentas ~6 s; agora detalhados | Em teste | 0011, 0012 |
| Interface nova, peça 1: barra do topo, trilho de painéis à direita, menu em "Mais" | Feito (testado no Tab S10 Lite) | 0011, 0013, 0014 |
| Abertura: ações repetidas das ferramentas (6,1 s → 1,9 s; abertura total ~10 s) | Feito | 0014 |
| Abertura: caixa de pincéis (KisPaintopBox, ~2,3 s) criada sob demanda | Próximo | — |
| Animações: saída da abertura, painéis deslizando, troca de tela | Feito | 0015 |
| Interface nova, peça 2 (design "Desenho"): barra do topo com título, ferramentas à esquerda, tamanho/opacidade verticais, recolher | Em teste | 0016 |
| Interface nova, peça 3 (design "Main"): galeria com novas páginas e recentes | Feito (testado no Tab S10 Lite) | 0018 |
| Time-lapse automático (quadro por traço, sem pausas) e exportação em MP4 | Em teste | 0026 |
| Inserir imagem como camada; remover desenhos da galeria | Em teste | 0025 |
| Pincéis do Nanquim: etiqueta com pincéis leves do motor nativo, aberta por padrão | Em teste | 0030 |
| Boas-vindas na primeira abertura e guia rápido | Em teste | 0030 |
| Interface nova, próximas peças: galeria inicial, ferramentas recolhíveis, boas-vindas | Próximo | — |
| Importador `.sut` (Clip Studio), v1: ponta pela miniatura, ajustes básicos | Em teste | 0032 |
| Liquidificar mais rápido: aplicação em várias threads | Em teste | 0019 |
| Liquidificar mais rápido: prévia leve enquanto arrasta | Planejado | — |
| Modo de interface "Clássico" (desktop), alternável nas configurações | Projetado | — |
| Versão para Windows (usa o modo Clássico por padrão) | Ideia | — |

## Importador `.sut`

Um `.sut` é um banco SQLite. Plano: ler as tabelas da ferramenta (`Node`, `Variant`, `MaterialFile`), extrair a ponta (PNG) e a textura, e converter tamanho, espaçamento, pressão e opacidade para um preset `.kpp` do motor de pincel padrão do Krita. O traço fica parecido, não idêntico. Referência: os scripts de conversão da comunidade do Krita (Freya Lupen).

## Liquidificar

A ferramenta já existe no Krita (Transformar → Liquidificar). Plano:
1. Enquanto o dedo ou a caneta arrasta, deformar uma versão reduzida da camada (nível de detalhe), só na área do pincel.
2. Ao soltar, aplicar a deformação em resolução total, em várias threads.
3. Interface própria: barra inferior com Empurrar, Girar, Inflar, Encolher e Restaurar.

## Modos de interface: Tablet e Clássico

Duas interfaces sobre o mesmo motor, trocadas em Configurações → Interface (e por um botão no topo):

- **Tablet** (padrão no Android): barra fina, ferramentas recolhíveis, painéis que abrem só quando tocados.
- **Clássico** (padrão no Windows): menus no topo, barra de opções da ferramenta, caixa de ferramentas à esquerda, abas de documentos, coluna de pincéis rápidos ao lado da tela e painéis fixos à direita (cor, histórico, camadas).

Como fazer no código: o Krita já salva e troca arranjos de janela ("workspaces", arquivos `.kws`). O modo Clássico pode começar como um workspace do Nanquim mais um tema escuro com o azul-ciano. O modo Tablet é a interface nova. A troca salva a escolha e reabre a janela no modo escolhido.

Para Windows, o mesmo repositório pode ganhar um segundo workflow que compila o instalador `.exe`, com os mesmos patches.

## Formato `.sut`, estudado em 4 arquivos reais (2026-11-10)

- O arquivo é SQLite, com as tabelas `Node` (nome do pincel), `Variant` (cerca de 190 parâmetros) e `MaterialFile`.
- Parâmetros lidos sem problema: `BrushSize`, `Opacity`, `BrushFlow`, `BrushHardness`, `BrushInterval`, `BrushThickness`, `BrushRotation`, `AntiAlias`, `BrushUsePatternImage`, `TextureDensity`, entrada/saída do traço (`BrushUseIn/Out`), fita (`BrushRibbon`).
- As curvas de pressão ficam em blobs (`BrushSizeEffector`, `BrushFlowEffector`), ainda não decifrados.
- `MaterialFile.FileData` é um arquivo **tar**. Dentro: `thumbnail/thumbnail.png` (PNG comum, até 300 px, com a ponta ou a textura de papel) e `data/material_0.layer` (contêiner C2F paginado com a imagem em tamanho real; ainda não decifrado).
- Plano da v1: ponta a partir do `thumbnail.png`, parâmetros simples, pressão no tamanho por padrão. Limites conhecidos: ponta com no máximo 300 px, curvas de pressão e "fita" (ponta que acompanha o traço) não convertidas.
