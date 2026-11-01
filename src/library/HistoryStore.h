#pragma once
#include <QObject>
#include <QThread>
#include <QVariantList>
#include <QVariantMap>
#include <QHash>

class HistoryWorker;
class HistoryStore final : public QObject {
    Q_OBJECT
    Q_PROPERTY(QVariantList recent READ recent NOTIFY changed)
    Q_PROPERTY(bool ready READ ready NOTIFY changed)
    Q_PROPERTY(QString error READ error NOTIFY changed)
public:
    explicit HistoryStore(const QString &databasePath = {}, QObject *parent = nullptr);
    ~HistoryStore() override;
    QVariantList recent() const;
    bool ready() const { return m_ready; }
    QString error() const { return m_error; }
    double resumePosition(const QString &path, qint64 size, qint64 modified) const;
    void save(const QVariantMap &record);
    Q_INVOKABLE void clear();
signals:
    void changed();
    void writeRequested(QVariantMap record);
    void clearRequested();
private:
    QThread m_thread;
    HistoryWorker *m_worker = nullptr;
    QHash<QString, QVariantMap> m_records;
    bool m_ready = false;
    bool m_skipInitialRows = false;
    QString m_error;
};
