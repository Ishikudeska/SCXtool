#pragma once

#include "P4kTreeModel.h"

#include <QObject>
#include <QUrl>
#include <QVariantList>
#include <QtQml/qqmlregistration.h>

#include <atomic>
#include <memory>

class AppController;

class ExplorerController : public QObject
{
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(P4kTreeModel *tree READ tree CONSTANT)
    Q_PROPERTY(QString archivePath READ archivePath NOTIFY loadedChanged)
    Q_PROPERTY(bool loaded READ loaded NOTIFY loadedChanged)
    Q_PROPERTY(bool forgeLoaded READ forgeLoaded NOTIFY loadedChanged)
    Q_PROPERTY(QString summary READ summary NOTIFY loadedChanged)
    Q_PROPERTY(int recordsNode READ recordsNode NOTIFY loadedChanged)
    Q_PROPERTY(QVariantList results READ results NOTIFY resultsChanged)
    Q_PROPERTY(QString resultsNote READ resultsNote NOTIFY resultsChanged)
    Q_PROPERTY(bool searching READ searching NOTIFY resultsChanged)
    Q_PROPERTY(int currentNode READ currentNode NOTIFY previewChanged)
    Q_PROPERTY(QString previewTitle READ previewTitle NOTIFY previewChanged)
    Q_PROPERTY(QString previewInfo READ previewInfo NOTIFY previewChanged)
    Q_PROPERTY(QString previewText READ previewText NOTIFY previewChanged)
    Q_PROPERTY(QString previewKind READ previewKind NOTIFY previewChanged)
    Q_PROPERTY(bool previewBusy READ previewBusy NOTIFY previewChanged)
    Q_PROPERTY(int maxPointerDepth READ maxPointerDepth WRITE setMaxPointerDepth NOTIFY depthChanged)
    Q_PROPERTY(int maxReferenceDepth READ maxReferenceDepth WRITE setMaxReferenceDepth NOTIFY depthChanged)

public:
    explicit ExplorerController(QObject *parent = nullptr);
    ~ExplorerController() override;

    P4kTreeModel *tree() const { return tree_; }
    QString archivePath() const { return archivePath_; }
    bool loaded() const { return data_ != nullptr; }
    bool forgeLoaded() const { return data_ && data_->forge; }
    QString summary() const;
    int recordsNode() const;
    QVariantList results() const { return results_; }
    QString resultsNote() const { return resultsNote_; }
    bool searching() const { return searching_; }
    int currentNode() const { return currentNode_; }
    QString previewTitle() const { return previewTitle_; }
    QString previewInfo() const { return previewInfo_; }
    QString previewText() const { return previewText_; }
    QString previewKind() const { return previewKind_; }
    bool previewBusy() const { return previewBusy_; }
    int maxPointerDepth() const { return maxPointerDepth_; }
    void setMaxPointerDepth(int v);
    int maxReferenceDepth() const { return maxReferenceDepth_; }
    void setMaxReferenceDepth(int v);

    Q_INVOKABLE void open(const QString &path = {});
    Q_INVOKABLE void openUrl(const QUrl &url);
    Q_INVOKABLE void close();
    Q_INVOKABLE void loadDataForge();
    Q_INVOKABLE void search(const QString &pattern);
    Q_INVOKABLE void select(int node);
    Q_INVOKABLE QString nodePath(int node) const;
    Q_INVOKABLE bool isFolder(int node) const;
    Q_INVOKABLE int findNode(const QString &path) const;
    Q_INVOKABLE void extract(const QVariantList &nodes, const QUrl &outputDir, bool convertCryXml,
                             bool skipExisting);
    Q_INVOKABLE QString defaultExtractDir() const;
    Q_INVOKABLE QString defaultGameDataPath() const;
    Q_INVOKABLE void exportGameData(const QUrl &output, const QString &channel, const QUrl &baseIni,
                                    const QUrl &overlay);
    Q_INVOKABLE void copyPath(int node) const;

signals:
    void loadedChanged();
    void resultsChanged();
    void previewChanged();
    void depthChanged();

private:
    AppController &app() const;
    void setLoaded(std::shared_ptr<const ExplorerData> data, const QString &path);
    void setPreview(int node, const QString &title, const QString &info, const QString &text,
                    const QString &kind, bool busy);

    P4kTreeModel *tree_ = nullptr;
    std::shared_ptr<const ExplorerData> data_;
    QString archivePath_;
    QVariantList results_;
    QString resultsNote_;
    bool searching_ = false;
    std::shared_ptr<std::atomic<quint64>> searchSerial_ = std::make_shared<std::atomic<quint64>>(0);
    quint64 previewSerial_ = 0;
    int currentNode_ = -1;
    QString previewTitle_, previewInfo_, previewText_, previewKind_ = QStringLiteral("none");
    bool previewBusy_ = false;
    int maxPointerDepth_ = 100;
    int maxReferenceDepth_ = 1;
};
