#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "codeeditor.h"
#include "dialog.h" // ⚠️如果关于对话框类名不叫 Dialog，这里要改
#include <QFileDialog>
#include <QMessageBox>
#include <QTextStream>
#include <QFile>
#include <QFontDialog>
#include <QColorDialog>
#include <QInputDialog>
#include <QFileInfo>
#include <QTextCharFormat>
#include <QToolBar>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // 1. 初始化状态栏
    m_labelLength = new QLabel("length: 0", this);
    m_labelLines = new QLabel("lines: 1", this);
    m_labelPos = new QLabel("Ln: 1, Col: 1", this);
    QLabel *labelName = new QLabel("范明灿", this); // ⚠️改这里！！！改成你的名字

    statusBar()->addWidget(m_labelLength);
    statusBar()->addPermanentWidget(m_labelLines);
    statusBar()->addPermanentWidget(m_labelPos);
    statusBar()->addPermanentWidget(labelName);

    // 2. 绑定基本编辑功能
    connect(ui->actionUndo, &QAction::triggered, ui->plainTextEdit, &QPlainTextEdit::undo);
    connect(ui->actionRedo, &QAction::triggered, ui->plainTextEdit, &QPlainTextEdit::redo);
    connect(ui->actionCut, &QAction::triggered, ui->plainTextEdit, &QPlainTextEdit::cut);
    connect(ui->actionCopy, &QAction::triggered, ui->plainTextEdit, &QPlainTextEdit::copy);
    connect(ui->actionPaste, &QAction::triggered, ui->plainTextEdit, &QPlainTextEdit::paste);
    connect(ui->actionSelectAll, &QAction::triggered, ui->plainTextEdit, &QPlainTextEdit::selectAll);

    // 3. 实时更新信号
    connect(ui->plainTextEdit, &QPlainTextEdit::cursorPositionChanged, this, &MainWindow::updateStatusBar);
    connect(ui->plainTextEdit, &QPlainTextEdit::textChanged, this, &MainWindow::onTextChanged);
    connect(ui->plainTextEdit, &QPlainTextEdit::copyAvailable, this, &MainWindow::setEditActions);

    // 初始化时让剪切和复制按钮置灰
    setEditActions(false);

    // 4. 显示/隐藏界面
    connect(ui->actionToolBar, &QAction::toggled, this, [=](bool checked){
        QToolBar *tb = findChild<QToolBar*>();
        if (tb) tb->setVisible(checked);
    });
    connect(ui->actionStatusBar, &QAction::toggled, this, [=](bool checked){
        statusBar()->setVisible(checked);
    });

    // 5. 自动换行
    connect(ui->actionWrap, &QAction::toggled, this, [=](bool checked){
        ui->plainTextEdit->setLineWrapMode(checked ? QPlainTextEdit::WidgetWidth : QPlainTextEdit::NoWrap);
    });

    setWindowTitle("无标题[*] - 记事本");
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::updateStatusBar()
{
    QTextCursor cursor = ui->plainTextEdit->textCursor();
    int line = cursor.blockNumber() + 1;
    int col = cursor.columnNumber() + 1;

    m_labelLength->setText(QString("length: %1").arg(ui->plainTextEdit->toPlainText().length()));
    m_labelLines->setText(QString("lines: %1").arg(ui->plainTextEdit->blockCount()));
    m_labelPos->setText(QString("Ln: %1, Col: %2").arg(line).arg(col));
}

void MainWindow::onTextChanged()
{
    setWindowModified(true);
    updateStatusBar();
}

void MainWindow::setEditActions(bool available)
{
    ui->actionCut->setEnabled(available);
    ui->actionCopy->setEnabled(available);
}

// ---------------- 文件操作 ----------------

bool MainWindow::maybeSave()
{
    if (!isWindowModified()) return true;

    auto ret = QMessageBox::warning(this, "提示", "文档已修改，是否保存？",
                                    QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
    if (ret == QMessageBox::Save) {
        on_actionSave_triggered();
        return !isWindowModified();
    } else if (ret == QMessageBox::Cancel) {
        return false;
    }
    return true;
}

void MainWindow::on_actionNew_triggered()
{
    if (maybeSave()) {
        ui->plainTextEdit->clear();
        m_currentFilePath.clear();
        setWindowModified(false);
        setWindowTitle("无标题[*] - 记事本");
    }
}

void MainWindow::on_actionOpen_triggered()
{
    if (!maybeSave()) return;

    QString fileName = QFileDialog::getOpenFileName(this, "打开文件", "", "文本文件 (*.txt);;所有文件 (*.*)");
    if (fileName.isEmpty()) return;

    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "错误", "无法打开文件！");
        return;
    }

    QTextStream in(&file);
    ui->plainTextEdit->setPlainText(in.readAll());
    m_currentFilePath = fileName;
    setWindowModified(false);
    setWindowTitle(QFileInfo(fileName).fileName() + "[*] - 记事本");
}

void MainWindow::on_actionSave_triggered()
{
    if (m_currentFilePath.isEmpty()) {
        on_actionSaveAs_triggered();
        return;
    }
    saveToFile(m_currentFilePath);
}

void MainWindow::on_actionSaveAs_triggered()
{
    QString fileName = QFileDialog::getSaveFileName(this, "另存为", "", "文本文件 (*.txt);;所有文件 (*.*)");
    if (fileName.isEmpty()) return;

    m_currentFilePath = fileName;
    saveToFile(fileName);
    setWindowTitle(QFileInfo(fileName).fileName() + "[*] - 记事本");
}

void MainWindow::saveToFile(const QString &fileName)
{
    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "错误", "无法保存文件！");
        return;
    }
    QTextStream out(&file);
    out << ui->plainTextEdit->toPlainText();
    file.close();
    setWindowModified(false);
}

void MainWindow::on_actionExit_triggered()
{
    if (maybeSave()) close();
}

// 拦截关闭事件（右上角 X 按钮）
void MainWindow::closeEvent(QCloseEvent *event)
{
    if (maybeSave()) {
        event->accept();
    } else {
        event->ignore();
    }
}

// ---------------- 格式操作 ----------------

void MainWindow::on_actionFont_triggered()
{
    bool ok;
    QFont font = QFontDialog::getFont(&ok, ui->plainTextEdit->font(), this, "选择字体");
    if (ok) ui->plainTextEdit->setFont(font);
}

void MainWindow::on_actionFontColor_triggered()
{
    QColor color = QColorDialog::getColor(Qt::black, this, "选择字体颜色");
    if (color.isValid()) {
        QTextCharFormat fmt;
        fmt.setForeground(color);
        ui->plainTextEdit->setCurrentCharFormat(fmt);

        QTextCursor cursor = ui->plainTextEdit->textCursor();
        if (cursor.hasSelection()) {
            cursor.mergeCharFormat(fmt);
        }
    }
}

void MainWindow::on_actionEditorBgColor_triggered()
{
    QColor color = QColorDialog::getColor(Qt::white, this, "选择背景色");
    if (color.isValid()) {
        QPalette pal = ui->plainTextEdit->palette();
        pal.setColor(QPalette::Base, color);
        ui->plainTextEdit->setPalette(pal);
    }
}

// ---------------- 行号显示 ----------------

void MainWindow::on_actionLineNumber_toggled(bool checked)
{
    CodeEditor *editor = qobject_cast<CodeEditor*>(ui->plainTextEdit);
    if (editor) {
        editor->setLineNumberVisible(checked);
    }
}

// ---------------- 查找替换 ----------------

void MainWindow::on_actionFind_triggered()
{
    bool ok;
    QString text = QInputDialog::getText(this, "查找", "查找目标:", QLineEdit::Normal, "", &ok);
    if (ok && !text.isEmpty()) {
        if (!ui->plainTextEdit->find(text)) {
            QTextCursor cursor = ui->plainTextEdit->textCursor();
            cursor.movePosition(QTextCursor::Start);
            ui->plainTextEdit->setTextCursor(cursor);
            if (!ui->plainTextEdit->find(text)) {
                QMessageBox::information(this, "提示", "未找到目标！");
            }
        }
    }
}

void MainWindow::on_actionReplace_triggered()
{
    bool ok;
    QString text = QInputDialog::getText(this, "替换", "查找目标:", QLineEdit::Normal, "", &ok);
    if (ok && !text.isEmpty()) {
        QString replaceText = QInputDialog::getText(this, "替换", "替换为:", QLineEdit::Normal, "", &ok);
        if (ok) {
            QTextCursor cursor = ui->plainTextEdit->textCursor();
            if (cursor.hasSelection() && cursor.selectedText() == text) {
                cursor.insertText(replaceText);
                ui->plainTextEdit->find(text);
            } else {
                if (ui->plainTextEdit->find(text)) {
                    QTextCursor findCursor = ui->plainTextEdit->textCursor();
                    findCursor.insertText(replaceText);
                } else {
                    QTextCursor resetCursor = ui->plainTextEdit->textCursor();
                    resetCursor.movePosition(QTextCursor::Start);
                    ui->plainTextEdit->setTextCursor(resetCursor);
                    if (ui->plainTextEdit->find(text)) {
                        QTextCursor findCursor = ui->plainTextEdit->textCursor();
                        findCursor.insertText(replaceText);
                    } else {
                        QMessageBox::information(this, "提示", "未找到目标！");
                    }
                }
            }
        }
    }
}

// 帮助 -> 关于 的槽函数
void MainWindow::on_actionAbout_triggered()
{
    Dialog dlg(this);
    dlg.exec();
}