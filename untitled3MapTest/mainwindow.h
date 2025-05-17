#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QLineEdit>
#include <QPushButton>
#include <QComboBox>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QMap>
#include <QVector>
#include <QPair>
#include <QMessageBox>
#include <QMouseEvent>
#include <QDialog>

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_searchButton_clicked(const QString &searchQuery);
    void on_pathSearchButton_clicked(const QString &currentQuery, const QString &targetQuery, const QString &transport, const QString &strategy);
    void on_multiPathSearchButton_clicked(const QStringList &attractionsList, const QString &transport);
    void on_recommendButton_clicked();
    void on_internalSearchButton_clicked(const QString &searchQuery, QGraphicsScene *internalScene, const QString &targetType);
    void on_internalPathSearchButton_clicked(const QString &currentQuery, const QString &targetQuery);
    void on_internalFoodSearchButton_clicked(const QString &foodName); // 新增槽函数
    void on_cuisineRecommendButton_clicked(const QString &type); // 新增槽函数
private:
    QGraphicsScene *scene;
    QMap<QString, QPair<int, QPointF>> attractionsWithPopularity;
    QMap<QString, QMap<QString, QMap<QString, int>>> graph;
    QMap<QString, QMap<QString, QMap<QString, int>>> internalGraph;
    QMap<QString, QPointF> internalAttractions;
    QVector<QPair<QString, QString>> internalPaths;
    QJsonArray foodData; // 新增成员变量，存储美食数据
    QSet<QString> cuisines;

    void loadAttractions();
    void loadInternalAttractions();
    void loadFoodData(); // 新增方法，加载美食数据
    void drawMap();
    QVector<QString> dijkstra(const QMap<QString, QMap<QString, QMap<QString, int>>> &graph, const QString &start, const QString &end, const QString &transport);
    int calculateTotalTime(const QMap<QString, QMap<QString, QMap<QString, int>>> &graph, const QVector<QString> &path, const QString &transport);
    QString fuzzyMatchAttraction(const QString &query, const QMap<QString, QPair<int, QPointF>> &attractions);
    QString fuzzyMatchAttraction(const QString &query, const QMap<QString, QPointF> &attractions);

    bool eventFilter(QObject *obj, QEvent *event);
    void showInternalMap(const QString &attraction);

    QMap<QString, QMap<QString, int>> buildMST(const QMap<QString, QMap<QString, QMap<QString, int>>> &graph, const QString &transport, const QStringList &targetAttractions);
    void dfs(const QMap<QString, QMap<QString, int>> &mst, const QString &current, const QSet<QString> &targets, QSet<QString> &visited, QVector<QString> &path, bool &foundAll);
    int calculateTotalDistance(const QMap<QString, QMap<QString, int>> &graph, const QVector<QString> &path, const QString &transport);
    QVector<QString> topKAttractions(int k); // 新增成员函数
};
#endif // MAINWINDOW_H
