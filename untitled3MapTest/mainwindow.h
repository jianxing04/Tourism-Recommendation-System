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
    void on_pathSearchButton_clicked(const QString &currentQuery, const QString &targetQuery, const QString &transport);

private:
    QGraphicsScene *scene;
    QMap<QString, QPair<int, QPointF>> attractionsWithPopularity;
    QMap<QString, QMap<QString, QMap<QString, int>>> graph;
    QMap<QString, QMap<QString, QMap<QString, int>>> internalGraph;
    QMap<QString, QPointF> internalAttractions;
    QVector<QPair<QString, QString>> internalPaths;

    void loadAttractions();
    void loadInternalAttractions();
    void drawMap();
    QVector<QString> dijkstra(const QMap<QString, QMap<QString, QMap<QString, int>>> &graph, const QString &start, const QString &end, const QString &transport);
    int calculateTotalTime(const QMap<QString, QMap<QString, QMap<QString, int>>> &graph, const QVector<QString> &path, const QString &transport);
    QString fuzzyMatchAttraction(const QString &query, const QMap<QString, QPair<int, QPointF>> &attractions);

    bool eventFilter(QObject *obj, QEvent *event);
    void showInternalMap(const QString &attraction);
};
#endif // MAINWINDOW_H
