#include "HistoryStore.h"
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QStandardPaths>
#include <QDir>
#include <QFileInfo>
#include <QUuid>
#include <algorithm>

class HistoryWorker final : public QObject {
    Q_OBJECT
public:
    QString path;
    QSqlDatabase database;
    QString connection;
    void close() {
        database.close(); database = {};
        QSqlDatabase::removeDatabase(connection);
    }
public slots:
    void initialize() {
        QDir().mkpath(QFileInfo(path).absolutePath());
        connection = "history-" + QUuid::createUuid().toString(QUuid::WithoutBraces);
        database = QSqlDatabase::addDatabase("QSQLITE", connection);
        database.setDatabaseName(path);
        database.setConnectOptions("QSQLITE_BUSY_TIMEOUT=1000");
        if (!database.open()) { emit failed(database.lastError().text()); return; }
        QSqlQuery q(database);
        if (!q.exec("CREATE TABLE IF NOT EXISTS playback (path TEXT PRIMARY KEY, title TEXT, position REAL, duration REAL, size INTEGER, modified INTEGER, played INTEGER)")) {
            emit failed(q.lastError().text()); return;
        }
        q.exec("PRAGMA journal_mode=WAL");
        if (!q.exec("SELECT path,title,position,duration,size,modified,played FROM playback")) {
            emit failed(q.lastError().text()); return;
        }
        QVariantList records;
        while (q.next()) {
            QVariantMap r;
            const QStringList names{"path", "title", "position", "duration", "size", "modified", "played"};
            for (int i=0; i<names.size(); ++i) r.insert(names[i],q.value(i));
            records.append(r);
        }
        emit initialized(records);
    }
    void write(QVariantMap record) {
        if (!database.isOpen()) return;
        QSqlQuery q(database);
        q.prepare("INSERT OR REPLACE INTO playback (path,title,position,duration,size,modified,played) VALUES (?,?,?,?,?,?,?)");
        for (const auto &key : {"path", "title", "position", "duration", "size", "modified", "played"}) q.addBindValue(record.value(key));
        if (!q.exec()) emit failed(q.lastError().text());
    }
    void clear() {
        QSqlQuery q(database);
        if (!q.exec("DELETE FROM playback")) emit failed(q.lastError().text());
    }
signals:
    void initialized(QVariantList records);
    void failed(QString error);
};

HistoryStore::HistoryStore(const QString &databasePath, QObject *parent) : QObject(parent), m_worker(new HistoryWorker) {
    m_worker->path = databasePath.isEmpty()
        ? QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation) + "/playback.sqlite" : databasePath;
    m_worker->moveToThread(&m_thread);
    connect(&m_thread, &QThread::started, m_worker, &HistoryWorker::initialize);
    connect(&m_thread, &QThread::finished, m_worker, &QObject::deleteLater);
    connect(this, &HistoryStore::writeRequested, m_worker, &HistoryWorker::write);
    connect(this, &HistoryStore::clearRequested, m_worker, &HistoryWorker::clear);
    connect(m_worker, &HistoryWorker::initialized, this, [this](const QVariantList &rows) {
        // Saves queued before initialization must win over older rows loaded from disk.
        for (const auto &row : m_skipInitialRows ? QVariantList{} : rows) {
            const auto r = row.toMap();
            if (!m_records.contains(r.value("path").toString())) m_records.insert(r.value("path").toString(),r);
        }
        m_ready = true; emit changed();
    });
    connect(m_worker, &HistoryWorker::failed, this, [this](const QString &message) {
        m_error = "Playback history: " + message; emit changed();
    });
    m_thread.start();
}
HistoryStore::~HistoryStore() {
    // Flush queued writes on the SQLite-owning thread before its event loop exits.
    QMetaObject::invokeMethod(m_worker, [worker=m_worker] { worker->close(); }, Qt::BlockingQueuedConnection);
    m_thread.quit(); m_thread.wait();
}
QVariantList HistoryStore::recent() const {
    QVariantList list;
    for (auto it=m_records.cbegin(); it!=m_records.cend(); ++it) list.append(it.value());
    std::sort(list.begin(),list.end(),[](const QVariant &a,const QVariant &b) {
        return a.toMap().value("played").toLongLong() > b.toMap().value("played").toLongLong();
    });
    return list;
}
double HistoryStore::resumePosition(const QString &path, qint64 size, qint64 modified) const {
    const auto r=m_records.value(path);
    if (r.value("size").toLongLong()!=size || r.value("modified").toLongLong()!=modified) return 0;
    return r.value("position").toDouble();
}
void HistoryStore::save(const QVariantMap &record) {
    if (record.value("path").toString().isEmpty()) return;
    const auto key=record.value("path").toString();
    m_records.insert(key,record);
    emit writeRequested(record);
    // Notify the recent view on file changes, not on every time-position event.
    emit changed();
}
void HistoryStore::clear() { m_skipInitialRows=true; m_records.clear(); emit clearRequested(); emit changed(); }

#include "HistoryStore.moc"
