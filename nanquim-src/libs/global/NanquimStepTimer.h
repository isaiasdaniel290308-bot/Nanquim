/*
 *  SPDX-FileCopyrightText: 2026 Nanquim
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef NANQUIM_STEP_TIMER_H
#define NANQUIM_STEP_TIMER_H

#include <QElapsedTimer>
#include <QString>

#include "KisUsageLogger.h"

#ifdef Q_OS_ANDROID
#include <KSharedConfig>
#include <KConfigGroup>
#endif

/**
 * Nanquim: cronômetro de etapas. Cada mark() escreve no log do Krita
 * ("[Nanquim tempo] <prefixo>: <etapa> - N ms") o tempo desde a marca anterior.
 *
 * Quando registrar: no Android, com -DNANQUIM_TIMING na compilação OU com o
 * interruptor de diagnóstico ligado no app (Configurações › Desempenho ›
 * "Registrar tempos no log" — entrada Nanquim/tempos do kritarc). A leitura
 * da configuração acontece uma vez por processo; fora disso não faz nada.
 */
class NanquimStepTimer
{
public:
    explicit NanquimStepTimer(const QString &prefix)
        : m_prefix(prefix)
    {
        m_timer.start();
    }

    // minMs: só registra etapas que levaram pelo menos isso (evita poluir o log)
    void mark(const QString &step, qint64 minMs = 0)
    {
#if defined(Q_OS_ANDROID)
        const qint64 ms = m_timer.restart();
        if (ms >= minMs && timingOn()) {
            KisUsageLogger::log(QStringLiteral("[Nanquim tempo] %1: %2 - %3 ms").arg(m_prefix, step).arg(ms));
        }
#else
        Q_UNUSED(step);
        Q_UNUSED(minMs);
        Q_UNUSED(m_timer.restart());
#endif
    }

private:
    static bool timingOn()
    {
#ifdef NANQUIM_TIMING
        return true;
#else
        static const bool s_on = KConfigGroup(KSharedConfig::openConfig(), QStringLiteral("Nanquim")).readEntry("tempos", false);
        return s_on;
#endif
    }

    QString m_prefix;
    QElapsedTimer m_timer;
};

#endif
