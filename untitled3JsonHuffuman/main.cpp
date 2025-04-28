#include <QCoreApplication>
#include <QFile>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>
#include <QDebug>
#include <queue>
#include <unordered_map>
#include <vector>

// 哈夫曼树节点结构体
struct HuffmanNode {
    char data;
    int freq;
    HuffmanNode *left, *right;

    HuffmanNode(char data, int freq) : data(data), freq(freq), left(nullptr), right(nullptr) {}
};

// 比较器，用于优先队列
struct compare {
    bool operator()(HuffmanNode* l, HuffmanNode* r) {
        return l->freq > r->freq;
    }
};

// 生成哈夫曼树
HuffmanNode* buildHuffmanTree(const std::string& text) {
    std::unordered_map<char, int> freq;
    for (char ch : text) {
        freq[ch]++;
    }

    std::priority_queue<HuffmanNode*, std::vector<HuffmanNode*>, compare> minHeap;

    for (const auto& entry : freq) {
        minHeap.push(new HuffmanNode(entry.first, entry.second));
    }

    while (minHeap.size() != 1) {
        HuffmanNode* left = minHeap.top();
        minHeap.pop();
        HuffmanNode* right = minHeap.top();
        minHeap.pop();

        HuffmanNode* top = new HuffmanNode('$', left->freq + right->freq);
        top->left = left;
        top->right = right;

        minHeap.push(top);
    }

    return minHeap.top();
}

// 生成哈夫曼编码表
void generateCodes(HuffmanNode* root, std::string str, std::unordered_map<char, std::string>& huffmanCode) {
    if (!root) return;

    if (!root->left && !root->right) {
        huffmanCode[root->data] = str;
    }

    generateCodes(root->left, str + "0", huffmanCode);
    generateCodes(root->right, str + "1", huffmanCode);
}

// 压缩文本
std::string compress(const std::string& text, std::unordered_map<char, std::string>& huffmanCode) {
    std::string compressed;
    for (char ch : text) {
        compressed += huffmanCode[ch];
    }
    return compressed;
}

// 解压缩文本
std::string decompress(HuffmanNode* root, const std::string& compressed) {
    std::string decompressed;
    HuffmanNode* current = root;
    for (char bit : compressed) {
        if (bit == '0') {
            current = current->left;
        } else {
            current = current->right;
        }

        if (!current->left && !current->right) {
            decompressed += current->data;
            current = root;
        }
    }
    return decompressed;
}

// 写入多条评论到JSON文件
void writeCommentsToJson(const QString& filePath, const std::vector<std::pair<int, std::pair<QString, QString>>>& comments) {
    QFile file(filePath);
    QJsonArray dataArray;

    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QByteArray jsonData = file.readAll();
        file.close();
        QJsonDocument doc = QJsonDocument::fromJson(jsonData);
        if (!doc.isNull() && doc.isArray()) {
            dataArray = doc.array();
        }
    }

    for (const auto& commentInfo : comments) {
        int userId = commentInfo.first;
        QString userName = commentInfo.second.first;
        QString comment = commentInfo.second.second;

        std::string text = comment.toStdString();
        HuffmanNode* root = buildHuffmanTree(text);
        std::unordered_map<char, std::string> huffmanCode;
        generateCodes(root, "", huffmanCode);
        std::string compressed = compress(text, huffmanCode);

        QJsonObject codeTable;
        for (const auto& entry : huffmanCode) {
            codeTable[QString(entry.first)] = QString::fromStdString(entry.second);
        }

        QJsonObject commentData;
        commentData["userId"] = userId;
        commentData["userName"] = userName;
        commentData["likes"] = 0;
        commentData["codeTable"] = codeTable;
        commentData["compressed"] = QString::fromStdString(compressed);

        dataArray.append(commentData);
    }

    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QJsonDocument newDoc(dataArray);
        file.write(newDoc.toJson(QJsonDocument::Indented));
        file.close();
    }
}

// 从JSON文件读取评论
QJsonArray readCommentsFromJson(const QString& filePath) {
    QFile file(filePath);
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QByteArray jsonData = file.readAll();
        file.close();
        QJsonDocument doc = QJsonDocument::fromJson(jsonData);
        if (!doc.isNull() && doc.isArray()) {
            return doc.array();
        }
    }
    return QJsonArray();
}

// 解码JSON文件中的评论
void decodeComments(const QJsonArray& comments) {
    for (const QJsonValue& commentValue : comments) {
        QJsonObject commentObj = commentValue.toObject();
        QJsonObject codeTable = commentObj["codeTable"].toObject();
        QString compressed = commentObj["compressed"].toString();

        std::unordered_map<char, std::string> huffmanCode;
        for (const QString& key : codeTable.keys()) {
            huffmanCode[key.at(0).toLatin1()] = codeTable[key].toString().toStdString();
        }

        HuffmanNode* root = new HuffmanNode('$', 0);
        for (const auto& entry : huffmanCode) {
            HuffmanNode* current = root;
            for (char bit : entry.second) {
                if (bit == '0') {
                    if (!current->left) {
                        current->left = new HuffmanNode('$', 0);
                    }
                    current = current->left;
                } else {
                    if (!current->right) {
                        current->right = new HuffmanNode('$', 0);
                    }
                    current = current->right;
                }
            }
            current->data = entry.first;
        }

        std::string decompressed = decompress(root, compressed.toStdString());
        qDebug() << "User ID:" << commentObj["userId"].toInt();
        qDebug() << "User Name:" << commentObj["userName"].toString();
        qDebug() << "Likes:" << commentObj["likes"].toInt();
        qDebug() << "Decompressed Comment:" << QString::fromStdString(decompressed);
    }
}

int main(int argc, char *argv[]) {
    QCoreApplication a(argc, argv);

    QString filePath = "D:\\My_Codes\\Qt_codes\\untitled3JsonHuffuman\\comments.json";

    // 定义多条评论
    std::vector<std::pair<int, std::pair<QString, QString>>> commentsToAdd = {
        {2, {"Alice", "Hi, how are you?"}},
        {3, {"Bob", "Great day today!"}}
    };

    // 写入多条评论
    writeCommentsToJson(filePath, commentsToAdd);

    // 读取所有评论
    QJsonArray allComments = readCommentsFromJson(filePath);

    // 解码并输出所有评论
    decodeComments(allComments);

    return a.exec();
}
