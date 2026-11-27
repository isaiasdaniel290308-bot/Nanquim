/*
 *  SPDX-FileCopyrightText: 2026 Nanquim
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef NANQUIM_WHEEL_H
#define NANQUIM_WHEEL_H

#include <QWidget>
#include <QPainter>
#include <QPainterPath>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QConicalGradient>
#include <QImage>
#include <QStringList>
#include <QVector>
#include <QTimer>
#include <QPointer>
#include <QtMath>
#include <functional>

/**
 * Nanquim: roda de atalhos (build 60).
 *
 * Anel de fora: oito pincéis (borracha no topo, três favoritos e quatro
 * recentes). Anel do meio: matiz. Centro: triângulo de saturação e brilho.
 * Acima da roda: o pincel ativo e o tamanho; abaixo: a camada ativa, que
 * abre o painel de camadas.
 *
 * O widget não conhece o Krita: recebe os pincéis e a cor, e avisa pelas
 * funções onColor, onBrush, onLayers e onClose.
 */
class NanquimWheel : public QWidget
{
public:
    static constexpr int R_OUT = 216;
    static constexpr int R_LAYER_IN = 150;   // anel dos pincéis: de R_LAYER_IN a R_OUT
    static constexpr int R_HUE_OUT = 146;
    static constexpr int R_HUE_IN = 112;
    static constexpr int R_TRI = 104;
    static constexpr int LABEL_H = 54;   // faixa acima da roda para as etiquetas
    static constexpr int FOOT_H = 58;    // faixa abaixo da roda: camada ativa
    static constexpr int SECTORS = 8;

    enum Kind { Empty = -1, Eraser = 0, Favorite = 1, Recent = 2 };
    struct Brush {
        QString name;
        QString size;
        QImage thumb;
        int kind = Empty;
    };

    std::function<void(const QColor &)> onColor;
    std::function<void(int)> onBrush;
    std::function<void()> onLayers;
    std::function<void()> onClose;

    explicit NanquimWheel(QWidget *parent)
        : QWidget(parent)
    {
        setObjectName(QStringLiteral("nanquimWheel"));
        setAttribute(Qt::WA_TranslucentBackground);
        setAttribute(Qt::WA_NoSystemBackground);
        setFocusPolicy(Qt::StrongFocus);
        setFixedSize(2 * R_OUT + 12, 2 * R_OUT + 12 + LABEL_H + FOOT_H);
    }

    // centro da roda em coordenadas do widget
    QPointF center() const { return QPointF(width() / 2.0, LABEL_H + 6 + R_OUT); }

    void setBrushes(const QVector<Brush> &brushes, int active, const QString &layer)
    {
        m_brushes = brushes.mid(0, SECTORS);
        m_active = active;
        m_layer = layer;
        update();
    }

    void setColor(const QColor &c)
    {
        qreal h, s, v, a;
        c.getHsvF(&h, &s, &v, &a);
        if (h >= 0) m_hue = h;   // cinza não tem matiz: guarda a de antes
        m_sat = s;
        m_val = v;
        m_triangle = QImage();
        m_triangleDirty = false;
        update();
    }

    QColor color() const { return QColor::fromHsvF(m_hue, m_sat, m_val); }

    // posiciona a roda com o centro em p (coordenadas do pai), sem sair do pai
    void showAt(const QPoint &p)
    {
        const QPointF c = center();
        QPoint topLeft = p - c.toPoint();
        if (parentWidget()) {
            const QRect area = parentWidget()->rect();
            topLeft.setX(qBound(area.left(), topLeft.x(), qMax(area.left(), area.right() - width() + 1)));
            topLeft.setY(qBound(area.top(), topLeft.y(), qMax(area.top(), area.bottom() - height() + 1)));
        }
        move(topLeft);
        show();
        raise();
        setFocus();
    }

    static QColor hueColor(qreal h) { return QColor::fromHsvF(h, 1.0, 1.0); }

    // ponto (coordenadas do widget) dentro da roda ou da etiqueta da camada
    bool hits(const QPointF &p) const
    {
        return QLineF(center(), p).length() <= R_OUT || (!m_layer.isEmpty() && layerPill().contains(p));
    }

protected:
    // ângulo da tela (0° à direita, sentido horário) para a matiz:
    // vermelho no topo e a volta no sentido horário, como no design
    static qreal angleOf(const QPointF &d) { return qRadiansToDegrees(std::atan2(d.y(), d.x())); }
    static qreal hueToAngle(qreal hue) { return hue * 360.0 - 90.0; }
    static qreal angleToHue(qreal deg) { qreal h = (deg + 90.0) / 360.0; h -= std::floor(h); return h; }

    // vértices do triângulo: matiz, branco e preto
    void triangle(QPointF &ph, QPointF &pw, QPointF &pb) const
    {
        const QPointF c = center();
        const qreal a = qDegreesToRadians(hueToAngle(m_hue));
        ph = c + QPointF(std::cos(a), std::sin(a)) * R_TRI;
        pw = c + QPointF(std::cos(a + 2 * M_PI / 3), std::sin(a + 2 * M_PI / 3)) * R_TRI;
        pb = c + QPointF(std::cos(a - 2 * M_PI / 3), std::sin(a - 2 * M_PI / 3)) * R_TRI;
    }

    static void bary(const QPointF &p, const QPointF &a, const QPointF &b, const QPointF &c, qreal &u, qreal &v, qreal &w)
    {
        const QPointF v0 = b - a, v1 = c - a, v2 = p - a;
        const qreal d00 = QPointF::dotProduct(v0, v0), d01 = QPointF::dotProduct(v0, v1), d11 = QPointF::dotProduct(v1, v1);
        const qreal d20 = QPointF::dotProduct(v2, v0), d21 = QPointF::dotProduct(v2, v1);
        const qreal den = d00 * d11 - d01 * d01;
        v = (d11 * d20 - d01 * d21) / den;
        w = (d00 * d21 - d01 * d20) / den;
        u = 1.0 - v - w;
    }

    QPointF svPoint() const
    {
        QPointF ph, pw, pb;
        triangle(ph, pw, pb);
        const qreal a = m_sat * m_val, b = m_val - a, c = 1.0 - m_val;
        return ph * a + pw * b + pb * c;
    }

    void pickSV(const QPointF &p)
    {
        QPointF ph, pw, pb;
        triangle(ph, pw, pb);
        qreal a, b, c;
        bary(p, ph, pw, pb, a, b, c);
        a = qMax(0.0, a); b = qMax(0.0, b); c = qMax(0.0, c);
        const qreal sum = a + b + c;
        if (sum <= 0) return;
        a /= sum; b /= sum; c /= sum;
        m_val = qBound(0.0, a + b, 1.0);
        m_sat = m_val > 0.0001 ? qBound(0.0, a / m_val, 1.0) : 0.0;
        emitColor();
    }

    void emitColor()
    {
        // durante o arrasto, só o triângulo e a etiqueta do topo mudam; o anel
        // de pincéis e a sombra não precisam ser repintados a cada frame
        update(colorRect());
        if (onColor) onColor(color());
    }

    // área que muda quando a cor muda: anel de matiz com o marcador dele,
    // triângulo com o marcador de S/B e a etiqueta do topo
    QRect colorRect() const
    {
        const QPointF c = center();
        const int r = R_HUE_OUT + 16;   // alcança o marcador do anel
        return QRect(qRound(c.x() - r), qRound(c.y() - r), 2 * r, 2 * r)
                .united(QRect(0, 0, width(), LABEL_H));
    }

    QImage renderTriangle() const
    {
        // acima de 2× o olho não vê diferença num gradiente; e o custo do
        // laço cresce com o quadrado do DPR
        const qreal dpr = qMin(devicePixelRatioF(), 2.0);
        const QPointF c = center();
        const QRectF box(c.x() - R_TRI, c.y() - R_TRI, 2 * R_TRI, 2 * R_TRI);
        QImage img(QSize(qCeil(box.width() * dpr), qCeil(box.height() * dpr)), QImage::Format_ARGB32_Premultiplied);
        img.fill(Qt::transparent);
        QPointF ph, pw, pb;
        triangle(ph, pw, pb);
        const QColor hc = hueColor(m_hue);
        for (int y = 0; y < img.height(); y++) {
            QRgb *line = reinterpret_cast<QRgb*>(img.scanLine(y));
            for (int x = 0; x < img.width(); x++) {
                const QPointF p(box.left() + (x + 0.5) / dpr, box.top() + (y + 0.5) / dpr);
                qreal a, b, cc;
                bary(p, ph, pw, pb, a, b, cc);
                if (a < -0.01 || b < -0.01 || cc < -0.01) continue;
                a = qMax(0.0, a); b = qMax(0.0, b);
                const int r = qBound(0, qRound((a * hc.redF() + b) * 255), 255);
                const int g = qBound(0, qRound((a * hc.greenF() + b) * 255), 255);
                const int bl = qBound(0, qRound((a * hc.blueF() + b) * 255), 255);
                line[x] = qRgb(r, g, bl);
            }
        }
        img.setDevicePixelRatio(dpr);
        return img;
    }

    void paintEvent(QPaintEvent *) override
    {
        QPainter g(this);
        g.setRenderHint(QPainter::Antialiasing);
        g.setRenderHint(QPainter::SmoothPixmapTransform);
        const QPointF c = center();

        // sombra leve e disco
        for (int i = 6; i > 0; i--) {
            g.setPen(Qt::NoPen);
            g.setBrush(QColor(0, 0, 0, 10));
            g.drawEllipse(c + QPointF(0, 3), R_OUT + i, R_OUT + i);
        }
        g.setBrush(QColor(0x2e, 0x2d, 0x29, 246));
        g.setPen(QPen(QColor(0x52, 0x4f, 0x48), 1));
        g.drawEllipse(c, R_OUT, R_OUT);

        // pincéis no anel de fora
        const qreal step = 360.0 / SECTORS;
        for (int i = 0; i < SECTORS; i++) {
            const qreal mid = -90.0 + i * step;
            const bool has = i < m_brushes.size() && m_brushes[i].kind != Empty;
            const bool active = has && i == m_active;
            QPainterPath sector = ringSector(c, R_LAYER_IN + 1, R_OUT - 1, mid - step / 2 + 0.6, mid + step / 2 - 0.6);
            g.setPen(Qt::NoPen);
            if (active) {
                QLinearGradient grad(c - QPointF(R_OUT, R_OUT), c + QPointF(R_OUT, R_OUT));
                grad.setColorAt(0, QColor(0x3e, 0x8e, 0x7e));
                grad.setColorAt(1, QColor(0x6f, 0xbf, 0xad));
                g.setBrush(grad);
            } else {
                g.setBrush(QColor(0x34, 0x33, 0x2e));
            }
            g.drawPath(sector);
            if (i == m_pressedSector) {
                g.setBrush(QColor(255, 255, 255, 22));
                g.drawPath(sector);
            }
            if (!has) continue;
            const Brush &br = m_brushes[i];
            const qreal a = qDegreesToRadians(mid);
            const QPointF t = c + QPointF(std::cos(a), std::sin(a)) * ((R_LAYER_IN + R_OUT) / 2.0 + (br.kind == Eraser ? 0 : 6));
            const QColor ink = active ? QColor(0x13, 0x23, 0x20) : QColor(0xee, 0xea, 0xe0);
            if (br.kind == Eraser) {
                // borracha desenhada
                g.save();
                g.translate(t);
                g.rotate(-35);
                g.setPen(QPen(ink, 2.4, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
                g.setBrush(Qt::NoBrush);
                g.drawRoundedRect(QRectF(-16, -10, 32, 20), 5, 5);
                g.drawLine(QPointF(-3, -10), QPointF(-3, 10));
                g.restore();
            } else {
                const qreal side = 40;
                const QRectF box(t.x() - side / 2, t.y() - side / 2, side, side);
                if (!br.thumb.isNull()) {
                    QPainterPath clip;
                    clip.addRoundedRect(box, 9, 9);
                    g.save();
                    g.setClipPath(clip);
                    g.drawImage(box, br.thumb);
                    g.restore();
                    g.setPen(QPen(active ? QColor(0x13, 0x23, 0x20) : QColor(0x52, 0x4f, 0x48), 1.4));
                    g.setBrush(Qt::NoBrush);
                    g.drawRoundedRect(box, 9, 9);
                } else {
                    QPainterPath wave;
                    wave.moveTo(t + QPointF(-18, 8));
                    wave.cubicTo(t + QPointF(-8, -12), t + QPointF(8, 12), t + QPointF(18, -8));
                    g.setPen(QPen(ink, 4, Qt::SolidLine, Qt::RoundCap));
                    g.setBrush(Qt::NoBrush);
                    g.drawPath(wave);
                }
                // selo: estrela (favorito) ou relógio (recente), do lado de dentro
                const QPointF badge = c + QPointF(std::cos(a), std::sin(a)) * (R_LAYER_IN + 10);
                g.setPen(Qt::NoPen);
                g.setBrush(active ? QColor(0x13, 0x23, 0x20) : QColor(0x2e, 0x2d, 0x29));
                g.drawEllipse(badge, 8, 8);
                if (br.kind == Favorite) {
                    QPolygonF star;
                    for (int k = 0; k < 10; ++k) {
                        const qreal sa = -M_PI / 2 + k * M_PI / 5;
                        const qreal rad = (k % 2 == 0) ? 5.5 : 2.4;
                        star << badge + QPointF(std::cos(sa) * rad, std::sin(sa) * rad);
                    }
                    g.setBrush(QColor(0x6f, 0xbf, 0xad));
                    g.drawPolygon(star);
                } else {
                    g.setPen(QPen(QColor(0xa8, 0xa3, 0x98), 1.3, Qt::SolidLine, Qt::RoundCap));
                    g.setBrush(Qt::NoBrush);
                    g.drawEllipse(badge, 5, 5);
                    g.drawLine(badge, badge + QPointF(0, -3));
                    g.drawLine(badge, badge + QPointF(2.2, 1.3));
                }
            }
        }
        g.setPen(Qt::NoPen);
        g.setBrush(QColor(0x2e, 0x2d, 0x29));
        g.drawEllipse(c, R_LAYER_IN - 1, R_LAYER_IN - 1);

        // anel de matiz
        QConicalGradient hue(c, 0);
        // QConicalGradient gira no sentido anti-horário a partir das 3 h
        for (int i = 0; i <= 12; i++) {
            const qreal t = i / 12.0;                 // fração no sentido anti-horário
            const qreal screenDeg = -t * 360.0;       // em graus da tela (horário)
            hue.setColorAt(t, hueColor(angleToHue(screenDeg)));
        }
        QPainterPath ring;
        ring.addEllipse(c, R_HUE_OUT, R_HUE_OUT);
        ring.addEllipse(c, R_HUE_IN, R_HUE_IN);
        g.setPen(Qt::NoPen);
        g.setBrush(hue);
        g.drawPath(ring);

        // triângulo
        if (m_triangle.isNull()) m_triangle = renderTriangle();
        g.drawImage(QPointF(c.x() - R_TRI, c.y() - R_TRI), m_triangle);

        // marcadores
        const qreal ha = qDegreesToRadians(hueToAngle(m_hue));
        const QPointF hm = c + QPointF(std::cos(ha), std::sin(ha)) * ((R_HUE_IN + R_HUE_OUT) / 2.0);
        marker(g, hm, hueColor(m_hue), 12);
        marker(g, svPoint(), color(), 10);

        // etiqueta acima: pincel ativo (ou o que está sob o dedo) e o tamanho
        QFont lf = font();
        lf.setPixelSize(15);
        lf.setWeight(QFont::Bold);
        g.setFont(lf);
        const int shown = m_pressedSector >= 0 ? m_pressedSector : m_active;
        if (shown >= 0 && shown < m_brushes.size() && m_brushes[shown].kind != Empty) {
            const Brush &br = m_brushes[shown];
            const QString text = br.size.isEmpty() ? br.name : br.name + QStringLiteral(" · ") + br.size;
            const QFontMetrics fm(lf);
            const qreal w = qMin<qreal>(width() - 20, fm.horizontalAdvance(text) + 40);
            const QRectF r(width() / 2.0 - w / 2, 6, w, 40);
            g.setPen(QPen(QColor(0x2f, 0x75, 0x68), 1));
            g.setBrush(QColor(0x3a, 0x39, 0x34, 245));
            g.drawRoundedRect(r, 20, 20);
            g.setPen(QColor(0xee, 0xea, 0xe0));
            g.drawText(r, Qt::AlignCenter, fm.elidedText(text, Qt::ElideRight, int(w - 28)));
        }
        // abaixo: camada ativa (toque abre as camadas)
        const QRectF lr = layerPill();
        if (!m_layer.isEmpty()) {
            QFont pf = font();
            pf.setPixelSize(14);
            pf.setWeight(QFont::DemiBold);
            g.setFont(pf);
            g.setPen(QPen(QColor(0x5a, 0x57, 0x4f), 1));
            g.setBrush(m_layerPressed ? QColor(0x4a, 0x48, 0x42) : QColor(0x3a, 0x39, 0x34, 245));
            g.drawRoundedRect(lr, lr.height() / 2, lr.height() / 2);
            g.setPen(QColor(0xee, 0xea, 0xe0));
            const QString text = QStringLiteral("Camada: ") + m_layer;
            g.drawText(lr.adjusted(18, 0, -34, 0), Qt::AlignVCenter | Qt::AlignLeft,
                       QFontMetrics(pf).elidedText(text, Qt::ElideRight, int(lr.width() - 52)));
            // seta
            const QPointF ch(lr.right() - 22, lr.center().y());
            g.setPen(QPen(QColor(0xa8, 0xa3, 0x98), 1.8, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
            g.drawPolyline(QPolygonF() << ch + QPointF(-5, -3) << ch + QPointF(0, 2) << ch + QPointF(5, -3));
        }
    }

    QRectF layerPill() const
    {
        const QPointF c = center();
        return QRectF(c.x() - 120, c.y() + R_OUT + 10, 240, 44);
    }

    static QPainterPath ringSector(const QPointF &c, qreal r1, qreal r2, qreal a1, qreal a2)
    {
        QPainterPath p;
        const QRectF outer(c.x() - r2, c.y() - r2, 2 * r2, 2 * r2);
        const QRectF inner(c.x() - r1, c.y() - r1, 2 * r1, 2 * r1);
        // arcTo usa graus no sentido anti-horário; a1/a2 vêm em graus da tela (horário)
        p.arcMoveTo(outer, -a1);
        p.arcTo(outer, -a1, -(a2 - a1));
        p.arcTo(inner, -a2, a2 - a1);
        p.closeSubpath();
        return p;
    }

    static void marker(QPainter &g, const QPointF &p, const QColor &fill, qreal r)
    {
        g.setPen(QPen(QColor(0x24, 0x23, 0x1f, 160), 4));
        g.setBrush(Qt::NoBrush);
        g.drawEllipse(p, r, r);
        g.setPen(QPen(Qt::white, 2.5));
        g.setBrush(fill);
        g.drawEllipse(p, r, r);
    }

    enum Zone { None, Layers, Hue, Tri, Outside, LayerPill };

    Zone zoneAt(const QPointF &p) const
    {
        const qreal r = QLineF(center(), p).length();
        if (layerPill().contains(p) && !m_layer.isEmpty()) return LayerPill;
        if (r > R_OUT) return Outside;
        if (r >= R_LAYER_IN) return Layers;
        if (r >= R_HUE_IN - 4) return Hue;
        if (r <= R_TRI + 6) return Tri;
        return None;
    }

    int sectorAt(const QPointF &p) const
    {
        qreal deg = angleOf(p - center());
        // setor i ocupa [-90 + 45i - 22,5, -90 + 45i + 22,5)
        const qreal step = 360.0 / SECTORS;
        qreal rel = deg - (-90.0 - step / 2);
        rel -= std::floor(rel / 360.0) * 360.0;
        return qBound(0, int(rel / step), SECTORS - 1);
    }

    void mousePressEvent(QMouseEvent *e) override
    {
        m_zone = zoneAt(e->localPos());
        if (m_zone == Outside) {
            // fora da roda (ou nas etiquetas): fecha
            if (onClose) onClose();
            return;
        }
        if (m_zone == LayerPill) {
            m_layerPressed = true;
            update();
        } else if (m_zone == Layers) {
            m_pressedSector = sectorAt(e->localPos());
            update();
        } else {
            drag(e->localPos());
        }
    }

    void mouseMoveEvent(QMouseEvent *e) override
    {
        if (m_zone == Hue || m_zone == Tri) {
            drag(e->localPos());
        } else if (m_zone == Layers) {
            // arrastar pelo anel mostra o pincel sob o dedo
            const int s = zoneAt(e->localPos()) == Layers ? sectorAt(e->localPos()) : -1;
            if (s != m_pressedSector) {
                m_pressedSector = s;
                update();
            }
        }
    }

    void mouseReleaseEvent(QMouseEvent *e) override
    {
        if (m_zone == Layers) {
            const int s = zoneAt(e->localPos()) == Layers ? sectorAt(e->localPos()) : -1;
            if (s >= 0 && s < m_brushes.size() && m_brushes[s].kind != Empty) {
                m_active = s;
                if (onBrush) onBrush(s);
            }
        } else if (m_zone == LayerPill && m_layerPressed) {
            m_layerPressed = false;
            if (layerPill().contains(e->localPos()) && onLayers) onLayers();
        }
        m_layerPressed = false;
        m_pressedSector = -1;
        m_zone = None;
        if (m_triangleDirty) {   // fim do arrasto: o último frame sai em crisp
            m_triangle = QImage();
            m_triangleDirty = false;
        }
        update();
    }

    void keyPressEvent(QKeyEvent *e) override
    {
        if (e->key() == Qt::Key_Escape || e->key() == Qt::Key_Back) {
            if (onClose) onClose();
            e->accept();
            return;
        }
        QWidget::keyPressEvent(e);
    }

    void drag(const QPointF &p)
    {
        if (m_zone == Hue) {
            m_hue = angleToHue(angleOf(p - center()));
            // renderizar o triângulo de novo custa um laço por pixel: em vez de
            // fazê-lo a cada move do arrasto (120 Hz), agrupa e faz a ~33 Hz.
            // A cor de verdade (etiqueta, swatch, pincel) continua em tempo real.
            m_triangleDirty = true;
            scheduleTriangleRender();
            emitColor();
        } else if (m_zone == Tri) {
            pickSV(p);
        }
    }

    void scheduleTriangleRender()
    {
        if (m_renderScheduled) return;
        m_renderScheduled = true;
        QPointer<NanquimWheel> guard(this);
        QTimer::singleShot(30, this, [guard]() {
            if (!guard) return;
            guard->m_renderScheduled = false;
            if (guard->m_triangleDirty) {
                guard->m_triangle = QImage();
                guard->m_triangleDirty = false;
                guard->update(guard->colorRect());
            }
        });
    }

private:
    QVector<Brush> m_brushes;
    QString m_layer;
    bool m_layerPressed = false;
    int m_active = -1;
    int m_pressedSector = -1;
    qreal m_hue = 0.6;
    qreal m_sat = 0.8;
    qreal m_val = 0.9;
    Zone m_zone = None;
    mutable QImage m_triangle;
    bool m_triangleDirty = false;
    bool m_renderScheduled = false;
};

#endif
