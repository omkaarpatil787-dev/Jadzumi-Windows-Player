#pragma once
#include <QLibrary>
#include <QString>
#include <mpv/client.h>

// Resolve the official C ABI from an explicit DLL path; no import-library ABI dependency.
class MpvApi final {
public:
    bool load(const QString &absolutePath, QString &error);
#define MPV_FUNCTIONS(X) \
    X(create) X(initialize) X(terminate_destroy) X(client_api_version) \
    X(set_option_string) X(set_option) X(observe_property) X(wait_event) \
    X(set_wakeup_callback) X(command_async) X(set_property_async) X(error_string) \
    X(get_property_string) X(free)
#define DECLARE(name) decltype(&mpv_##name) name = nullptr;
    MPV_FUNCTIONS(DECLARE)
#undef DECLARE
private:
    QLibrary m_library;
};
