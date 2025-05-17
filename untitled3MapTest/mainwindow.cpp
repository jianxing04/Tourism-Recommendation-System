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
    QComboBox *strategyComBox=new QComboBox(centralWidget);
    strategyComBox->addItems({"时间最短","距离最短"});
    QPushButton *pathSearchButton = new QPushButton("路径搜索", centralWidget);
    pathSearchLayout->addWidget(new QLabel("当前位置:", centralWidget));
    pathSearchLayout->addWidget(currentLocationLineEdit);
    pathSearchLayout->addWidget(new QLabel("目标位置:", centralWidget));
    pathSearchLayout->addWidget(targetLocationLineEdit);
    pathSearchLayout->addWidget(transportComboBox);
    pathSearchLayout->addWidget(strategyComBox);
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

    // 创建推荐景点按钮
    QPushButton *recommendButton = new QPushButton("推荐景点", centralWidget);
    mainLayout->addWidget(recommendButton);

    // 加载景点数据
    loadAttractions();
    loadInternalAttractions();
    loadFoodData(); // 加载美食数据
    drawMap();

    // 连接信号和槽
    connect(searchButton, &QPushButton::clicked, this, [=]() {
        QString searchQuery = searchLineEdit->text().trimmed();
        on_searchButton_clicked(searchQuery);
    });
    connect(pathSearchButton, &QPushButton::clicked, this, [=]() {
        QString currentQuery = currentLocationLineEdit->text().trimmed();
        QString targetQuery = targetLocationLineEdit->text().trimmed();
        QString transport = transportComboBox->currentText();
        QString strategy = strategyComBox->currentText();
        on_pathSearchButton_clicked(currentQuery, targetQuery, transport,strategy);
    });
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
    connect(recommendButton, &QPushButton::clicked, this, &MainWindow::on_recommendButton_clicked); // 连接推荐按钮信号
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

void MainWindow::loadFoodData()
{
    QFile file(":/delicious_food.json");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::critical(this, "错误", "未找到 delicious_food.json 文件，请确保文件存在。");
        return;
    }

    QTextStream in(&file);
    QString jsonString = in.readAll();
    file.close();

    QJsonDocument jsonDoc = QJsonDocument::fromJson(jsonString.toUtf8());
    foodData = jsonDoc.array();

    // 获取所有菜系并添加到下拉框中
    for (const auto &food : foodData) {
        QJsonObject foodObj = food.toObject();
        cuisines.insert(foodObj["菜系"].toString());
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

void MainWindow::on_pathSearchButton_clicked(const QString &currentQuery, const QString &targetQuery, const QString &transport, const QString &strategy)
{
    QString currentLocation = fuzzyMatchAttraction(currentQuery, attractionsWithPopularity);
    QString targetLocation = fuzzyMatchAttraction(targetQuery, attractionsWithPopularity);

    if (currentLocation.isEmpty() || targetLocation.isEmpty()) {
        QMessageBox::information(this, "提示", "未找到匹配的景点。");
        return;
    }
    QString t=transport;
    //"时间最短","距离最短"
    QVector<QString> tem={"步行", "自行车", "电动车", "自驾"};
    if (strategy=="距离最短"){
        int k=1;
        for (int i=0;i<4;i++){
            if (tem[i]==transport){
                k=i;
                break;
            }
        }
        k=(k+2)%4;
        t=tem[k];
    }

    QVector<QString> path = dijkstra(graph, currentLocation, targetLocation, t);
    if (!path.isEmpty()) {
        QString pathStr = path.join(" -> ");
        int totalTime = calculateTotalTime(graph, path, t);
        if (t==transport){
            QMessageBox::information(this, "路径信息", QString("您的路径是：%1，所需时间为：%2").arg(pathStr).arg(totalTime));
        }
        else{
            QMessageBox::information(this, "路径信息", QString("您的路径是：%1，全程距离为：%2").arg(pathStr).arg(totalTime));
        }
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

    // 创建内部地图的搜索区域
    QHBoxLayout *internalSearchLayout = new QHBoxLayout();
    QLineEdit *internalSearchLineEdit = new QLineEdit(internalMapDialog);
    QPushButton *internalSearchButton = new QPushButton("搜索", internalMapDialog);
    QComboBox *targetComboBox = new QComboBox(internalMapDialog);
    targetComboBox->addItems({"卫生间", "超市"});
    internalSearchLayout->addWidget(internalSearchLineEdit);
    internalSearchLayout->addWidget(targetComboBox);
    internalSearchLayout->addWidget(internalSearchButton);
    internalLayout->addLayout(internalSearchLayout);

    // 创建内部地图的路径搜索区域
    QHBoxLayout *internalPathSearchLayout = new QHBoxLayout();
    QLineEdit *internalCurrentLocationLineEdit = new QLineEdit(internalMapDialog);
    QLineEdit *internalTargetLocationLineEdit = new QLineEdit(internalMapDialog);
    QPushButton *internalPathSearchButton = new QPushButton("路径搜索", internalMapDialog);
    internalPathSearchLayout->addWidget(new QLabel("当前位置:", internalMapDialog));
    internalPathSearchLayout->addWidget(internalCurrentLocationLineEdit);
    internalPathSearchLayout->addWidget(new QLabel("目标位置:", internalMapDialog));
    internalPathSearchLayout->addWidget(internalTargetLocationLineEdit);
    internalPathSearchLayout->addWidget(internalPathSearchButton);
    internalLayout->addLayout(internalPathSearchLayout);

    // 创建内部地图的美食搜索区域
    QHBoxLayout *internalFoodSearchLayout = new QHBoxLayout();
    QLineEdit *internalFoodSearchLineEdit = new QLineEdit(internalMapDialog);
    QPushButton *internalFoodSearchButton = new QPushButton("美食搜索", internalMapDialog);
    internalFoodSearchLayout->addWidget(internalFoodSearchLineEdit);
    internalFoodSearchLayout->addWidget(internalFoodSearchButton);
    internalLayout->addLayout(internalFoodSearchLayout);

    // 创建内部地图的美食推荐按钮
    QHBoxLayout *internalFoodRecommendLayout = new QHBoxLayout();
    QLabel *internalFoodRecommendLabel = new QLabel("推荐美食",internalMapDialog);
    QComboBox *internalFoodRecommendComBox = new QComboBox(internalMapDialog);
    internalFoodRecommendComBox->addItem("全部");
    for (const auto &cuisine:cuisines){
        internalFoodRecommendComBox->addItem(cuisine);
    }
    QPushButton *recommendFoodButton = new QPushButton("查看推荐", internalMapDialog);
    internalFoodRecommendLayout->addWidget(internalFoodRecommendLabel);
    internalFoodRecommendLayout->addWidget(internalFoodRecommendComBox);
    internalFoodRecommendLayout->addWidget(recommendFoodButton);
    internalLayout->addLayout(internalFoodRecommendLayout);

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

    // 连接内部地图的搜索按钮信号
    connect(internalSearchButton, &QPushButton::clicked, this, [=]() {
        QString searchQuery = internalSearchLineEdit->text().trimmed();
        QString targetType = targetComboBox->currentText();
        on_internalSearchButton_clicked(searchQuery,internalScene,targetType);
    });

    // 连接内部地图的路径搜索按钮信号
    connect(internalPathSearchButton, &QPushButton::clicked, this, [=]() {
        QString currentQuery = internalCurrentLocationLineEdit->text().trimmed();
        QString targetQuery = internalTargetLocationLineEdit->text().trimmed();
        on_internalPathSearchButton_clicked(currentQuery, targetQuery);
    });

    // 连接内部地图的美食搜索按钮信号
    connect(internalFoodSearchButton, &QPushButton::clicked, this, [=]() {
        QString foodName;
        foodName=internalFoodSearchLineEdit->text().trimmed();
        on_internalFoodSearchButton_clicked(foodName);
    });

    connect(recommendFoodButton,&QPushButton::clicked,this,[=](){
        QString type=internalFoodRecommendComBox->currentText();
        on_cuisineRecommendButton_clicked(type);
    });

    connect(internalMapDialog, &QDialog::finished, this, [=]() {
        this->show();
    });

    internalMapDialog->exec();
}

//处理内部地图美食搜索
void MainWindow::on_internalFoodSearchButton_clicked(const QString &foodName)
{
    for (const auto &food : foodData) {
        QJsonObject foodObj = food.toObject();
        if (foodObj["名字"].toString() == foodName) {
            QString message = QString("菜名：%1\n菜系：%2\n热度：%3\n位置：%4")
                                  .arg(foodObj["名字"].toString())
                                  .arg(foodObj["菜系"].toString())
                                  .arg(foodObj["热度"].toInt())
                                  .arg(foodObj["位置"].toString());
            QMessageBox::information(this, "美食信息", message);
            return;
        }
    }
    QMessageBox::information(this, "提示", "未找到匹配的美食。");
}

// 处理推荐菜系按钮点击事件
void MainWindow::on_cuisineRecommendButton_clicked(const QString &type)
{
    QString selectedCuisine = type;

    QVector<QPair<int, QJsonObject>> filteredFoods;
    for (const auto &food : foodData) {
        QJsonObject foodObj = food.toObject();
        if (selectedCuisine == "全部" || foodObj["菜系"].toString() == selectedCuisine) {
            filteredFoods.append(qMakePair(foodObj["热度"].toInt(), foodObj));
        }
    }

    std::sort(filteredFoods.begin(), filteredFoods.end(), [](const auto &a, const auto &b) {
        return a.first > b.first;
    });

    QString message = QString("推荐 %1 美食：\n").arg(selectedCuisine);
    int count = 0;
    for (const auto &pair : filteredFoods) {
        if (count >= 10) break;
        QJsonObject foodObj = pair.second;
        message += QString("菜名：%1，菜系：%2，热度：%3，位置：%4\n")
                       .arg(foodObj["名字"].toString())
                       .arg(foodObj["菜系"].toString())
                       .arg(foodObj["热度"].toInt())
                       .arg(foodObj["位置"].toString());
        count++;
    }

    QMessageBox::information(this, "美食推荐", message);
}

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

// 实现 top-k 排序算法
QVector<QString> MainWindow::topKAttractions(int k)
{
    QVector<QPair<int, QString>> attractions;
    for (const auto &attraction : attractionsWithPopularity.keys()) {
        int popularity = attractionsWithPopularity[attraction].first;
        attractions.append(qMakePair(popularity, attraction));
    }

    auto partition = [&](int left, int right) {
        int pivot = attractions[right].first;
        int i = left - 1;
        for (int j = left; j < right; ++j) {
            if (attractions[j].first > pivot) {
                ++i;
                std::swap(attractions[i], attractions[j]);
            }
        }
        std::swap(attractions[i + 1], attractions[right]);
        return i + 1;
    };

     std::function<void(int, int, int)> quickSelect = [&](int left, int right, int k) {
        if (left == right) return;
        int pivotIndex = partition(left, right);
        if (pivotIndex == k) return;
        else if (pivotIndex > k) quickSelect(left, pivotIndex - 1, k);
        else quickSelect(pivotIndex + 1, right, k);
    };

    int n = attractions.size();
    if (k > n) k = n;
    quickSelect(0, n - 1, k - 1);

    std::sort(attractions.begin(), attractions.begin() + k, [](const auto &a, const auto &b) {
        return a.first > b.first;
    });

    QVector<QString> topK;
    for (int i = 0; i < k; ++i) {
        topK.append(attractions[i].second);
    }
    return topK;
}

// 处理推荐景点按钮点击事件
void MainWindow::on_recommendButton_clicked()
{
    QVector<QString> top10 = topKAttractions(10);
    QString message = "热度前10的景点：\n";
    for (const auto &attraction : top10) {
        int popularity = attractionsWithPopularity[attraction].first;
        message += QString("%1 (%2)\n").arg(attraction).arg(popularity);
    }
    QMessageBox::information(this, "景点推荐", message);
}

QString MainWindow::fuzzyMatchAttraction(const QString &query, const QMap<QString, QPointF> &attractions)
{
    // 实现模糊匹配逻辑，不考虑热度
    for (const auto &key : attractions.keys()) {
        if (key.contains(query, Qt::CaseInsensitive)) {
            return key;
        }
    }
    return "";
}

// void MainWindow::on_internalSearchButton_clicked(const QString &searchQuery, QGraphicsScene *internalScene)
// {
//     QString matchedAttraction = fuzzyMatchAttraction(searchQuery, internalAttractions);

//     if (!matchedAttraction.isEmpty()) {
//         QPointF point = internalAttractions[matchedAttraction];
//         internalScene->addEllipse(point.x() - 10, point.y() - 10, 20, 20, QPen(Qt::red), QBrush(Qt::red, Qt::Dense4Pattern));
//     } else {
//         QMessageBox::information(this, "提示", "未找到匹配的景点。");
//     }
// }

void MainWindow::on_internalSearchButton_clicked(const QString &searchQuery, QGraphicsScene *internalScene, const QString &targetType)
{
    QString matchedAttraction = fuzzyMatchAttraction(searchQuery, internalAttractions);

    if (!matchedAttraction.isEmpty()) {
        QPointF startPoint = internalAttractions[matchedAttraction];
        QPointF point = internalAttractions[matchedAttraction];
        internalScene->addEllipse(point.x() - 10, point.y() - 10, 20, 20, QPen(Qt::red), QBrush(Qt::red, Qt::Dense4Pattern));
        QVector<QPair<double, QString>> distances;
        for (const auto &place : internalAttractions.keys()) {
            if (place.contains(targetType)) {
                QPointF endPoint = internalAttractions[place];
                double distance = QLineF(startPoint, endPoint).length();
                distances.append(qMakePair(distance, place));
            }
        }

        std::sort(distances.begin(), distances.end(), [](const auto &a, const auto &b) {
            return a.first < b.first;
        });

        QString message = QString("距离 %1 最近的三个 %2 及距离：\n").arg(matchedAttraction).arg(targetType);
        int count = 0;
        for (const auto &pair : distances) {
            if (count >= 3) break;
            QPointF point = internalAttractions[pair.second];
            internalScene->addEllipse(point.x() - 5, point.y() - 5, 10, 10, QPen(Qt::green), QBrush(Qt::green));
            message += QString("%1: %2\n").arg(pair.second).arg(pair.first);
            count++;
        }

        QMessageBox::information(this, "距离信息", message);
    } else {
        QMessageBox::information(this, "提示", "未找到匹配的景点。");
    }
}

void MainWindow::on_internalPathSearchButton_clicked(const QString &currentQuery, const QString &targetQuery)
{
    QString currentLocation = fuzzyMatchAttraction(currentQuery, internalAttractions);
    QString targetLocation = fuzzyMatchAttraction(targetQuery, internalAttractions);

    if (currentLocation.isEmpty() || targetLocation.isEmpty()) {
        QMessageBox::information(this, "提示", "未找到匹配的景点。");
        return;
    }

    QVector<QString> path = dijkstra(internalGraph, currentLocation, targetLocation, "步行");
    if (!path.isEmpty()) {
        QString pathStr = path.join(" -> ");
        QMessageBox::information(this, "路径信息", QString("您的路径是：%1").arg(pathStr));
    } else {
        QMessageBox::information(this, "提示", "未找到可行路径。");
    }
}
