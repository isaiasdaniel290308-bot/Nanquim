/*
 *  SPDX-FileCopyrightText: 2026 Nanquim
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef NANQUIM_TABLET_UI_H
#define NANQUIM_TABLET_UI_H

class KisMainWindow;
class QWidget;
class QPoint;

/**
 * Nanquim: interface de tablet (primeira peça).
 *
 * - barra do topo (design "Desenho"): galeria, título do desenho, desfazer/refazer,
 *   editor de pincel, Pincéis, Cor, Camadas, Opções, Réguas, Referência e Mais;
 * - barra de ferramentas à esquerda: 8 ferramentas, tamanho e opacidade na vertical,
 *   botão de recolher;
 * - os painéis abrem um de cada vez ao lado do trilho e fecham com outro toque;
 * - a barra de menus vira o botão "Mais", e as barras padrão do Krita somem;
 * - a primeira tela de desenho da sessão começa limpa (só a caixa de ferramentas).
 *
 * Nada aqui muda os dados ou as ações do Krita: só reorganiza o que já existe.
 */
namespace NanquimTabletUi
{
// monta a barra e o trilho (uma vez, no fim do construtor da janela)
void install(KisMainWindow *mainWindow);

// reaplica o arranjo depois que o Krita troca entre a tela inicial e a de desenho
// (o Krita restaura menus, barras e painéis salvos nessas trocas)
void applyLayout(KisMainWindow *mainWindow, bool canvasMode);

// roda de atalhos do Nanquim no lugar da paleta do Krita (botão da S Pen);
// devolve false no modo Clássico, e aí o Krita abre a paleta dele
bool toggleWheel(QWidget *canvasWidget, const QPoint &pos);
}

#endif
