#pragma once
#include <QObject>
#include <QSettings>
#include <QVariantMap>

class Preferences final : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString theme READ theme WRITE setTheme NOTIFY changed)
    Q_PROPERTY(bool animations READ animations WRITE setAnimations NOTIFY changed)
    Q_PROPERTY(bool reducedMotion READ reducedMotion WRITE setReducedMotion NOTIFY changed)
    Q_PROPERTY(bool effectiveReducedMotion READ effectiveReducedMotion NOTIFY changed)
    Q_PROPERTY(double uiScale READ uiScale WRITE setUiScale NOTIFY changed)
    Q_PROPERTY(bool expandedSidebar READ expandedSidebar WRITE setExpandedSidebar NOTIFY changed)
    Q_PROPERTY(bool resumeEnabled READ resumeEnabled WRITE setResumeEnabled NOTIFY changed)
    Q_PROPERTY(bool autoplay READ autoplay WRITE setAutoplay NOTIFY changed)
    Q_PROPERTY(QVariantMap shortcuts READ shortcuts NOTIFY changed)
    Q_PROPERTY(QString shortcutError READ shortcutError NOTIFY changed)
public:
    explicit Preferences(QObject *parent = nullptr);
    QString theme() const;
    bool animations() const;
    bool reducedMotion() const;
    bool effectiveReducedMotion() const;
    double uiScale() const;
    bool expandedSidebar() const;
    bool resumeEnabled() const;
    bool autoplay() const;
    QVariantMap shortcuts() const;
    QString shortcutError() const { return m_shortcutError; }
    void setTheme(const QString &value);
    void setAnimations(bool value);
    void setReducedMotion(bool value);
    void setUiScale(double value);
    void setExpandedSidebar(bool value);
    void setResumeEnabled(bool value);
    void setAutoplay(bool value);
    Q_INVOKABLE bool setShortcut(const QString &action, const QString &sequence);
    Q_INVOKABLE void resetShortcuts();
    Q_INVOKABLE void refreshSystemMotion();
signals:
    void changed();
private:
    void write(const QString &key, const QVariant &value);
    QSettings m_settings;
    bool m_systemAnimations = true;
    QString m_shortcutError;
};
