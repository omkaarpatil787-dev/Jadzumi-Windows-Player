#pragma once
#include <QWindow>

// mpv owns the D3D11 swapchain inside this native window; Qt owns the HWND lifetime.
class VideoWindow final : public QWindow {
    Q_OBJECT
public:
    VideoWindow();
signals:
    void toggleFullscreenRequested();
protected:
    bool event(QEvent *event) override;
};
