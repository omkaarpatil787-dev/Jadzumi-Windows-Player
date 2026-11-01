#include "Preferences.h"
#include <QKeySequence>
#include <QSet>
#include <cmath>
#ifdef Q_OS_WIN
#include <windows.h>
#endif

Preferences::Preferences(QObject *parent) : QObject(parent) { refreshSystemMotion(); }
QString Preferences::theme() const { return m_settings.value("appearance/theme", "system").toString(); }
bool Preferences::animations() const { return m_settings.value("appearance/animations", true).toBool(); }
bool Preferences::reducedMotion() const { return m_settings.value("appearance/reducedMotion", false).toBool(); }
bool Preferences::effectiveReducedMotion() const { return !animations() || reducedMotion() || !m_systemAnimations; }
double Preferences::uiScale() const { return m_settings.value("appearance/uiScale", 1.0).toDouble(); }
bool Preferences::expandedSidebar() const { return m_settings.value("appearance/sidebar", true).toBool(); }
bool Preferences::resumeEnabled() const { return m_settings.value("playback/resume", true).toBool(); }
bool Preferences::autoplay() const { return m_settings.value("playback/autoplay", true).toBool(); }
void Preferences::write(const QString &key, const QVariant &value) {
    if (m_settings.value(key) == value) return;
    m_settings.setValue(key, value);
    emit changed();
}
void Preferences::setTheme(const QString &value) {
    if (QSet<QString>{"light", "dark", "amoled", "system"}.contains(value)) write("appearance/theme", value);
}
void Preferences::setAnimations(bool value) { write("appearance/animations", value); }
void Preferences::setReducedMotion(bool value) { write("appearance/reducedMotion", value); }
void Preferences::setUiScale(double value) { if (std::isfinite(value)) write("appearance/uiScale", qBound(0.85, value, 1.25)); }
void Preferences::setExpandedSidebar(bool value) { write("appearance/sidebar", value); }
void Preferences::setResumeEnabled(bool value) { write("playback/resume", value); }
void Preferences::setAutoplay(bool value) { write("playback/autoplay", value); }
QVariantMap Preferences::shortcuts() const {
    QVariantMap defaults{{"open", "Ctrl+O"}, {"playPause", "Space"}, {"seekBack", "Left"},
        {"seekForward", "Right"}, {"volumeUp", "Up"}, {"volumeDown", "Down"},
        {"fullscreen", "F"}, {"mute", "M"}, {"subtitles", "S"}, {"frameStep", "."},
        {"next", "Ctrl+Right"}, {"previous", "Ctrl+Left"}, {"stop", "Ctrl+S"}};
    for (auto it = defaults.begin(); it != defaults.end(); ++it)
        it.value() = m_settings.value("shortcuts/" + it.key(), it.value());
    return defaults;
}
bool Preferences::setShortcut(const QString &action, const QString &sequence) {
    const auto all = shortcuts();
    const auto normalized = QKeySequence::fromString(sequence, QKeySequence::PortableText).toString(QKeySequence::PortableText);
    if (!all.contains(action) || normalized.isEmpty() || normalized.contains(',') ||
        normalized == "Esc" || normalized == "Alt+F4" || normalized == "F11") {
        m_shortcutError = "Choose one valid key combination. Esc, F11 and Alt+F4 are reserved.";
        emit changed(); return false;
    }
    for (auto it = all.begin(); it != all.end(); ++it) {
        if (it.key() != action && it.value().toString() == normalized) {
            m_shortcutError = "That shortcut is already used by " + it.key() + ".";
            emit changed(); return false;
        }
    }
    m_shortcutError.clear(); write("shortcuts/" + action, normalized); emit changed(); return true;
}
void Preferences::resetShortcuts() { m_settings.remove("shortcuts"); m_shortcutError.clear(); emit changed(); }
void Preferences::refreshSystemMotion() {
#ifdef Q_OS_WIN
    BOOL enabled = TRUE;
    if (SystemParametersInfoW(SPI_GETCLIENTAREAANIMATION, 0, &enabled, 0)) m_systemAnimations = enabled;
#endif
    emit changed();
}
