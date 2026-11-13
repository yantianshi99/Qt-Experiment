#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLabel>
#include <QCloseEvent>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void closeEvent(QCloseEvent *event) override;

private slots:
    // 文件操作
    void on_actionNew_triggered();
    void on_actionOpen_triggered();
    void on_actionSave_triggered();
    void on_actionSaveAs_triggered();
    void on_actionExit_triggered();

    // 格式操作
    void on_actionFont_triggered();
    void on_actionFontColor_triggered();
    void on_actionEditorBgColor_triggered();

    // 查找替换
    void on_actionFind_triggered();
    void on_actionReplace_triggered();

    // 界面显示与帮助
    void on_actionLineNumber_toggled(bool checked);
    void on_actionAbout_triggered();

    // 自定义状态更新
    void updateStatusBar();
    void onTextChanged();
    void setEditActions(bool available);

private:
    Ui::MainWindow *ui;
    QString m_currentFilePath;

    QLabel *m_labelLength;
    QLabel *m_labelLines;
    QLabel *m_labelPos;

    bool maybeSave();
    void saveToFile(const QString &fileName);
};

#endif // MAINWINDOW_H