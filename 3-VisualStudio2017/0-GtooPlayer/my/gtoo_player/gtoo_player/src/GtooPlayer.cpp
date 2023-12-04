#include <QDebug>
#include <QFileDialog>
#include <QDateTime>
#include <QTextCodec>


#include "GtooPlayer.h"
#include "About.h"
extern "C"
{
#include "libavcodec/avcodec.h"
}

///由于我们建立的是C++的工程
///编译的时候使用的C++的编译器编译
///而FFMPEG是C的库
///因此这里需要加上extern "C"
///否则会提示各种未定义

void test(void) {
    printf("%s\n", avcodec_configuration());
}


GtooPlayer::GtooPlayer(QWidget *parent):
    ui(new Ui::GtooPlayerClass),
    QMainWindow(parent)
{    
    //QTextCodec::setCodecForLocale(QTextCodec::codecForName("UTF-8")); // 没有用
    qDebug() << QString::fromLocal8Bit("中文");

    ui->setupUi(this);
    initUi();
    initConnect();
}

void GtooPlayer::initUi(void) {
    openSecond1 = new QPushButton(QString::fromLocal8Bit("关于"));
    //openSecond1 = new QPushButton("about1");
    ui->ToolBar->addWidget(openSecond1);

    openFileButton = new QPushButton("open file");
    ui->ToolBar->addWidget(openFileButton);

}

void GtooPlayer::initConnect(void) {
    connect(ui->pushButton, &QPushButton::clicked, this, &test);

    connect(openSecond1, &QPushButton::clicked, this, &GtooPlayer::openAbout);
    connect(openFileButton, &QPushButton::clicked, this, &GtooPlayer::openFile);
}

void GtooPlayer::openAbout(void) {
    About* aboutWindow = new About(this);
    aboutWindow->show();
}

void GtooPlayer::openFile(void) {
    QString filePath = QFileDialog::getOpenFileName(this, "test", "E:\\Desktop\\tool\\player_test\\test_video");
    QFileInfo info(filePath);
    qDebug() << filePath;

    //is = stream_open(filePath);
    av_log(NULL, AV_LOG_INFO, "Failed to initialize VideoState!\n");
}


GtooPlayer::~GtooPlayer()
{}
