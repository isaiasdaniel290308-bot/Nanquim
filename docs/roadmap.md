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
| Interface nova, peça 3 (design "Main"): galeria com novas páginas e recentes | Em teste | 0018 |
| Pincéis do Nanquim (motor nativo, leves) no lugar dos MyPaint no conjunto inicial | Próximo | — |
| Interface nova, próximas peças: galeria inicial, ferramentas recolhíveis, boas-vindas | Próximo | — |
| Importador `.sut` (Clip Studio) | Planejado | — |
| Liquidificar mais rápido (prévia leve + aplicação em resolução total) | Planejado | — |
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
