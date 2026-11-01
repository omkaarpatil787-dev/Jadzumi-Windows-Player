#pragma once
#include <QObject>
#include <QColor>
#include <QVariantAnimation>
#include <QVector>
#include "Preferences.h"

class ThemeController final : public QObject {
    Q_OBJECT
    Q_PROPERTY(bool dark READ dark NOTIFY metricsChanged)
    Q_PROPERTY(bool black READ black NOTIFY metricsChanged)
    Q_PROPERTY(double scaleFactor READ scaleFactor NOTIFY metricsChanged)
    Q_PROPERTY(int motion READ motion NOTIFY metricsChanged)
    Q_PROPERTY(QString fontFamily READ fontFamily CONSTANT)
    Q_PROPERTY(QColor canvas READ canvas NOTIFY colorsChanged)
    Q_PROPERTY(QColor surface READ surface NOTIFY colorsChanged)
    Q_PROPERTY(QColor raised READ raised NOTIFY colorsChanged)
    Q_PROPERTY(QColor text READ text NOTIFY colorsChanged)
    Q_PROPERTY(QColor muted READ muted NOTIFY colorsChanged)
    Q_PROPERTY(QColor outline READ outline NOTIFY colorsChanged)
    Q_PROPERTY(QColor accent READ accent NOTIFY colorsChanged)
    Q_PROPERTY(QColor accentInk READ accentInk NOTIFY colorsChanged)
    Q_PROPERTY(QColor danger READ danger NOTIFY colorsChanged)
    Q_PROPERTY(QColor hero READ hero NOTIFY colorsChanged)
public:
    explicit ThemeController(Preferences &preferences,QObject *parent=nullptr);
    bool dark()const{return m_dark;}
    bool black()const{return m_black;}
    double scaleFactor()const{return m_preferences.uiScale();}
    int motion()const{return m_preferences.effectiveReducedMotion()?0:200;}
    QString fontFamily()const;
    QColor canvas()const{return m_colors.value(0);}
    QColor surface()const{return m_colors.value(1);}
    QColor raised()const{return m_colors.value(2);}
    QColor text()const{return m_colors.value(3);}
    QColor muted()const{return m_colors.value(4);}
    QColor outline()const{return m_colors.value(5);}
    QColor accent()const{return m_colors.value(6);}
    QColor accentInk()const{return m_colors.value(7);}
    QColor danger()const{return m_colors.value(8);}
    QColor hero()const{return m_colors.value(9);}
signals:
    void colorsChanged();
    void metricsChanged();
private:
    bool eventFilter(QObject *object,QEvent *event)override;
    void refresh();
    Preferences &m_preferences;
    QVariantAnimation m_animation;
    QVector<QColor> m_colors,m_from,m_target;
    bool m_dark=false,m_black=false;
};
