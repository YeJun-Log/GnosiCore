#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFileSystemModel>
#include <QSortFilterProxyModel>
#include <QListWidgetItem>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

private slots:
    void on_treeView_clicked(const QModelIndex &index);
    void on_pushButton_clicked();
    void on_listWidgetKeywords_itemClicked(QListWidgetItem *item);

    void on_pushButton_2_clicked();

private:
    Ui::MainWindow *ui;
    QFileSystemModel *fileModel;
    QSortFilterProxyModel *proxyModel;
};
#endif // MAINWINDOW_H
