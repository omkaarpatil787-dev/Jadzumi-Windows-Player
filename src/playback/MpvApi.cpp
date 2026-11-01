#include "MpvApi.h"
#include <QFileInfo>

bool MpvApi::load(const QString &absolutePath, QString &error) {
    if (!QFileInfo(absolutePath).isAbsolute() || !QFileInfo::exists(absolutePath)) {
        error = QStringLiteral("libmpv must be an existing absolute library path.");
        return false;
    }
    m_library.setFileName(absolutePath);
    if (!m_library.load()) { error = m_library.errorString(); return false; }
#define RESOLVE(name) \
    name = reinterpret_cast<decltype(name)>(m_library.resolve("mpv_" #name)); \
    if (!name) { error = QStringLiteral("Missing libmpv symbol: mpv_" #name); return false; }
    MPV_FUNCTIONS(RESOLVE)
#undef RESOLVE
    // This client uses functions available since client API 2.0.
    const auto version = client_api_version();
    if ((version >> 16) != 2) {
        error = QStringLiteral("Incompatible libmpv client API major (requires 2).");
        return false;
    }
    return true;
}
