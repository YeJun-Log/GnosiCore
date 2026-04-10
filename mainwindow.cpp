#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <utility>
#include <QFileSystemModel>
#include <QFile>
#include <QTextStream>
#include <QMessageBox>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QProcessEnvironment>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    fileModel = new QFileSystemModel(this);
    fileModel->setRootPath(QDir::currentPath());

    proxyModel = new QSortFilterProxyModel(this);
    proxyModel -> setSourceModel(fileModel);
    proxyModel -> setFilterCaseSensitivity(Qt::CaseInsensitive);

    ui -> treeView -> setModel(proxyModel);

    QModelIndex sourceIndex = fileModel -> index(QDir::currentPath());
    ui -> treeView -> setRootIndex(proxyModel -> mapFromSource(sourceIndex));

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_treeView_clicked(const QModelIndex &index)
{
    QModelIndex sourceIndex = proxyModel -> mapToSource(index);
    QString filePath = fileModel -> filePath(sourceIndex);

    if(!fileModel->isDir(sourceIndex)){
        QFile file(filePath);

        if(file.open(QIODevice::ReadOnly | QIODevice::Text)){
            QTextStream in(&file);
            in.setEncoding(QStringConverter::Utf8);

            ui -> textEdit -> setPlainText(in.readAll());
            file.close();
        } else {
            QMessageBox::warning(this, "Error", "Cannot open File!");
        }
    }
}


void MainWindow::on_pushButton_clicked()
{
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();

    QString apiKey = env.value("GEMINI_API_KEY");

    if (apiKey.isEmpty()) {
        ui->textEdit->append("\n[에러] API 키 환경 변수가 설정되지 않았어!");
        return;
    }

    QString apiUrl = "https://generativelanguage.googleapis.com/v1beta/models/gemini-3.1-flash-lite-preview:generateContent?key=" + apiKey;

    // 1. 요청 보낼 데이터 준비 (JSON)

    QString promptText = "다음 노트를 분석해서 반드시 아래 JSON 형식으로만 응답해. 다른 설명은 하지 마.\n"
                         "형식: {\"summary\": \"3줄 요약 내용\", \"keywords\": [\"키워드1\", \"키워드2\", \"키워드3\"]}\n"
                         "내용: " + ui->textEdit->toPlainText();

    QJsonObject prompt;
    prompt["text"] = promptText;

    QJsonArray parts;
    parts.append(QJsonObject{{"text", prompt["text"]}});

    QJsonObject contents;
    contents["parts"] = parts;

    QJsonArray contentsArray;
    contentsArray.append(contents);

    QJsonObject root;
    root["contents"] = contentsArray;

    // 2. HTTP 요청 설정
    QNetworkRequest request((QUrl(apiUrl)));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    // 3. 네트워크 매니저를 통해 POST 요청 전송
    QNetworkAccessManager *manager = new QNetworkAccessManager(this);

    // 응답이 오면 실행될 작업 연결
    connect(manager, &QNetworkAccessManager::finished, this, [=](QNetworkReply *reply) {
        if (reply->error() == QNetworkReply::NoError) {
            QByteArray response = reply->readAll();
            QJsonDocument jsonResponse = QJsonDocument::fromJson(response);
            QJsonObject rootObj = jsonResponse.object(); // 임시 객체 고정!

            // 체인 방식 대신 안전하게 하나씩 확인하며 파싱
            QJsonArray candidates = rootObj.value("candidates").toArray();
            if (!candidates.isEmpty()) {
                QJsonObject contentObj = candidates.at(0).toObject().value("content").toObject();
                QJsonArray parts = contentObj.value("parts").toArray();

                if (!parts.isEmpty()) {
                    QString rawText = parts.at(0).toObject().value("text").toString();

                    // AI 응답(JSON 문자열) 파싱
                    QJsonDocument structuredDoc = QJsonDocument::fromJson(rawText.toUtf8());
                    QJsonObject obj = structuredDoc.object();

                    ui->textEdit->append("\n\n--- AI 자동 요약 ---\n" + obj.value("summary").toString());

                    ui->listWidgetKeywords->clear();
                    QJsonArray keywords = obj.value("keywords").toArray();
                    for (const QJsonValue &val : std::as_const(keywords)) {
                        ui->listWidgetKeywords->addItem(val.toString());
                    }
                }
            }
        } else {
            ui->textEdit->append("\n\n에러 발생: " + reply->errorString());
        }
        reply->deleteLater();
        manager->deleteLater();
    });
    manager->post(request, QJsonDocument(root).toJson());
}


void MainWindow::on_listWidgetKeywords_itemClicked(QListWidgetItem *item)
{
    QString keyword = item -> text();
    proxyModel -> setFilterFixedString(keyword);
    ui -> textEdit -> append("\n\n[알림] '" + keyword + "' 키워드로 필터링된 결과");
}


void MainWindow::on_pushButton_2_clicked()
{
    proxyModel -> setFilterFixedString("");
}

