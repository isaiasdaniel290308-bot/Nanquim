/*
 *  SPDX-FileCopyrightText: 2026 Nanquim
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef NANQUIM_SUT_IMPORT_H
#define NANQUIM_SUT_IMPORT_H

#include <QString>

class QWidget;

/**
 * Nanquim: importa subferramentas do Clip Studio Paint (.sut) como pincéis
 * do Krita (ponta por imagem, textura de papel, curvas de pressão/inclinação/
 * velocidade/aleatório, espessura e rotação) e pacotes .zip com pincéis.
 */
namespace NanquimSutImport
{
// devolve true se o pincel foi criado; "message" recebe o texto para o usuário
bool importFile(QWidget *parent, const QString &path, QString *message);

// extrai e importa automaticamente .sut, .kpp e .bundle de dentro de um .zip
bool importZipArchive(QWidget *parent, const QString &zipPath, QString *message);
}

#endif
