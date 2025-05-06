#include "mainwindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGraphicsView>
#include <QLineEdit>
#include <QPushButton>
#include <QComboBox>
#include <QFile>
#include <QTextStream>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QMap>
#include <QVector>
#include <QPair>
#include <QMessageBox>
#include <QMouseEvent>
#include <QDialog>
#include <QLabel>
#include <QGraphicsTextItem>
#include <climits>


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    // 创建主布局
    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);
    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);

    // 创建地图显示区域
    scene = new QGraphicsScene(this);
    QGraphicsView *graphicsView = new QGraphicsView(scene, centralWidget);
    mainLayout->addWidget(graphicsView);

    // 创建搜索区域
    QHBoxLayout *searchLayout = new QHBoxLayout();
    QLineEdit *searchLineEdit = new QLineEdit(centralWidget);
    QPushButton *searchButton = new QPushButton("搜索", centralWidget);
    searchLayout->addWidget(searchLineEdit);
    searchLayout->addWidget(searchButton);
    mainLayout->addLayout(searchLayout);

    // 创建路径搜索区域
    QHBoxLayout *pathSearchLayout = new QHBoxLayout();
    QLineEdit *currentLocationLineEdit = new QLineEdit(centralWidget);
    QLineEdit *targetLocationLineEdit = new QLineEdit(centralWidget);
    QComboBox *transportComboBox = new QComboBox(centralWidget);
    transportComboBox->addItems({"步行", "自行车", "电动车", "自驾"});
    QPushButton *pathSearchButton = new QPushButton("路径搜索", centralWidget);
    pathSearchLayout->addWidget(new QLabel("当前位置:", centralWidget));
    pathSearchLayout->addWidget(currentLocationLineEdit);
    pathSearchLayout->addWidget(new QLabel("目标位置:", centralWidget));
    pathSearchLayout->addWidget(targetLocationLineEdit);
    pathSearchLayout->addWidget(transportComboBox);
    pathSearchLayout->addWidget(pathSearchButton);
    mainLayout->addLayout(pathSearchLayout);

    // 创建多景点路径搜索区域
    QHBoxLayout *multiPathSearchLayout = new QHBoxLayout();
    QLineEdit *numAttractionsLineEdit = new QLineEdit(centralWidget);
    QLineEdit *attractionsLineEdit = new QLineEdit(centralWidget);
    QComboBox *multiTransportComboBox = new QComboBox(centralWidget);
    multiTransportComboBox->addItems({"步行", "自行车", "电动车", "自驾"});
    QPushButton *multiPathSearchButton = new QPushButton("多景点路径搜索", centralWidget);
    multiPathSearchLayout->addWidget(new QLabel("景点个数:", centralWidget));
    multiPathSearchLayout->addWidget(numAttractionsLineEdit);
    multiPathSearchLayout->addWidget(new QLabel("景点名字（用逗号分隔）:", centralWidget));
    multiPathSearchLayout->addWidget(attractionsLineEdit);
    multiPathSearchLayout->addWidget(multiTransportComboBox);
    multiPathSearchLayout->addWidget(multiPathSearchButton);
    mainLayout->addLayout(multiPathSearchLayout);

    // 加载景点数据
    loadAttractions();
    loadInternalAttractions();
    drawMap();

    //QMap<QString, QMap<QString, int>> mst = buildMST(graph, transportComboBox->currentText());

    // 连接信号和槽
    connect(searchButton, &QPushButton::clicked, this, [=]() {
        QString searchQuery = searchLineEdit->text().trimmed();
        on_searchButton_clicked(searchQuery);
    });
    connect(pathSearchButton, &QPushButton::clicked, this, [=]() {
        QString currentQuery = currentLocationLineEdit->text().trimmed();
        QString targetQuery = targetLocationLineEdit->text().trimmed();
        QString transport = transportComboBox->currentText();
        on_pathSearchButton_clicked(currentQuery, targetQuery, transport);
    });
    // connect(multiPathSearchButton, &QPushButton::clicked, this, [=]() {
    //     int numAttractions = numAttractionsLineEdit->text().toInt();
    //     QString attractionsStr = attractionsLineEdit->text().trimmed();
    //     QString transport = multiTransportComboBox->currentText();
    //     QStringList attractionsList = attractionsStr.split(',');
    //     if (attractionsList.size() != numAttractions) {
    //         QMessageBox::information(this, "提示", "输入的景点个数与实际输入的景点名字数量不匹配。");
    //         return;
    //     }
    //     on_multiPathSearchButton_clicked(attractionsList, transport, mst);
    // });
    connect(multiPathSearchButton, &QPushButton::clicked, this, [=]() {
        int numAttractions = numAttractionsLineEdit->text().toInt();
        QString attractionsStr = attractionsLineEdit->text().trimmed();
        QString transport = multiTransportComboBox->currentText();
        QStringList attractionsList = attractionsStr.split(',');
        if (attractionsList.size() != numAttractions) {
            QMessageBox::information(this, "提示", "输入的景点个数与实际输入的景点名字数量不匹配。");
            return;
        }
        on_multiPathSearchButton_clicked(attractionsList, transport);
    });
    graphicsView->viewport()->installEventFilter(this);
}

MainWindow::~MainWindow()
{
    // 析构函数中可以添加资源释放的代码
}

void MainWindow::loadAttractions()
{
    QFile file(":/attractions.json");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::critical(this, "错误", "未找到 attractions.json 文件，请确保文件存在。");
        return;
    }

    QTextStream in(&file);
    QString jsonString = in.readAll();
    file.close();

    QJsonDocument jsonDoc = QJsonDocument::fromJson(jsonString.toUtf8());
    QJsonObject jsonObj = jsonDoc.object();

    QJsonObject attractionsObj = jsonObj["attractions"].toObject();
    QJsonArray pathsArray = jsonObj["paths"].toArray();

    for (const auto &key : attractionsObj.keys()) {
        QJsonArray pointArray = attractionsObj[key].toArray();
        QPointF point(pointArray[0].toDouble(), pointArray[1].toDouble());
        int popularity = rand() % 100 + 1;
        attractionsWithPopularity[key] = qMakePair(popularity, point);
    }

    for (const auto &pathValue : pathsArray) {
        QJsonArray pathArray = pathValue.toArray();
        QString start = pathArray[0].toString();
        QString end = pathArray[1].toString();
        QJsonObject transportTimesObj = pathArray[2].toObject();

        if (!graph.contains(start)) {
            graph[start] = QMap<QString, QMap<QString, int>>();
        }
        if (!graph.contains(end)) {
            graph[end] = QMap<QString, QMap<QString, int>>();
        }

        QMap<QString, int> transportTimes;
        for (const auto &transport : transportTimesObj.keys()) {
            transportTimes[transport] = transportTimesObj[transport].toInt();
        }

        graph[start][end] = transportTimes;
        graph[end][start] = transportTimes;
    }
}

void MainWindow::loadInternalAttractions()
{
    QFile file(":/attraction_internal.json");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::critical(this, "错误", "未找到 attraction_internal.json 文件，请确保文件存在。");
        return;
    }

    QTextStream in(&file);
    QString jsonString = in.readAll();
    file.close();

    QJsonDocument jsonDoc = QJsonDocument::fromJson(jsonString.toUtf8());
    QJsonObject jsonObj = jsonDoc.object();

    QJsonObject attractionsObj = jsonObj["attractions"].toObject();
    QJsonArray pathsArray = jsonObj["paths"].toArray();

    for (const auto &key : attractionsObj.keys()) {
        QJsonArray pointArray = attractionsObj[key].toArray();
        QPointF point(pointArray[0].toDouble(), pointArray[1].toDouble());
        internalAttractions[key] = point;
    }

    for (const auto &pathValue : pathsArray) {
        QJsonArray pathArray = pathValue.toArray();
        QString start = pathArray[0].toString();
        QString end = pathArray[1].toString();

        if (!internalGraph.contains(start)) {
            internalGraph[start] = QMap<QString, QMap<QString, int>>();
        }
        if (!internalGraph.contains(end)) {
            internalGraph[end] = QMap<QString, QMap<QString, int>>();
        }

        QMap<QString, int> transportTimes;
        transportTimes["步行"] = 1;

        internalGraph[start][end] = transportTimes;
        internalGraph[end][start] = transportTimes;

        internalPaths.append(qMakePair(start, end));
    }
}

void MainWindow::drawMap()
{
    // 绘制景点路径
    for (const auto &path : graph.keys()) {
        for (const auto &neighbor : graph[path].keys()) {
            QPointF startPoint = attractionsWithPopularity[path].second;
            QPointF endPoint = attractionsWithPopularity[neighbor].second;
            scene->addLine(startPoint.x(), startPoint.y(), endPoint.x(), endPoint.y(), QPen(Qt::blue));
        }
    }

    // 绘制景点和热度
    for (const auto &attraction : attractionsWithPopularity.keys()) {
        QPointF point = attractionsWithPopularity[attraction].second;
        int popularity = attractionsWithPopularity[attraction].first;
        scene->addEllipse(point.x() - 5, point.y() - 5, 10, 10, QPen(Qt::black), QBrush(Qt::black));
        scene->addText(QString("%1 (%2)").arg(attraction).arg(popularity))->setPos(point.x() + 5, point.y());
    }
}

QVector<QString> MainWindow::dijkstra(const QMap<QString, QMap<QString, QMap<QString, int>>> &graph, const QString &start, const QString &end, const QString &transport)
{
    QMap<QString, int> distances;
    QMap<QString, QString> previousNodes;
    QSet<QString> unvisitedNodes;

    for (const auto &node : graph.keys()) {
        distances[node] = INT_MAX;
        previousNodes[node] = "";
        unvisitedNodes.insert(node);
    }

    distances[start] = 0;

    while (!unvisitedNodes.isEmpty()) {
        QString currentNode;
        int minDistance = INT_MAX;

        for (const auto &node : unvisitedNodes) {
            if (distances[node] < minDistance) {
                minDistance = distances[node];
                currentNode = node;
            }
        }

        if (currentNode.isEmpty()) {
            break;
        }

        unvisitedNodes.remove(currentNode);

        for (const auto &neighbor : graph[currentNode].keys()) {
            if (graph[currentNode][neighbor].contains(transport)) {
                int newDistance = distances[currentNode] + graph[currentNode][neighbor][transport];
                if (newDistance < distances[neighbor]) {
                    distances[neighbor] = newDistance;
                    previousNodes[neighbor] = currentNode;
                }
            }
        }
    }

    QVector<QString> path;
    QString currentNode = end;

    while (!currentNode.isEmpty()) {
        path.prepend(currentNode);
        currentNode = previousNodes[currentNode];
    }

    if (path.first() != start) {
        path.clear();
    }

    return path;
}

int MainWindow::calculateTotalTime(const QMap<QString, QMap<QString, QMap<QString, int>>> &graph, const QVector<QString> &path, const QString &transport)
{
    int totalTime = 0;
    for (int i = 0; i < path.size() - 1; ++i) {
        QString startNode = path[i];
        QString endNode = path[i + 1];
        totalTime += graph[startNode][endNode][transport];
    }
    return totalTime;
}

QString MainWindow::fuzzyMatchAttraction(const QString &query, const QMap<QString, QPair<int, QPointF>> &attractions)
{
    QString bestMatch;
    int bestScore = 0;

    for (const auto &attraction : attractions.keys()) {
        int score = 0;
        for (const auto &c : query) {
            if (attraction.contains(c)) {
                score++;
            }
        }
        if (score > bestScore) {
            bestScore = score;
            bestMatch = attraction;
        }
    }

    return bestMatch;
}

void MainWindow::on_searchButton_clicked(const QString &searchQuery)
{
    QString matchedAttraction = fuzzyMatchAttraction(searchQuery, attractionsWithPopularity);

    if (!matchedAttraction.isEmpty()) {
        QPointF point = attractionsWithPopularity[matchedAttraction].second;
        scene->addEllipse(point.x() - 10, point.y() - 10, 20, 20, QPen(Qt::red), QBrush(Qt::red, Qt::Dense4Pattern));
    } else {
        QMessageBox::information(this, "提示", "未找到匹配的景点。");
    }
}

void MainWindow::on_pathSearchButton_clicked(const QString &currentQuery, const QString &targetQuery, const QString &transport)
{
    QString currentLocation = fuzzyMatchAttraction(currentQuery, attractionsWithPopularity);
    QString targetLocation = fuzzyMatchAttraction(targetQuery, attractionsWithPopularity);

    if (currentLocation.isEmpty() || targetLocation.isEmpty()) {
        QMessageBox::information(this, "提示", "未找到匹配的景点。");
        return;
    }

    QVector<QString> path = dijkstra(graph, currentLocation, targetLocation, transport);
    if (!path.isEmpty()) {
        QString pathStr = path.join(" -> ");
        int totalTime = calculateTotalTime(graph, path, transport);
        QMessageBox::information(this, "路径信息", QString("您的路径是：%1，所需时间为：%2").arg(pathStr).arg(totalTime));
    } else {
        QMessageBox::information(this, "提示", "未找到可行路径。");
    }
}

bool MainWindow::eventFilter(QObject *obj, QEvent *event)
{
    if (obj->isWidgetType() && static_cast<QWidget*>(obj)->parent() == findChild<QGraphicsView*>()) {
        if (event->type() == QEvent::MouseButtonPress) {
            QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
            QPointF scenePos = findChild<QGraphicsView*>()->mapToScene(mouseEvent->pos());
            for (const auto &attraction : attractionsWithPopularity.keys()) {
                QPointF point = attractionsWithPopularity[attraction].second;
                if (QLineF(scenePos, point).length() < 10) {
                    showInternalMap(attraction);
                    break;
                }
            }
            return true;
        }
    }
    return QObject::eventFilter(obj, event);
}

void MainWindow::showInternalMap(const QString &attraction)
{
    this->hide();
    QDialog *internalMapDialog = new QDialog(this);
    internalMapDialog->setWindowTitle(QString("%1 内部地图").arg(attraction));

    QVBoxLayout *internalLayout = new QVBoxLayout(internalMapDialog);
    QGraphicsScene *internalScene = new QGraphicsScene(internalMapDialog);
    QGraphicsView *internalView = new QGraphicsView(internalScene, internalMapDialog);
    internalLayout->addWidget(internalView);

    // 绘制内部景点路径
    for (const auto &path : internalPaths) {
        QPointF startPoint = internalAttractions[path.first];
        QPointF endPoint = internalAttractions[path.second];
        internalScene->addLine(startPoint.x(), startPoint.y(), endPoint.x(), endPoint.y(), QPen(Qt::blue));
    }

    // 绘制内部景点
    for (const auto &place : internalAttractions.keys()) {
        QPointF point = internalAttractions[place];
        internalScene->addEllipse(point.x() - 5, point.y() - 5, 10, 10, QPen(Qt::black), QBrush(Qt::black));
        internalScene->addText(place)->setPos(point.x() + 5, point.y());
    }

    connect(internalMapDialog, &QDialog::finished, this, [=]() {
        this->show();
    });

    internalMapDialog->exec();
}

// QVector<QString> MainWindow::prim(const QMap<QString, QMap<QString, QMap<QString, int>>> &graph, const QVector<QString> &nodes, const QString &transport)
// {
//     QVector<QString> path;
//     if (nodes.isEmpty()) return path;

//     QSet<QString> visited;
//     QMap<QString, int> distances;
//     QMap<QString, QString> previousNodes;

//     for (const auto &node : nodes) {
//         distances[node] = INT_MAX;
//         previousNodes[node] = "";
//     }

//     QString startNode = nodes.first();
//     distances[startNode] = 0;

//     while (visited.size() < nodes.size()) {
//         QString currentNode;
//         int minDistance = INT_MAX;

//         for (const auto &node : nodes) {
//             if (!visited.contains(node) && distances[node] < minDistance) {
//                 minDistance = distances[node];
//                 currentNode = node;
//             }
//         }

//         if (currentNode.isEmpty()) break;

//         visited.insert(currentNode);
//         path.append(currentNode);

//         for (const auto &neighbor : nodes) {
//             if (!visited.contains(neighbor) && graph[currentNode].contains(neighbor) && graph[currentNode][neighbor].contains(transport)) {
//                 int newDistance = graph[currentNode][neighbor][transport];
//                 if (newDistance < distances[neighbor]) {
//                     distances[neighbor] = newDistance;
//                     previousNodes[neighbor] = currentNode;
//                 }
//             }
//         }
//     }

//     return path;
// }

// void MainWindow::on_multiPathSearchButton_clicked(const QStringList &attractionsList, const QString &transport)
// {
//     QVector<QString> nodes;
//     for (const auto &attraction : attractionsList) {
//         QString matchedAttraction = fuzzyMatchAttraction(attraction, attractionsWithPopularity);
//         if (matchedAttraction.isEmpty()) {
//             QMessageBox::information(this, "提示", QString("未找到匹配的景点: %1").arg(attraction));
//             return;
//         }
//         nodes.append(matchedAttraction);
//     }

//     QVector<QString> path = prim(graph, nodes, transport);
//     if (!path.isEmpty()) {
//         QString pathStr = path.join(" -> ");
//         int totalTime = calculateTotalTime(graph, path, transport);
//         QMessageBox::information(this, "路径信息", QString("您的路径是：%1，所需时间为：%2").arg(pathStr).arg(totalTime));
//     } else {
//         QMessageBox::information(this, "提示", "未找到可行路径。");
//     }
// }

// 构建最小生成树（Prim算法）
// QMap<QString, QMap<QString, int>> MainWindow::buildMST(const QMap<QString, QMap<QString, QMap<QString, int>>> &graph, const QString &transport)
// {
//     QMap<QString, QMap<QString, int>> mst;
//     QSet<QString> visited;
//     QMap<QString, int> distances;
//     QMap<QString, QString> previousNodes;

//     auto allNodes = graph.keys();
//     if (allNodes.isEmpty()) return mst;

//     QString startNode = allNodes.first();
//     for (const auto &node : allNodes) {
//         distances[node] = INT_MAX;
//         previousNodes[node] = "";
//     }
//     distances[startNode] = 0;

//     while (visited.size() < allNodes.size()) {
//         QString currentNode;
//         int minDistance = INT_MAX;

//         for (const auto &node : allNodes) {
//             if (!visited.contains(node) && distances[node] < minDistance) {
//                 minDistance = distances[node];
//                 currentNode = node;
//             }
//         }

//         if (currentNode.isEmpty()) break;

//         visited.insert(currentNode);
//         if (!previousNodes[currentNode].isEmpty()) {
//             mst[previousNodes[currentNode]][currentNode] = minDistance;
//             mst[currentNode][previousNodes[currentNode]] = minDistance;
//         }

//         for (const auto &neighbor : graph[currentNode].keys()) {
//             if (!visited.contains(neighbor) && graph[currentNode][neighbor].contains(transport)) {
//                 int newDistance = graph[currentNode][neighbor][transport];
//                 if (newDistance < distances[neighbor]) {
//                     distances[neighbor] = newDistance;
//                     previousNodes[neighbor] = currentNode;
//                 }
//             }
//         }
//     }

//     return mst;
// }

// 构建最小生成树（Prim算法）
QMap<QString, QMap<QString, int>> MainWindow::buildMST(const QMap<QString, QMap<QString, QMap<QString, int>>> &graph, const QString &transport, const QStringList &targetAttractions)
{
    QMap<QString, QMap<QString, int>> mst;
    QSet<QString> visited;
    QMap<QString, int> distances;
    QMap<QString, QString> previousNodes;

    auto allNodes = graph.keys();
    if (allNodes.isEmpty() || targetAttractions.isEmpty()) return mst;

    QString startNode = targetAttractions.first();
    for (const auto &node : allNodes) {
        distances[node] = INT_MAX;
        previousNodes[node] = "";
    }
    distances[startNode] = 0;

    QSet<QString> targetSet(targetAttractions.begin(),targetAttractions.end());

    while (visited.size() < allNodes.size()) {
        QString currentNode;
        int minDistance = INT_MAX;

        for (const auto &node : allNodes) {
            if (!visited.contains(node) && distances[node] < minDistance) {
                minDistance = distances[node];
                currentNode = node;
            }
        }

        if (currentNode.isEmpty()) break;

        visited.insert(currentNode);
        if (!previousNodes[currentNode].isEmpty()) {
            mst[previousNodes[currentNode]][currentNode] = minDistance;
            mst[currentNode][previousNodes[currentNode]] = minDistance;
        }

        if (targetSet.contains(currentNode)) {
            targetSet.remove(currentNode);
            if (targetSet.isEmpty()) {
                break; // 所有目标景点都已包含在生成树中，停止构建
            }
        }

        for (const auto &neighbor : graph[currentNode].keys()) {
            if (!visited.contains(neighbor) && graph[currentNode][neighbor].contains(transport)) {
                int newDistance = graph[currentNode][neighbor][transport];
                if (newDistance < distances[neighbor]) {
                    distances[neighbor] = newDistance;
                    previousNodes[neighbor] = currentNode;
                }
            }
        }
    }

    return mst;
}

// 深度优先搜索最小生成树
void MainWindow::dfs(const QMap<QString, QMap<QString, int>> &mst, const QString &current, const QSet<QString> &targets, QSet<QString> &visited, QVector<QString> &path, bool &foundAll)
{
    visited.insert(current);
    path.append(current);

    if (targets.contains(current)) {
        if (targets.size() == 1) {
            foundAll = true;
            return;
        }
        QSet<QString> newTargets = targets;
        newTargets.remove(current);
        for (const auto &neighbor : mst[current].keys()) {
            if (!visited.contains(neighbor)) {
                dfs(mst, neighbor, newTargets, visited, path, foundAll);
                if (foundAll) return;
            }
        }
    } else {
        for (const auto &neighbor : mst[current].keys()) {
            if (!visited.contains(neighbor)) {
                dfs(mst, neighbor, targets, visited, path, foundAll);
                if (foundAll) return;
            }
        }
    }

    path.removeLast();
}

// void MainWindow::on_multiPathSearchButton_clicked(const QStringList &attractionsList, const QString &transport, const QMap<QString, QMap<QString, int>> &mst)
// {
//     QVector<QString> nodes;
//     for (const auto &attraction : attractionsList) {
//         QString matchedAttraction = fuzzyMatchAttraction(attraction, attractionsWithPopularity);
//         if (matchedAttraction.isEmpty()) {
//             QMessageBox::information(this, "提示", QString("未找到匹配的景点: %1").arg(attraction));
//             return;
//         }
//         nodes.append(matchedAttraction);
//     }

//     //QSet<QString> targetSet = QSet<QString>::fromList(nodes.toList());
//     QSet<QString> targetSet;
//     for (const auto& node : nodes) {
//         targetSet.insert(node);
//     }

//     QSet<QString> visited;
//     QVector<QString> path;
//     bool foundAll = false;

//     // 可以选择任意一个目标景点作为起始点
//     if (!nodes.isEmpty()) {
//         dfs(mst, nodes.first(), targetSet, visited, path, foundAll);
//     }

//     if (foundAll) {
//         QString pathStr = path.join(" -> ");
//         int totalTime = calculateTotalDistance(mst, path, transport);
//         QMessageBox::information(this, "路径信息", QString("您的路径是：%1，所需时间为：%2").arg(pathStr).arg(totalTime));
//     } else {
//         QMessageBox::information(this, "提示", "未找到可行路径。");
//     }
// }

void MainWindow::on_multiPathSearchButton_clicked(const QStringList &attractionsList, const QString &transport)
{
    QStringList validAttractions;
    for (const auto &attraction : attractionsList) {
        QString matchedAttraction = fuzzyMatchAttraction(attraction, attractionsWithPopularity);
        if (!matchedAttraction.isEmpty()) {
            validAttractions.append(matchedAttraction);
        }
    }

    if (validAttractions.isEmpty()) {
        QMessageBox::information(this, "提示", "未找到匹配的景点。");
        return;
    }

    QMap<QString, QMap<QString, int>> mst = buildMST(graph, transport, validAttractions);

    QSet<QString> targets(attractionsList.begin(), attractionsList.end());
    QSet<QString> visited;
    QVector<QString> path;
    bool foundAll = false;

    dfs(mst, validAttractions.first(), targets, visited, path, foundAll);

    if (foundAll) {
        QString pathStr = path.join(" -> ");
        int totalTime = calculateTotalTime(graph, path, transport);
        QMessageBox::information(this, "路径信息", QString("您的路径是：%1，所需时间为：%2").arg(pathStr).arg(totalTime));
    } else {
        QMessageBox::information(this, "提示", "未找到可行路径。");
    }
}

// 计算路径的总距离
int MainWindow::calculateTotalDistance(const QMap<QString, QMap<QString, int>> &graph, const QVector<QString> &path, const QString &transport)
{
    int totalDistance = 0;
    for (int i = 0; i < path.size() - 1; ++i) {
        const QString &start = path[i];
        const QString &end = path[i + 1];
        if (graph[start].contains(end)) {
            totalDistance += graph[start][end];
        } else {
            return INT_MAX;
        }
    }
    return totalDistance;
}
