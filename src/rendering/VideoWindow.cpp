#include "VideoWindow.h"
#include <QEvent>

VideoWindow::VideoWindow() {
    setSurfaceType(QSurface::RasterSurface);
    setFlags(Qt::FramelessWindowHint);
    setTitle(QStringLiteral("Jadzumi video surface"));
}
bool VideoWindow::event(QEvent *event) {
    if (event->type() == QEvent::MouseButtonDblClick) {
        emit toggleFullscreenRequested();
        return true;
    }
    return QWindow::event(event);
}
