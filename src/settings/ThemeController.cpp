#include "ThemeController.h"
#include <QGuiApplication>
#include <QPalette>
#include <QEvent>

ThemeController::ThemeController(Preferences &preferences,QObject *parent):QObject(parent),m_preferences(preferences){
    m_animation.setStartValue(0.0);m_animation.setEndValue(1.0);m_animation.setEasingCurve(QEasingCurve::OutCubic);
    connect(&m_animation,&QVariantAnimation::valueChanged,this,[this](const QVariant &v){
        const double p=v.toDouble();
        for(int i=0;i<m_colors.size();++i){
            const auto a=m_from[i],b=m_target[i];
            m_colors[i]=QColor::fromRgbF(a.redF()+(b.redF()-a.redF())*p,a.greenF()+(b.greenF()-a.greenF())*p,a.blueF()+(b.blueF()-a.blueF())*p);
        }
        emit colorsChanged();
    });
    connect(&m_preferences,&Preferences::changed,this,[this]{refresh();});
    qGuiApp->installEventFilter(this);
    refresh();
}
bool ThemeController::eventFilter(QObject *object,QEvent *event){
    if(object==qGuiApp && event->type()==QEvent::ApplicationPaletteChange)refresh();
    return QObject::eventFilter(object,event);
}
QString ThemeController::fontFamily()const{
#ifdef Q_OS_WIN
    return "Segoe UI Variable";
#else
    return "Sans Serif";
#endif
}
void ThemeController::refresh(){
    const auto mode=m_preferences.theme();
    m_black=mode=="amoled";
    m_dark=m_black || mode=="dark" || (mode=="system" && QGuiApplication::palette().color(QPalette::Window).lightnessF()<0.5);
    const QVector<QColor> target=m_dark?QVector<QColor>{m_black?"#000000":"#101722",m_black?"#0b0d12":"#192332","#233247","#eaf1fc","#a8b8ce","#344055","#a4cdff","#102f53","#ffb5c0","#192d46"}
        :QVector<QColor>{"#f2f6fc","#ffffff","#e8f0fc","#16263c","#5d7089","#d6e0ee","#235fa7","#ffffff","#a52e44","#deebfd"};
    emit metricsChanged();
    if(m_colors.isEmpty() || motion()==0){m_animation.stop();m_colors=m_target=target;emit colorsChanged();return;}
    if(target==m_target)return;
    m_animation.stop();m_from=m_colors;m_target=target;m_animation.setDuration(motion());m_animation.start();
}
