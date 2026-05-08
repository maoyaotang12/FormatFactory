#include "mainwindow.h"
#include <opencv2/opencv.hpp>
using namespace cv;

#include <QString>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QSplitter>
#include <QFormLayout>
#include <QAction>
#include <QFileDialog>
#include <QMessageBox>
#include <QProcess>
#include <QMenu>
#include <QDir>
#include <QDesktopServices>
#include <QUrl>
#include <QLabel>
#include <QProgressBar>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QTimer>
#include <QGraphicsOpacityEffect>
#include <QTime>
#include <QGraphicsBlurEffect>
#include <QGraphicsColorizeEffect>
#include <QAudioOutput>
#include <QSvgRenderer>
#include <QPainter>
#include <QBuffer>
#include <QFileInfo>

void addImageTextWatermark(cv::Mat &img, QString text, int fontSize, cv::Scalar color, int posIdx)
{
    if (img.empty() || text.isEmpty())
        return;

    int fontFace = cv::FONT_HERSHEY_SIMPLEX;
    int thickness = 2;
    double scale = fontSize / 28.0;

    cv::Size textSize = cv::getTextSize(text.toStdString(), fontFace, scale, thickness, nullptr);
    int tw = textSize.width;
    int th = textSize.height;

    int mar = 15;
    int x, y;

    switch (posIdx) {
        case 0: x = mar;              y = th + mar;        break;
        case 1: x = img.cols - tw - mar; y = th + mar;    break;
        case 2: x = mar;              y = img.rows - mar;  break;
        case 3: x = img.cols - tw - mar; y = img.rows - mar; break;
        default:x = (img.cols - tw)/2; y = (img.rows + th)/2; break;
    }

    cv::putText(img, text.toStdString(), cv::Point(x, y),
                fontFace, scale, color, thickness);
}

MainWindow::MainWindow(QWidget *parent)
: QMainWindow(parent)
{
    setWindowTitle("万能格式转换器Linux");
    resize(1000, 750);
    initUI();
    initLeftMenu();

    m_fileMenu = new QMenu(this);
    m_selectAll = new QAction("全选", this);
    m_clearSelect = new QAction("清空选择", this);

    m_fileMenu->addAction(m_selectAll);
    m_fileMenu->addAction(m_clearSelect);

    connect(m_selectAll, &QAction::triggered, this, [this]() {
        m_fileList->selectAll();
    });
    connect(m_clearSelect, &QAction::triggered, this, [this]() {
        m_fileList->clearSelection();
    });

    m_fileList->setSelectionMode(QAbstractItemView::ExtendedSelection);
    m_fileList->setStyleSheet(R"(
    QListWidget::item:selected {
        background-color: #409eff;
        color: white;
    }
    )");
}

void MainWindow::initUI()
{
    setWindowIcon(QIcon::fromTheme("FormatFactory"));

    m_fontSimplify = 1.0;
    m_fontCompress = 6;
    m_fontEmUnit = 1000;
    m_fontKeepMeta = true;
    m_fontFixOutline = true;
    m_fontAsciiOnly = false;

    QSplitter *splitter = new QSplitter(Qt::Horizontal);
    m_leftTree = new QTreeWidget;
    m_leftTree->setFixedWidth(190);
    m_leftTree->setHeaderHidden(true);

    QWidget *rightPanel = new QWidget;
    QVBoxLayout *mainLayout = new QVBoxLayout(rightPanel);
    mainLayout->setContentsMargins(8,8,8,8);
    mainLayout->setSpacing(10);

    grpAdv = new QGroupBox("视频高级配置");
    QVBoxLayout *advLayout = new QVBoxLayout(grpAdv);
    advLayout->setContentsMargins(12,12,12,12);
    advLayout->setSpacing(10);

    int labelCtrlSpace = 0;
    int itemSpacing    = 8;

    QHBoxLayout *row1 = new QHBoxLayout;
    row1->setContentsMargins(0,0,0,0);

    cmbResolution = new QComboBox;
    cmbResolution->addItems({"原分辨率","360p","480p","720p","1080p","2K","4K"});
    cmbResolution->setFixedWidth(85);
    row1->addWidget(new QLabel("分辨率"));
    row1->addSpacing(labelCtrlSpace);
    row1->addWidget(cmbResolution);
    row1->addSpacing(itemSpacing);

    cmbFPS = new QComboBox;
    cmbFPS->setEditable(true);
    cmbFPS->addItems({"原帧率","24","30","60","90","120","144"});
    cmbFPS->setFixedWidth(75);
    row1->addWidget(new QLabel("帧率"));
    row1->addSpacing(labelCtrlSpace);
    row1->addWidget(cmbFPS);
    row1->addSpacing(itemSpacing);

    cmbCodec = new QComboBox;
    cmbCodec->addItems({"默认","H.264","H.265","MPEG-4","VP9","Theora"});
    cmbCodec->setFixedWidth(85);
    row1->addWidget(new QLabel("编码"));
    row1->addSpacing(labelCtrlSpace);
    row1->addWidget(cmbCodec);
    row1->addSpacing(itemSpacing);

    cmbPreset = new QComboBox;
    cmbPreset->addItems({
        "自定义",
        "抖音短视频",
        "B站超清",
        "高质量小体积",
        "录屏专用",
        "动漫专用",
        "电影压制",
        "超快压缩"
    });
    cmbPreset->setFixedWidth(100);
    row1->addWidget(new QLabel("快速预设"));
    row1->addSpacing(labelCtrlSpace);
    row1->addWidget(cmbPreset);

    row1->addStretch();

    QHBoxLayout *row2 = new QHBoxLayout;
    row2->setContentsMargins(0,0,0,0);

    // ==============================
    // 视频合并模式（独立控件）
    // ==============================
    cmbMergeMode = new QComboBox;  // 👈 用新变量，不覆盖
    cmbMergeMode->addItems({
        "默认合并(无损快切)",
                           "兼容合并(重新编码)",
                           "仅合并视频(无声)",
                           "合并转MP4(标准)"
    });
    cmbMergeMode->setFixedWidth(130);
    row2->addWidget(new QLabel("视频合并"));
    row2->addSpacing(labelCtrlSpace);
    row2->addWidget(cmbMergeMode);
    row2->addSpacing(itemSpacing);

    cmbFrameMode = new QComboBox;  // 👈 用新变量，不覆盖
    cmbFrameMode->addItems({
        "每秒1帧",
        "每秒5帧",
        "每秒10帧",
        "每秒20帧",
        "每秒30帧",
        "每5秒1帧",
        "每10秒1帧",
        "仅关键帧(I帧)"
    });
    cmbFrameMode->setFixedWidth(100);
    row2->addWidget(new QLabel("提取视频帧"));
    row2->addSpacing(labelCtrlSpace);
    row2->addWidget(cmbFrameMode);
    row2->addSpacing(itemSpacing);

    cmbFilterPreset = new QComboBox;
    cmbFilterPreset->addItems({
        "无预设","锐化","模糊","黑白","复古","水平翻转","垂直翻转",
        "亮度+50","对比度+30","高斯模糊","镜像效果","灰度反转"
    });
    cmbFilterPreset->setFixedWidth(100);
    row2->addWidget(new QLabel("滤镜预设"));
    row2->addSpacing(labelCtrlSpace);
    row2->addWidget(cmbFilterPreset);
    row2->addStretch();

    QHBoxLayout *row3 = new QHBoxLayout;
    row3->setContentsMargins(0,0,0,0);

    cmbWatermark = new QComboBox(this);
    cmbWatermark->addItems({"无水印","自定义水印"});
    cmbWatermark->setFixedWidth(85);
    row3->addWidget(new QLabel("水印"));
    row3->addSpacing(labelCtrlSpace);
    row3->addWidget(cmbWatermark);
    row3->addSpacing(itemSpacing);

    editWaterText = new QLineEdit(this);
    editWaterText->setPlaceholderText("水印文字");
    editWaterText->setFixedWidth(85);
    row3->addWidget(editWaterText);
    row3->addSpacing(itemSpacing);

    cmbWaterSize = new QComboBox(this);
    cmbWaterSize->addItems({"16","20","24","28","32","36","40","48","60","80","100","120","140","160","180","200"});
    cmbWaterSize->setCurrentText("24");
    cmbWaterSize->setFixedWidth(50);
    row3->addWidget(new QLabel("大小"));
    row3->addSpacing(labelCtrlSpace);
    row3->addWidget(cmbWaterSize);
    row3->addSpacing(itemSpacing);

    cmbWaterColor = new QComboBox(this);
    cmbWaterColor->addItems({"白色","黄色","红色","黑色","蓝色","绿色"});
    cmbWaterColor->setCurrentText("白色");
    cmbWaterColor->setFixedWidth(60);
    row3->addWidget(new QLabel("颜色"));
    row3->addSpacing(labelCtrlSpace);
    row3->addWidget(cmbWaterColor);
    row3->addSpacing(itemSpacing);

    cmbWaterPos = new QComboBox(this);
    cmbWaterPos->addItems({"左上角","右上角","左下角","右下角","居中"});
    cmbWaterPos->setCurrentText("右下角");
    cmbWaterPos->setFixedWidth(80);
    row3->addWidget(new QLabel("位置"));
    row3->addSpacing(labelCtrlSpace);
    row3->addWidget(cmbWaterPos);
    row3->addStretch();

    QHBoxLayout *row4 = new QHBoxLayout;
    row4->setContentsMargins(0,0,0,0);

    editStartTime = new QLineEdit("00:00:00");
    editEndTime = new QLineEdit("00:00:00");
    editStartTime->setFixedWidth(110);
    editEndTime->setFixedWidth(110);

    row4->addWidget(new QLabel("开始时间"));
    row4->addSpacing(labelCtrlSpace);
    row4->addWidget(editStartTime);
    row4->addSpacing(itemSpacing);

    row4->addWidget(new QLabel("结束时间"));
    row4->addSpacing(labelCtrlSpace);
    row4->addWidget(editEndTime);
    row4->addSpacing(itemSpacing);

    QPushButton *btnPlayIn = new QPushButton("▶ 实时预览");
    btnPlayIn->setFixedWidth(100);
    row4->addWidget(btnPlayIn);
    row4->addStretch();

    QHBoxLayout *row5 = new QHBoxLayout;
    row5->setContentsMargins(0,0,0,0);

    sliderTrim = new QSlider(Qt::Horizontal);
    sliderTrim->setRange(0, 100);
    sliderTrim->setFixedWidth(100);

    cmbQuality = new QComboBox;
    cmbQuality->addItems({"低画质", "中等画质", "高画质", "超清", "无损画质"});
    cmbQuality->setCurrentIndex(3);
    cmbQuality->setFixedWidth(90);

    // ===================== 新增：字幕下拉框 =====================
    cmbSubtitle = new QComboBox;       // 字幕下拉框
    cmbSubtitle->addItems({"无字幕", "崁入字幕", "提取字幕", "删除字幕"}); // 3个常用选项
    cmbSubtitle->setCurrentIndex(0);    // 默认无字幕
    cmbSubtitle->setFixedWidth(100);     // 和画质一样宽，整齐好看

    // 同一行：进度条 + 画质等级 + 字幕
    row5->addWidget(new QLabel("进度条"));
    row5->addWidget(sliderTrim);
    row5->addSpacing(10);
    row5->addWidget(new QLabel("画质等级"));
    row5->addWidget(cmbQuality);
    row5->addSpacing(15);             // 画质和字幕之间留一点空隙，不拥挤
    row5->addWidget(new QLabel("字幕"));  // 字幕文字
    row5->addWidget(cmbSubtitle);         // 字幕下拉框
    row5->addStretch();

    // 左边：你的 5 行控件 完全不动
    QVBoxLayout *leftAll = new QVBoxLayout;
    leftAll->setContentsMargins(0,0,0,0);
    leftAll->setSpacing(6);

    leftAll->addLayout(row1);
    leftAll->addLayout(row2);
    leftAll->addLayout(row3);
    leftAll->addLayout(row4);
    leftAll->addLayout(row5);

    // ===================== 右边播放器：宽度不变，竖直占满 5 行 =====================
    QGraphicsView *videoView = new QGraphicsView(grpAdv);
    videoView->setFixedWidth(340);        // 宽度不变
    videoView->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding); // 高度自动占满五行
    videoView->setStyleSheet("background:transparent; border:none;");
    videoView->setFrameStyle(QFrame::NoFrame);
    videoView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    videoView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    // 删掉了 move(600,80)  👍

    QGraphicsScene *scene = new QGraphicsScene(this);
    videoView->setScene(scene);

    videoItem = new QGraphicsVideoItem;
    scene->addItem(videoItem);
    videoItem->setAspectRatioMode(Qt::IgnoreAspectRatio);
    videoItem->setSize(QSizeF(340, 240));

    // =========================
    // 只加一个居中播放图标
    // =========================
    QGraphicsTextItem *playIcon = new QGraphicsTextItem("▶");
    playIcon->setDefaultTextColor(QColor("#999999")); // 浅灰色图标
    QFont f = playIcon->font();
    f.setPointSize(36);
    playIcon->setFont(f);
    playIcon->setPos(150, 80); // 居中
    scene->addItem(playIcon);

    // 左右合并：左边5行 + 右边播放器
    QHBoxLayout *finalLayout = new QHBoxLayout;
    finalLayout->setContentsMargins(0,0,0,0);
    finalLayout->setSpacing(4);
    finalLayout->addLayout(leftAll);
    finalLayout->addWidget(videoView);

    // 把最终布局设置到 advLayout
    advLayout->addLayout(finalLayout);

    m_player = new QMediaPlayer(this);
    QAudioOutput *videoAudio = new QAudioOutput(this);
    m_player->setAudioOutput(videoAudio);
    m_player->setVideoOutput(videoItem);

    connect(btnPlayIn, &QPushButton::clicked, this, [this, playIcon]() {
        updateVideoPreview();
        playIcon->setVisible(false);
    });

    connect(editStartTime, &QLineEdit::editingFinished, this, [this, playIcon]() {
        updateVideoPreview();
        playIcon->setVisible(false);
    });
    connect(editEndTime, &QLineEdit::editingFinished, this, [this, playIcon]() {
        updateVideoPreview();
        playIcon->setVisible(false);
    });

    connect(cmbFilterPreset, &QComboBox::currentTextChanged, this, [this]() {
        applyVideoFilter();
    });

    panelAudio = new QGroupBox("音频高级配置");
    QVBoxLayout *lyAudio = new QVBoxLayout(panelAudio);
    lyAudio->setContentsMargins(12,12,12,12);
    lyAudio->setSpacing(10);

    cmbAudioCodec = new QComboBox;
    cmbAudioCodec->addItems({"AAC","MP3","FLAC无损","WAV无损","Opus","AC3杜比"});
    cmbAudioCodec->setFixedWidth(90);

    cmbSampleRate = new QComboBox;
    cmbSampleRate->addItems({"22050","32000","44100","48000","96000"});
    cmbSampleRate->setFixedWidth(80);

    cmbBitrate = new QComboBox;
    cmbBitrate->addItems({"64k","96k","128k","192k","256k","320k","500k","1000k"});
    cmbBitrate->setFixedWidth(75);

    cmbAudioChannel = new QComboBox;
    cmbAudioChannel->addItems({"单声道","立体声","5.1环绕"});
    cmbAudioChannel->setFixedWidth(85);

    cmbAudioFilter = new QComboBox;
    cmbAudioFilter->addItems({"无音效","降噪","低音增强","人声增强","消人声","淡入淡出"});
    cmbAudioFilter->setFixedWidth(110);

    cmbAudioPreset = new QComboBox;
    cmbAudioPreset->addItems({"自定义","抖音","标准音乐","无损","录音","极速压缩"});
    cmbAudioPreset->setFixedWidth(110);

    editAudioStart = new QLineEdit("00:00:00");
    editAudioEnd   = new QLineEdit("00:00:00");
    editAudioStart->setFixedWidth(110);
    editAudioEnd->setFixedWidth(110);

    btnAudioPlay = new QPushButton("播放音频");
    audioSlider = new QSlider(Qt::Horizontal);
    lblAudioTime = new QLabel("00:00 / 00:00");
    audioSlider->setMinimumWidth(220);

    QHBoxLayout *audioRow1 = new QHBoxLayout;
    audioRow1->setSpacing(8);
    audioRow1->addWidget(new QLabel("编码"));
    audioRow1->addWidget(cmbAudioCodec);
    audioRow1->addSpacing(6);
    audioRow1->addWidget(new QLabel("采样率"));
    audioRow1->addWidget(cmbSampleRate);
    audioRow1->addSpacing(6);
    audioRow1->addWidget(new QLabel("比特率"));
    audioRow1->addWidget(cmbBitrate);
    audioRow1->addSpacing(6);
    audioRow1->addWidget(new QLabel("声道"));
    audioRow1->addWidget(cmbAudioChannel);
    audioRow1->addSpacing(6);
    audioRow1->addWidget(new QLabel("音效"));
    audioRow1->addWidget(cmbAudioFilter);
    audioRow1->addSpacing(6);
    audioRow1->addWidget(new QLabel("快速预设"));
    audioRow1->addWidget(cmbAudioPreset);
    audioRow1->addStretch();

    QHBoxLayout *audioRow2 = new QHBoxLayout;
    audioRow2->setSpacing(8);
    audioRow2->addWidget(new QLabel("开始"));
    audioRow2->addWidget(editAudioStart);
    audioRow2->addSpacing(10);
    audioRow2->addWidget(new QLabel("结束"));
    audioRow2->addWidget(editAudioEnd);
    audioRow2->addSpacing(15);
    audioRow2->addWidget(btnAudioPlay);
    audioRow2->addWidget(audioSlider);
    audioRow2->addWidget(lblAudioTime);
    audioRow2->addStretch();

    lyAudio->addLayout(audioRow1);
    lyAudio->addLayout(audioRow2);
    mainLayout->addWidget(panelAudio);

    panelImage = new QGroupBox("图片转换配置");
    QVBoxLayout *lyImage = new QVBoxLayout(panelImage);
    lyImage->setContentsMargins(12,12,12,12);
    lyImage->setSpacing(10);

    QHBoxLayout *imgRow1 = new QHBoxLayout;
    imgRow1->setSpacing(8);

    cmbImageFormat = new QComboBox;
    cmbImageFormat->addItems({"JPG","PNG","WebP","BMP","GIF","TIFF","JPEG","TIF","PPG","PGM","PBM","JP2"});
    cmbImageFormat->setFixedWidth(90);
    imgRow1->addWidget(new QLabel("输出格式"));
    imgRow1->addWidget(cmbImageFormat);
    imgRow1->addSpacing(10);

    editImgWidth = new QLineEdit;
    editImgWidth->setPlaceholderText("宽度");
    editImgWidth->setFixedWidth(70);
    imgRow1->addWidget(new QLabel("宽"));
    imgRow1->addWidget(editImgWidth);
    imgRow1->addSpacing(10);

    editImgHeight = new QLineEdit;
    editImgHeight->setPlaceholderText("高度");
    editImgHeight->setFixedWidth(70);
    imgRow1->addWidget(new QLabel("高"));
    imgRow1->addWidget(editImgHeight);
    imgRow1->addSpacing(10);

    cmbImageQuality = new QComboBox;
    cmbImageQuality->addItems({"低画质","中等","高画质","无损"});
    cmbImageQuality->setCurrentIndex(2);
    cmbImageQuality->setFixedWidth(90);
    imgRow1->addWidget(new QLabel("画质"));
    imgRow1->addWidget(cmbImageQuality);
    imgRow1->addSpacing(10);

    cmbRotate = new QComboBox;
    cmbRotate->addItems({"不旋转","90°","180°","270°"});
    cmbRotate->setFixedWidth(90);
    imgRow1->addWidget(new QLabel("旋转"));
    imgRow1->addWidget(cmbRotate);
    imgRow1->addSpacing(10);

    cmbFlip = new QComboBox;
    cmbFlip->addItems({"不翻转","水平","垂直"});
    cmbFlip->setFixedWidth(90);
    imgRow1->addWidget(new QLabel("翻转"));
    imgRow1->addWidget(cmbFlip);
    imgRow1->addStretch();

    QHBoxLayout *imgRow2 = new QHBoxLayout;
    imgRow2->setSpacing(8);

    cmbImagePreset = new QComboBox;
    cmbImagePreset->addItems({"自定义","微信图","高清图","网页小图","超高压缩","无损PNG"});
    cmbImagePreset->setFixedWidth(120);
    imgRow2->addWidget(new QLabel("快速预设"));
    imgRow2->addWidget(cmbImagePreset);
    imgRow2->addSpacing(10);

    cmbImageEffect = new QComboBox;
    cmbImageEffect->addItems({
        "无特效",
        "黑白",
        "灰度",
        "反色",
        "锐化",
        "轻度模糊",
        "高斯模糊",
        "复古",
        "亮度+10%",
        "亮度+20%",
        "对比度+20%"
    });
    cmbImageEffect->setFixedWidth(120);
    imgRow2->addWidget(new QLabel("图片特效"));
    imgRow2->addWidget(cmbImageEffect);
    imgRow2->addSpacing(10);

    cmbImageWatermark = new QComboBox;
    cmbImageWatermark->addItems({"无水印","自定义水印"});
    cmbImageWatermark->setFixedWidth(100);
    imgRow2->addWidget(new QLabel("水印"));
    imgRow2->addWidget(cmbImageWatermark);
    imgRow2->addSpacing(10);

    editImageWaterText = new QLineEdit;
    editImageWaterText->setPlaceholderText("水印文字");
    editImageWaterText->setFixedWidth(90);
    imgRow2->addWidget(editImageWaterText);
    imgRow2->addSpacing(10);

    cmbImageWaterSize = new QComboBox;
    cmbImageWaterSize->addItems({"16","20","24","28","32","36","40","48","60","80","100","120","140","160","180","200"});
    cmbImageWaterSize->setCurrentText("24");
    cmbImageWaterSize->setFixedWidth(60);
    imgRow2->addWidget(cmbImageWaterSize);
    imgRow2->addSpacing(10);

    cmbImageWaterColor = new QComboBox;
    cmbImageWaterColor->addItems({"白色","黄色","红色","黑色","蓝色","绿色"});
    cmbImageWaterColor->setCurrentText("白色");
    cmbImageWaterColor->setFixedWidth(70);
    imgRow2->addWidget(cmbImageWaterColor);
    imgRow2->addSpacing(10);

    cmbImageWaterPos = new QComboBox;
    cmbImageWaterPos->addItems({"左上角","右上角","左下角","右下角","居中"});
    cmbImageWaterPos->setCurrentText("右下角");
    cmbImageWaterPos->setFixedWidth(90);
    imgRow2->addWidget(cmbImageWaterPos);
    imgRow2->addStretch();

    lyImage->addLayout(imgRow1);
    lyImage->addLayout(imgRow2);
    mainLayout->addWidget(panelImage);

    connect(cmbImagePreset, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::applyImagePreset);
    connect(cmbImageFormat, &QComboBox::currentTextChanged, this, [this](const QString &text){
        m_outSuffix = text.toLower();
    });

    panelFont = new QGroupBox("字体转换设置", this);
    QVBoxLayout *lyFont = new QVBoxLayout(panelFont);
    lyFont->setContentsMargins(14,14,14,14);
    lyFont->setSpacing(10);

    QHBoxLayout *fontRow1 = new QHBoxLayout;
    fontInputPath = new QLineEdit(this);
    QPushButton *btnFontSelect = new QPushButton("选择文件", this);
    fontRow1->addWidget(new QLabel("输入文件:"));
    fontRow1->addWidget(fontInputPath);
    fontRow1->addWidget(btnFontSelect);

    QHBoxLayout *fontRow2 = new QHBoxLayout;
    fontOutputPath = new QLineEdit(this);
    QPushButton *btnFontOut = new QPushButton("输出目录", this);
    fontRow2->addWidget(new QLabel("输出目录:"));
    fontRow2->addWidget(fontOutputPath);
    fontRow2->addWidget(btnFontOut);

    QHBoxLayout *fontRow3 = new QHBoxLayout;
    fontFormatCombo = new QComboBox(this);
    fontFormatCombo->addItems({"TTF", "OTF", "WOFF", "WOFF2"});
    fontRow3->addWidget(new QLabel("输出格式:"));
    fontRow3->addWidget(fontFormatCombo);
    fontRow3->addStretch();

    QPushButton *btnFontStart = new QPushButton("开始转换字体", this);

    cboFontPreset = new QComboBox(this);
    cboFontPreset->addItems({
        "自定义",
        "标准转换",
        "网页最小体积",
        "印刷高质量",
        "仅英文精简"
    });
    cboFontPreset->setFixedWidth(120);

    QHBoxLayout *fontPresetLine = new QHBoxLayout;
    fontPresetLine->addWidget(new QLabel("快速预设"));
    fontPresetLine->addWidget(cboFontPreset);
    fontPresetLine->addStretch();

    QHBoxLayout *fontLine1 = new QHBoxLayout;
    fontLine1->setSpacing(8);

    spinFontSimplify = new QDoubleSpinBox(this);
    spinFontSimplify->setRange(0.1, 5.0);
    spinFontSimplify->setValue(1.0);
    spinFontSimplify->setFixedWidth(80);

    spinFontCompress = new QSpinBox(this);
    spinFontCompress->setRange(1,9);
    spinFontCompress->setValue(6);
    spinFontCompress->setFixedWidth(70);

    spinFontEm = new QSpinBox(this);
    spinFontEm->setRange(500, 4096);
    spinFontEm->setValue(1000);
    spinFontEm->setFixedWidth(80);

    fontLine1->addWidget(new QLabel("轮廓简化"));
    fontLine1->addWidget(spinFontSimplify);
    fontLine1->addWidget(new QLabel("压缩"));
    fontLine1->addWidget(spinFontCompress);
    fontLine1->addWidget(new QLabel("EM大小"));
    fontLine1->addWidget(spinFontEm);
    fontLine1->addStretch();

    QHBoxLayout *fontLine2 = new QHBoxLayout;
    chkFontMeta = new QCheckBox("保留版权", this);
    chkFontMeta->setChecked(true);
    chkFontFix = new QCheckBox("修复轮廓", this);
    chkFontFix->setChecked(true);
    chkFontAscii = new QCheckBox("仅英文", this);
    fontLine2->addWidget(chkFontMeta);
    fontLine2->addWidget(chkFontFix);
    fontLine2->addWidget(chkFontAscii);
    fontLine2->addStretch();

    lyFont->addLayout(fontRow1);
    lyFont->addLayout(fontRow2);
    lyFont->addLayout(fontRow3);
    lyFont->addWidget(btnFontStart);
    lyFont->addSpacing(10);
    lyFont->addLayout(fontPresetLine);
    lyFont->addLayout(fontLine1);
    lyFont->addLayout(fontLine2);

    mainLayout->addWidget(panelFont);
    panelFont->hide();

    connect(btnFontSelect, &QPushButton::clicked, this, [this](){
        QString f = QFileDialog::getOpenFileName(this, "选择字体", "", "Font (*.ttf *.otf *.woff *.woff2)");
        if (!f.isEmpty()) fontInputPath->setText(f);
    });
        connect(btnFontOut, &QPushButton::clicked, this, [this](){
            QString d = QFileDialog::getExistingDirectory(this, "选择输出目录");
            if (!d.isEmpty()) fontOutputPath->setText(d);
        });
            connect(btnFontStart, &QPushButton::clicked, this, &MainWindow::startFontConvert);

            connect(spinFontSimplify, &QDoubleSpinBox::valueChanged, this, [this](double v){ m_fontSimplify = v; });
            connect(spinFontCompress, &QSpinBox::valueChanged, this, [this](int v){ m_fontCompress = v; });
            connect(spinFontEm, &QSpinBox::valueChanged, this, [this](int v){ m_fontEmUnit = v; });
            connect(chkFontMeta, &QCheckBox::toggled, this, [this](bool v){ m_fontKeepMeta = v; });
            connect(chkFontFix, &QCheckBox::toggled, this, [this](bool v){ m_fontFixOutline = v; });
            connect(chkFontAscii, &QCheckBox::toggled, this, [this](bool v){ m_fontAsciiOnly = v; });

            connect(cboFontPreset, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int idx){
                if (idx == 1) {
                    m_fontSimplify = 1.0; m_fontCompress = 6; m_fontEmUnit = 1000;
                    m_fontKeepMeta = true; m_fontFixOutline = true; m_fontAsciiOnly = false;
                } else if (idx == 2) {
                    m_fontSimplify = 2.0; m_fontCompress = 9; m_fontEmUnit = 1000;
                    m_fontKeepMeta = false; m_fontFixOutline = true; m_fontAsciiOnly = false;
                } else if (idx == 3) {
                    m_fontSimplify = 0.5; m_fontCompress = 6; m_fontEmUnit = 2048;
                    m_fontKeepMeta = true; m_fontFixOutline = true; m_fontAsciiOnly = false;
                } else if (idx == 4) {
                    m_fontSimplify = 1.5; m_fontCompress = 9; m_fontEmUnit = 1000;
                    m_fontKeepMeta = true; m_fontFixOutline = true; m_fontAsciiOnly = true;
                }
                spinFontSimplify->setValue(m_fontSimplify);
                spinFontCompress->setValue(m_fontCompress);
                spinFontEm->setValue(m_fontEmUnit);
                chkFontMeta->setChecked(m_fontKeepMeta);
                chkFontFix->setChecked(m_fontFixOutline);
                chkFontAscii->setChecked(m_fontAsciiOnly);
            });

            panelAbout = new QGroupBox("关于软件", this);
            QVBoxLayout *aboutLayout = new QVBoxLayout(panelAbout);
            aboutLayout->setAlignment(Qt::AlignCenter);
            aboutLayout->setSpacing(25);
            aboutLayout->setContentsMargins(40, 40, 40, 40);

            panelAbout->setStyleSheet(R"(
    QGroupBox::title {
        font-size: 22px;
        font-weight: bold;
        subcontrol-origin: margin;
        subcontrol-position: top center;
        padding: 10px;
    }
            )");

            QLabel *labTitle = new QLabel("万能格式转换器");
            labTitle->setAlignment(Qt::AlignCenter);
            labTitle->setStyleSheet("font-size: 30px; font-weight: bold;");
            aboutLayout->addWidget(labTitle);

            QLabel *labVer = new QLabel("版本：v1.0.1");
            labVer->setAlignment(Qt::AlignCenter);
            labVer->setStyleSheet("font-size: 20px;");
            aboutLayout->addWidget(labVer);

            QLabel *labAuthor = new QLabel("作者：Maoyaotang");
            labAuthor->setAlignment(Qt::AlignCenter);
            labAuthor->setStyleSheet("font-size: 20px;");
            aboutLayout->addWidget(labAuthor);

            QLabel *labEngine = new QLabel("基于：FFmpeg + Qt + OpenCV + FontForge");
            labEngine->setAlignment(Qt::AlignCenter);
            labEngine->setStyleSheet("font-size: 18px;");
            aboutLayout->addWidget(labEngine);

            QLabel *link = new QLabel("访问 GitHub 主页");
            link->setAlignment(Qt::AlignCenter);
            link->setStyleSheet("font-size: 20px;");
            link->setCursor(Qt::PointingHandCursor);
            link->setText("<a href=\"#\">访问 GitHub 主页</a>");
            link->setOpenExternalLinks(false);
            connect(link, &QLabel::linkActivated, this, [this](const QString&) {
                QDesktopServices::openUrl(QUrl("https://github.com/maoyaotang12"));
            });
            aboutLayout->addWidget(link);
            aboutLayout->addStretch();
            mainLayout->addWidget(panelAbout);

            audioPlayer = new QMediaPlayer(this);
            audioOutput = new QAudioOutput(this);
            audioPlayer->setAudioOutput(audioOutput);

            connect(audioPlayer, &QMediaPlayer::positionChanged, this, [this](qint64 pos){
                audioSlider->setValue(pos);
                QString curr = QTime(0,0,0).addMSecs(pos).toString("mm:ss");
                QString total = QTime(0,0,0).addMSecs(audioPlayer->duration()).toString("mm:ss");
                lblAudioTime->setText(curr + " / " + total);

                QTime et = QTime::fromString(editAudioEnd->text(), "hh:mm:ss");
                qint64 endMs = et.hour()*3600000 + et.minute()*60000 + et.second()*1000;
                if (pos >= endMs && endMs > 0) audioPlayer->stop();
            });

                connect(audioPlayer, &QMediaPlayer::durationChanged, audioSlider, &QSlider::setMaximum);
                connect(audioSlider, &QSlider::sliderMoved, audioPlayer, &QMediaPlayer::setPosition);
                connect(btnAudioPlay, &QPushButton::clicked, this, &MainWindow::playAudioFile);

                QHBoxLayout *btnLayout = new QHBoxLayout;
                btnLayout->setSpacing(8);
                btnLayout->setContentsMargins(0,5,0,5);

                m_btnAdd = new QPushButton("添加文件");
                m_btnDelete = new QPushButton("删除选中");
                m_btnClear = new QPushButton("清空列表");
                m_btnStart = new QPushButton("执行任务", this);
                // 下拉菜单（修复版！不报错！）
                QMenu* funcMenu = new QMenu(this);
                QAction* actVideo   = funcMenu->addAction("视频转换");
                QAction* actAudio   = funcMenu->addAction("音频转换");
                QAction* actMerge   = funcMenu->addAction("视频合并");
                QAction* actExtract = funcMenu->addAction("提取视频帧");
                QAction* actImage   = funcMenu->addAction("图片转换");

                m_btnStart->setMenu(funcMenu);

                // 👇👇👇 全部修复，无报错、无警告
                connect(actVideo, &QAction::triggered, this, [this]() {
                    startBatchConvert("视频转换");
                });
                connect(actAudio, &QAction::triggered, this, [this]() {
                    startBatchConvert("音频转换");
                });
                connect(actMerge, &QAction::triggered, this, [this]() {
                    startBatchConvert("视频合并");
                });
                connect(actExtract, &QAction::triggered, this, [this]() {
                    startBatchConvert("提取视频帧");
                });
                connect(actImage, &QAction::triggered, this, [this]() {
                    startBatchConvert("图片转换");
                });
                m_btnPlay = new QPushButton("播放选中");

                btnLayout->addWidget(m_btnAdd, 1);
                btnLayout->addWidget(m_btnDelete, 1);
                btnLayout->addWidget(m_btnClear, 1);
                btnLayout->addWidget(m_btnStart, 1);
                btnLayout->addWidget(m_btnPlay, 1);

                m_fileList = new QListWidget;
                m_fileList->setContextMenuPolicy(Qt::NoContextMenu);

                QHBoxLayout *outLayout = new QHBoxLayout;
                m_editOutPath = new QLineEdit(QDir::homePath() + "/FormatFactory");
                QPushButton *btnOutPath = new QPushButton("输出目录");
                outLayout->addWidget(m_editOutPath);
                outLayout->addWidget(btnOutPath);

                m_progress = new QProgressBar;

                mainLayout->addWidget(grpAdv);
                mainLayout->addWidget(panelAudio);
                mainLayout->addWidget(panelImage);
                mainLayout->addLayout(btnLayout);
                mainLayout->addWidget(m_fileList);
                mainLayout->addLayout(outLayout);
                mainLayout->addWidget(m_progress);

                splitter->addWidget(m_leftTree);
                splitter->addWidget(rightPanel);
                setCentralWidget(splitter);

                connect(m_btnAdd, &QPushButton::clicked, this, &MainWindow::addFile);
                connect(m_btnDelete, &QPushButton::clicked, this, &MainWindow::deleteSelectedFile);
                connect(m_btnClear, &QPushButton::clicked, this, &MainWindow::clearList);
                //connect(m_btnStart, &QPushButton::clicked, this, &MainWindow::startBatchConvert);
                connect(m_btnPlay, &QPushButton::clicked, this, &MainWindow::playSelectedFile);
                connect(m_leftTree, &QTreeWidget::itemClicked, this, &MainWindow::onMenuSelect);
                connect(btnOutPath, &QPushButton::clicked, this, &MainWindow::selectOutPath);
                connect(m_fileList, &QListWidget::customContextMenuRequested, this, &MainWindow::showFileListContextMenu);

                grpAdv->hide();
                panelAudio->hide();
                panelImage->hide();
                panelFont->hide();
}

void MainWindow::clearList()
{
    m_fileList->clear();
}

void MainWindow::playSelectedFile()
{
    if(m_fileList->selectedItems().isEmpty())
    {
        QMessageBox::warning(this,"提示","请先选中要播放的文件");
        return;
    }
    QString path = m_fileList->selectedItems().first()->text();
    QProcess::startDetached("xdg-open", QStringList() << path);
}

void MainWindow::initLeftMenu()
{
    m_leftTree->clear();
    m_leftTree->setSortingEnabled(false);

    auto makeIcon = [](const QString &fullSvg) -> QIcon {
        QByteArray ba = fullSvg.toUtf8();
        QBuffer buffer(&ba);
        buffer.open(QIODevice::ReadOnly);
        QSvgRenderer renderer(buffer.readAll());
        QPixmap pix(18, 18);
        pix.fill(Qt::transparent);
        QPainter painter(&pix);
        renderer.render(&painter);
        return QIcon(pix);
    };

    QString videoSvg = R"(
<svg width="24" height="24" viewBox="0 0 24 24">
<path fill="#e75749" d="M1.45.67A2.34 2.15 0 012.69.36h18.62a2.33 2.14 0 012.22 1.64 3.08 2.83 0 01.06.72v9.38h-1.44a1.66 1.52 0 00-.2.02 4.79 4.4 0 00-.53-.01c-.27.01-.56-.01-.84.01a2.08 1.91 0 00-.37-.02h-4.33c-.18 0-.36.01-.54-.01l.09-.05-5.81-3.2v3.27c-2.02 0-4.05 0-6.07 0a2.08 1.91 0 00-.23.02c-.19-.03-.39-.01-.59-.01-.26 0-.52-.01-.78.01A1.88 1.72 0 001.74 12.11H.33V2.96A4.38 4.02 0 01.39 2 2.32 2.13 0 011.45.67z"/>
<path fill="#ffffff" d="M9.64 8.84l5.81 3.2-.09.05-5.72 3.15z"/>
<path fill="#c0392b" d="M.33 12.11H1.74a1.88 1.72 0 01.23.02c-.14.22-.22.46-.22.71v.88a.63.58 0 00.63.55h1.09a.62.57 0 00.6-.48v-1.13a.58.53 0 00-.48-.52 2.08 1.91 0 01.23-.02h6.07v3.14l5.72-3.15c.18.01.36.01.54.01h4.33a2.08 1.91 0 01.37.02c-.13.22-.2.46-.2.7v.85a.62.57 0 00.63.6h1.06a.63.57 0 00.63-.55v-.83a.98.9 0 00-.09-.49.59.54 0 00-.38-.25 1.66 1.52 0 01.2-.02h1.44v9.62a2.35 2.16 0 01-.56 1.39 2.32 2.13 0 01-1.43.73h-13c-1.2 0-2.2-.9-2.2-2.2v-8.79c0-.25 0-.51.01-.76z"/>
</svg>)";

    QString audioSvg = R"(
<svg width="24" height="24" viewBox="0 0 24 24">
<path fill="#fa4b00" d="M3 3h10c1.8 0 3.2 1.4 3.2 3.2v10c0 1.8-1.4 3.2-3.2 3.2H3c-1.8 0-3.2-1.4-3.2-3.2V6C-.2 4.4 1.2 3 3 3z"/>
<path fill="#ffffff" d="M14 12.5c-.9 1-1.8 2-3 2.5-1.2.5-2.5.5-4 0V6l-6 1v11c0 1 1 2 2 2 1 0 2-1 2-2V8l8-2v9c-1 0-2.5-.5-3.5-1.5-1-1-1-2-1-3.5 0-1 1-2 2-2.5 1-.5 2-.5 3.5-.5V5l-6 1z"/>
<path fill="#fda076" d="M4 7V4c0-.5.5-1 1-1s1 .5 1 1v3c0 .5-.5 1-1 1s-1-.5-1-1zm-2 4h3c1 0 2 1 2 2v1c0 1-1 2-2 2H2c-1 0-2-1-2-2v-1c0-1 1-2 2-2zm10-6L5 4c-.5 0-1-.5-1-1V2c0-.5.5-1 1-1l7 2c.5 0 1 .5 1 1v1c0 .5-.5 1-1 1z"/>
</svg>)";

    QString imageSvg = R"(
<svg width="24" height="24" viewBox="0 0 24 24">
<path fill="#cce1ff" d="M1 1h22v22H1z"/>
<circle cx="6" cy="6" r="2.2" fill="#ec3232"/>
<path fill="#004bbc" d="M14 9.5c5 0 9 8 9 8s-1 4-4 4-4-1-4-1-9-11-9-11z"/>
<path fill="#5d9cf9" d="M1 13v8h16s-9-11-9-11-8 0-8 0z"/>
</svg>)";

    QString fontSvg = R"(
<svg width="24" height="24" viewBox="0 0 122.88 122.88" xmlns="http://www.w3.org/2000/svg">
<path fill="#2197d8" d="m 77.677005,114.48097 c -5.28662,-0.60648 -8.025471,-1.83355 -10.01273,-3.65299 l -0.140128,-0.15515 c -1.719743,-2.05922 -2.611462,-6.61488 -2.611462,-13.765726 v -76.48724 c 0,-1.071921 0.751591,-1.974592 1.783438,-1.974592 h 11.121009 c 9.961775,0 10.573239,1.523257 13.719734,4.640292 3.426748,2.975992 4.94267,4.499249 6.726108,14.386315 0.203822,0.916775 0.891719,1.593778 1.783436,1.593778 h 0.6879 c 1.03185,0 1.84713,-0.916775 1.78344,-2.059218 L 102.31393,4.2 H 6.7853468 C 5.8171948,4.2 5.0656031,5.0321496 5.0019089,6.104071 L 4.8617817,36.921814 c -0.063694,1.142442 0.7515916,2.059217 1.7834379,2.059217 h 0.6878975 c 0.8917189,0 1.5796164,-0.677003 1.7834379,-1.593778 1.783438,-9.816544 3.29936,-11.339801 6.726109,-14.301689 l 0.06369,-0.07052 c 3.095538,-3.046514 3.770697,-4.569771 13.668778,-4.569771 h 11.121009 c 0.968152,0 1.783438,0.83215 1.783438,1.974592 v 74.131833 c 0,8.518952 -0.828025,13.624682 -2.547768,15.288982 -0.140128,0.0705 -0.203822,0.22567 -0.280255,0.3103 -1.515922,2.27078 -4.050952,3.72351 -9.324833,4.32999 -0.968152,0.0705 -1.656049,0.91678 -1.656049,1.9746 0,1.07192 0.751591,1.97459 1.783438,1.97459 h 47.082762 c 0.968152,0 1.783438,-0.83215 1.783438,-1.97459 0.01274,-0.9873 -0.675159,-1.90407 -1.643311,-1.9746 z M 118.98907,83.79017 L 118.8999,69.826982 H 78.110126 c -0.407643,0 -0.726115,0.36671 -0.764331,0.803941 l -0.06369,13.116934 c -0.03822,0.493648 0.33121,0.874462 0.76433,0.874462 h 0.292994 c 0.382165,0 0.675158,-0.296188 0.76433,-0.677003 0.764331,-4.174852 1.401273,-4.823646 2.86624,-6.078923 l 0.03822,-0.04231 c 1.32484,-1.29759 1.617833,-1.946384 5.83439,-1.946384 h 4.751588 c 0.407643,0 0.764331,0.36671 0.764331,0.832149 v 31.551162 c 0,3.62479 -0.343949,5.78273 -1.082802,6.50205 -0.06369,0.0423 -0.08917,0.0987 -0.114649,0.12694 -0.636942,0.97319 -1.732483,1.57967 -3.987258,1.83355 -0.407643,0.0423 -0.713375,0.38081 -0.713375,0.83215 0,0.45133 0.33121,0.83215 0.76433,0.83215 h 20.101896 c 0.40764,0 0.76433,-0.36671 0.76433,-0.83215 0,-0.42313 -0.29299,-0.80394 -0.71337,-0.83215 -2.25478,-0.25388 -3.42675,-0.77574 -4.28026,-1.55147 l -0.0637,-0.0705 c -0.72611,-0.87446 -1.12102,-2.82085 -1.12102,-5.86736 V 76.723951 c 0,-0.451336 0.33121,-0.83215 0.76433,-0.83215 h 4.75159 c 4.25477,0 4.50955,0.648795 5.85987,1.974592 1.46497,1.255277 2.11465,1.918176 2.86624,6.121236 0.0892,0.380814 0.38216,0.677003 0.76433,0.677003 h 0.29299 c 0.4586,0 0.81529,-0.394918 0.77707,-0.874462 z"/>
</svg>)";

    QIcon icoVideo = makeIcon(videoSvg);
    QIcon icoAudio = makeIcon(audioSvg);
    QIcon icoImage = makeIcon(imageSvg);
    QIcon icoFont = makeIcon(fontSvg);

    QTreeWidgetItem *videoRoot = new QTreeWidgetItem(m_leftTree);
    videoRoot->setIcon(0, icoVideo);
    videoRoot->setText(0, "视频转换");
    QStringList vlist = {
        "转为 MP4","转为 MKV","转为 AVI",
        "转为 MOV","转为 FLV","转为 WMV","转为 WEBM","转为 MPEG","转为 MPG",
        "转为 3GP","转为 3G2",
        "转为 AMV","转为 OGV","转为 TS","转为 M2TS","转为 VOB","转为 ASF",
        "转为 M4V","转为 F4V","转为 GIF"
    };
    for (const QString& s : vlist) {
        QTreeWidgetItem *item = new QTreeWidgetItem(videoRoot);
        item->setText(0, s);
    }

    QTreeWidgetItem *audioRoot = new QTreeWidgetItem(m_leftTree);
    audioRoot->setIcon(0, icoAudio);
    audioRoot->setText(0, "音频转换");
    QStringList alist = {
        "转为 MP3", "转为 WAV", "转为 FLAC", "转为 AAC", "转为 M4A",
        "转为 OGG", "转为 WMA", "转为 ALAC", "转为 AC3", "转为 DTS",
        "转为 OPUS", "转为 AMR", "转为 MKA", "转为 AIFF", "转为 CAF"
    };
    for (const QString& s : alist) {
        QTreeWidgetItem *item = new QTreeWidgetItem(audioRoot);
        item->setText(0, s);
    }

    QTreeWidgetItem *imageRoot = new QTreeWidgetItem(m_leftTree);
    imageRoot->setIcon(0, icoImage);
    imageRoot->setText(0, "图片转换");
    QStringList ilist = {
        "转为 JPG", "转为 PNG", "转为 WebP", "转为 BMP", "转为 GIF",
        "转为 TIFF", "转为 JPEG", "转为 TIF", "转为 PPM", "转为 PGM",
        "转为 PBM", "转为 JP2"
    };
    for (const QString& s : ilist) {
        QTreeWidgetItem *item = new QTreeWidgetItem(imageRoot);
        item->setText(0, s);
    }

    QTreeWidgetItem *fontRoot = new QTreeWidgetItem(m_leftTree);
    fontRoot->setIcon(0, icoFont);
    fontRoot->setText(0, "字体转换");
    QStringList flist = {
        "TTF→OTF", "OTF→TTF", "WOFF→TTF", "WOFF2→TTF",
        "TTF→WOFF", "TTF→WOFF2", "EOT→TTF"
    };
    for (const QString& s : flist) {
        QTreeWidgetItem *item = new QTreeWidgetItem(fontRoot);
        item->setText(0, s);
    }

    new QTreeWidgetItem(m_leftTree, QStringList() << "关于软件");

    videoRoot->setExpanded(true);
    audioRoot->setExpanded(true);
    imageRoot->setExpanded(true);
    fontRoot->setExpanded(true);
}

void MainWindow::onMenuSelect(QTreeWidgetItem *item)
{
    if (!item) return;

    QTreeWidgetItem *root = item;
    while (root->parent())
        root = root->parent();

    bool isVideo = (root->text(0) == "视频转换");
    bool isAudio = (root->text(0) == "音频转换");
    bool isImage = (root->text(0) == "图片转换");
    bool isFont  = (root->text(0) == "字体转换");

    grpAdv->setVisible(false);
    panelAudio->setVisible(false);
    panelImage->setVisible(false);
    panelFont->setVisible(false);
    if (panelAbout) panelAbout->setVisible(false);

    QString currentItemText = item->text(0);
    if (currentItemText == "关于软件") {
        if (panelAbout) panelAbout->setVisible(true);
        return;
    }

    if (isFont)      panelFont->setVisible(true);
    else if (isVideo) grpAdv->setVisible(true);
    else if (isAudio) panelAudio->setVisible(true);
    else if (isImage) panelImage->setVisible(true);

    QString txt = item->text(0);
    m_fmtArg.clear();
    m_outSuffix.clear();

    if (txt == "转为 MP4") {
        m_fmtArg = "-threads 0 -preset fast -c:v libx264 -crf 24 -c:a aac -y";
        m_outSuffix = "mp4";
    } else if (txt == "转为 MP4 高清") {
        m_fmtArg = "-threads 0 -preset fast -c:v libx264 -s 1280x720 -b:v 2500k -c:a aac -b:a 192k -y";
        m_outSuffix = "mp4";
    } else if (txt == "转为 MP4 超清") {
        m_fmtArg = "-threads 0 -preset fast -c:v libx264 -s 1920x1080 -b:v 5000k -c:a aac -b:a 192k -y";
        m_outSuffix = "mp4";
    } else if (txt == "转为 MKV") {
        m_fmtArg = "-threads 0 -preset fast -c:v libx264 -crf 24 -c:a aac -y";
        m_outSuffix = "mkv";
    } else if (txt == "转为 AVI") {
        m_fmtArg = "-threads 0 -preset fast -c:v libxvid -qscale 4 -c:a mp3 -b:a 128k -y";
        m_outSuffix = "avi";
    } else if (txt == "转为 MOV") {
        m_fmtArg = "-threads 0 -preset fast -c:v libx264 -crf 24 -c:a aac -y";
        m_outSuffix = "mov";
    } else if (txt == "转为 FLV") {
        m_fmtArg = "-threads 0 -preset fast -c:v libx264 -c:a aac -f flv -y";
        m_outSuffix = "flv";
    } else if (txt == "转为 WMV") {
        m_fmtArg = "-threads 0 -preset fast -c:v wmv2 -b:v 1500k -c:a wmav2 -b:a 128k -y";
        m_outSuffix = "wmv";
    } else if (txt == "转为 WEBM") {
        m_fmtArg = "-threads 0 -preset fast -c:v libvpx -c:a libvorbis -y";
        m_outSuffix = "webm";
    } else if (txt == "转为 MPEG") {
        m_fmtArg = "-threads 0 -preset fast -c:v mpeg2video -b:v 2000k -c:a mp2 -b:a 128k -y";
        m_outSuffix = "mpeg";
    } else if (txt == "转为 MPG") {
        m_fmtArg = "-threads 0 -preset fast -c:v mpeg2video -b:v 2000k -c:a mp2 -b:a 128k -y";
        m_outSuffix = "mpg";
    } else if (txt == "转为 3GP") {
        m_fmtArg = "-threads 0 -preset fast -f 3gp -s 320x240 -b:v 128k -c:a amr_nb -y";
        m_outSuffix = "3gp";
    } else if (txt == "转为 3GP 高清") {
        m_fmtArg = "-threads 0 -preset fast -f 3gp -s 480x360 -b:v 384k -c:a aac -y";
        m_outSuffix = "3gp";
    } else if (txt == "转为 3GP 超清") {
        m_fmtArg = "-threads 0 -preset fast -f 3gp -s 720x480 -b:v 768k -c:a aac -y";
        m_outSuffix = "3gp";
    } else if (txt == "转为 3G2") {
        m_fmtArg = "-threads 0 -preset fast -f 3gp -s 320x240 -b:v 128k -c:a aac -y";
        m_outSuffix = "3g2";
    } else if (txt == "转为 3G2 高清") {
        m_fmtArg = "-threads 0 -preset fast -f 3gp -s 480x360 -b:v 384k -c:a aac -y";
        m_outSuffix = "3g2";
    } else if (txt == "转为 AMV") {
        m_fmtArg = "-threads 0 -preset fast -f amv -s 160x120 -c:a adpcm_ima_amv -y";
        m_outSuffix = "amv";
    } else if (txt == "转为 OGV") {
        m_fmtArg = "-threads 0 -preset fast -c:v libtheora -c:a libvorbis -y";
        m_outSuffix = "ogv";
    } else if (txt == "转为 TS") {
        m_fmtArg = "-threads 0 -preset fast -c:v libx264 -c:a aac -f mpegts -y";
        m_outSuffix = "ts";
    } else if (txt == "转为 M2TS") {
        m_fmtArg = "-threads 0 -preset fast -c:v libx264 -c:a aac -f mpegts -y";
        m_outSuffix = "m2ts";
    } else if (txt == "转为 VOB") {
        m_fmtArg = "-threads 0 -preset fast -c:v mpeg2video -b:v 4000k -c:a ac3 -b:a 128k -y";
        m_outSuffix = "vob";
    } else if (txt == "转为 ASF") {
        m_fmtArg = "-threads 0 -preset fast -c:v msmpeg4v2 -b:v 1000k -c:a mp3 -b:a 128k -y";
        m_outSuffix = "asf";
    } else if (txt == "转为 M4V") {
        m_fmtArg = "-threads 0 -preset fast -c:v libx264 -crf 24 -c:a aac -y";
        m_outSuffix = "m4v";
    } else if (txt == "转为 F4V") {
        m_fmtArg = "-threads 0 -preset fast -c:v libx264 -c:a aac -f f4v -y";
        m_outSuffix = "f4v";
    } else if (txt == "转为 RM") {
        m_fmtArg = "-c:v h263 -b:v 800k -c:a aac -y";
        m_outSuffix = "rm";
    } else if (txt == "转为 RMVB") {
        m_fmtArg = "-c:v libx264 -b:v 1500k -c:a aac -y";
        m_outSuffix = "rmvb";
    } else if (txt == "转为 DAT") {
        m_fmtArg = "-c:v mpeg1video -b:v 1500k -c:a mp2 -y";
        m_outSuffix = "dat";
    } else if (txt == "转为 GIF") {
        m_fmtArg = "-threads 0 -vf \"fps=15,scale=iw/2:ih/2\" -f gif -loop 0 -y";
        m_outSuffix = "gif";
    } else if (txt == "转为 MP3")  {
        m_fmtArg = "-threads 0 -vn -c:a libmp3lame -b:a 192k -y"; m_outSuffix = "mp3";
    } else if (txt == "转为 WAV")  {
        m_fmtArg = "-threads 0 -vn -c:a pcm_s16le -y"; m_outSuffix = "wav";
    } else if (txt == "转为 FLAC") {
        m_fmtArg = "-threads 0 -vn -c:a flac -compression_level 6 -y"; m_outSuffix = "flac";
    } else if (txt == "转为 AAC")  {
        m_fmtArg = "-threads 0 -vn -c:a aac -b:a 192k -y"; m_outSuffix = "aac";
    } else if (txt == "转为 M4A")  {
        m_fmtArg = "-threads 0 -vn -c:a alac -y"; m_outSuffix = "m4a";
    } else if (txt == "转为 OGG")  {
        m_fmtArg = "-threads 0 -vn -c:a libvorbis -b:a 160k -y"; m_outSuffix = "ogg";
    } else if (txt == "转为 WMA")  {
        m_fmtArg = "-threads 0 -vn -c:a wmav2 -b:a 192k -y"; m_outSuffix = "wma";
    } else if (txt == "转为 ALAC") {
        m_fmtArg = "-threads 0 -vn -c:a alac -y"; m_outSuffix = "alac";
    } else if (txt == "转为 AC3")  {
        m_fmtArg = "-threads 0 -vn -c:a ac3 -b:a 192k -y"; m_outSuffix = "ac3";
    } else if (txt == "转为 DTS")  {
        m_fmtArg = "-threads 0 -vn -c:a dca -b:a 192k -y"; m_outSuffix = "dts";
    } else if (txt == "转为 OPUS") {
        m_fmtArg = "-threads 0 -vn -c:a libopus -b:a 160k -y"; m_outSuffix = "opus";
    } else if (txt == "转为 AMR")  {
        m_fmtArg = "-threads 0 -vn -c:a amr_nb -y"; m_outSuffix = "amr";
    } else if (txt == "转为 MKA")  {
        m_fmtArg = "-threads 0 -vn -c:a copy -y"; m_outSuffix = "mka";
    } else if (txt == "转为 AIFF") {
        m_fmtArg = "-threads 0 -vn -c:a pcm_s16be -y"; m_outSuffix = "aiff";
    } else if (txt == "转为 CAF")  {
        m_fmtArg = "-threads 0 -vn -c:a alac -y"; m_outSuffix = "caf";
    } else if (isImage) {
        QString fmt = txt.replace("转为 ", "").toLower();
        m_outSuffix = fmt;
        m_fmtArg = "USE_OPENCV";
        int idx = cmbImageFormat->findText(fmt.toUpper());
        if (idx != -1) cmbImageFormat->setCurrentIndex(idx);
    } else if (isFont) {
        if (txt == "TTF→OTF") { m_outSuffix = "otf"; }
        else if (txt == "OTF→TTF") { m_outSuffix = "ttf"; }
        else if (txt == "WOFF→TTF") { m_outSuffix = "ttf"; }
        else if (txt == "WOFF2→TTF") { m_outSuffix = "ttf"; }
        else if (txt == "TTF→WOFF") { m_outSuffix = "woff"; }
        else if (txt == "TTF→WOFF2") { m_outSuffix = "woff2"; }
        else if (txt == "EOT→TTF") { m_outSuffix = "ttf"; }
        m_fmtArg = "-c \"Open('%1'); Generate('%2');\"";
    }
}

void MainWindow::addFile()
{
    QStringList files = QFileDialog::getOpenFileNames(this);
    m_fileList->addItems(files);
}

void MainWindow::selectOutPath()
{
    QString path = QFileDialog::getExistingDirectory(this);
    if (!path.isEmpty())
        m_editOutPath->setText(path);
}

void MainWindow::deleteSelectedFile()
{
    for (auto item : m_fileList->selectedItems())
        delete item;
}

void MainWindow::showFileListContextMenu(const QPoint &pos)
{
    QMenu menu;
    QAction *del = menu.addAction("删除选中");
    connect(del, &QAction::triggered, this, &MainWindow::deleteSelectedFile);
    menu.exec(m_fileList->mapToGlobal(pos));
}

void MainWindow::startBatchConvert(const QString &action)
{
    //------------------------------------------------------------------
    // 1. 视频转换
    //------------------------------------------------------------------
    if (action == "视频转换") {
        if (m_fileList->count() == 0 || m_outSuffix.isEmpty()) {
            QMessageBox::warning(this, "提示", "请添加文件并选择格式");
            return;
        }

        auto selected = m_fileList->selectedItems();
        if (selected.isEmpty()) {
            QMessageBox::warning(this, "提示", "请选择要转换的文件！");
            return;
        }

        // 预设参数
        QString preset = cmbPreset->currentText();
        QString codec = cmbCodec->currentText().trimmed();
        QString fps   = cmbFPS->currentText().trimmed();
        QString res   = cmbResolution->currentText();
        QString crf   = "23";

        if (preset == "抖音短视频") {
            codec = "H.264"; fps = "30"; res = "720p"; crf = "24";
        } else if (preset == "B站超清") {
            codec = "H.265"; fps = "60"; res = "1080p"; crf = "23";
        } else if (preset == "高质量小体积") {
            codec = "H.265"; crf = "26";
        } else if (preset == "录屏专用") {
            codec = "H.264"; fps = "30"; crf = "28";
        } else if (preset == "动漫专用") {
            codec = "H.265"; crf = "25";
        } else if (preset == "电影压制") {
            codec = "H.265"; crf = "22";
        } else if (preset == "超快压缩") {
            crf = "30";
        }

        m_btnStart->setEnabled(false);
        m_progress->setValue(0);

        for (int i = 0; i < selected.size(); ++i) {
            QString src = selected[i]->text();
            QFileInfo fi(src);
            QString out = m_editOutPath->text() + "/" + fi.baseName() + "." + m_outSuffix;

            QStringList args;
            args << "-y" << "-i" << src;

            // 清理旧参数
            args.removeAll("-c:v");
            args.removeAll("libmp3lame");
            args.removeAll("pcm_s16le");
            args.removeAll("flac");
            args.removeAll("libopus");
            args.removeAll("ac3");
            args.removeAll("ape");
            args.removeAll("-ar");
            args.removeAll("-b:a");
            args.removeAll("-ac");
            args.removeAll("-af");
            args.removeAll("libx264");
            args.removeAll("libx265");
            args.removeAll("mpeg4");
            args.removeAll("libvpx-vp9");
            args.removeAll("libtheora");
            args.removeAll("-c:a");
            args.removeAll("aac");
            args.removeAll("libvorbis");
            args.removeAll("-f");
            args.removeAll("ogg");
            args.removeAll("-r");
            args.removeAll("-s");
            args.removeAll("-crf");
            args.removeAll("-vf");
            args.removeAll("-ss");
            args.removeAll("-to");

            //QString decoder = cmbDecode->currentText().trimmed();
            //QString renderer = cmbRender->currentText().trimmed();

            // ===========================================
            // 🔥 最终修复版 —— 命令格式完全正确！
            // ===========================================
            int subMode = cmbSubtitle->currentIndex();

            if (subMode == 2)
            {
                QStringList cmd;
                // 必须分开！分开！分开！每个参数独立！
                cmd << "-y";
                cmd << "-i" << src;
                cmd << "-map" << "0:s:0";
                cmd << "-c:s" << "srt";

                QString outSub = m_editOutPath->text() + "/" + fi.baseName() + ".srt";
                cmd << outSub;

                qDebug() << "正确命令：" << cmd;

                QProcess p;
                p.start("ffmpeg", cmd);  // ✅ 正确用法
                p.waitForFinished(-1);

                m_progress->setValue((i+1)*100/selected.size());
                continue;
            }

            if (subMode == 0)
            {
                args << "-c:s" << "none";
            }
            else if (subMode == 1)
            {
                QString subPath = fi.absolutePath() + "/" + fi.completeBaseName() + ".srt";
                if (QFile::exists(subPath)) {
                    args << "-vf" << "subtitles=" + subPath;
                }
            }
            else if (subMode == 3)
            {
                args << "-c:s" << "none";
            }


            // 视频编码
            if (codec == "H.264") {
                args << "-c:v" << "libx264";
            }
            else if (codec == "H.265") {
                args << "-c:v" << "libx265";
            }
            else if (codec == "MPEG-4") {
                args << "-c:v" << "mpeg4";
            }
            else if (codec == "VP9") {
                args << "-c:v" << "libvpx-vp9";
            }
            else if (codec == "Theora") {
                args << "-c:v" << "libtheora";
            }

            // 音频编码
            if (codec == "Theora") {
                args << "-c:a" << "libvorbis";
                args << "-f" << "ogg";
            } else {
                args << "-c:a" << "aac";
            }

            // 帧率
            if (!fps.isEmpty() && fps != "原帧率") {
                args << "-r" << fps;
            }

            // 分辨率
            if (res == "360p") args << "-s" << "640x360";
            if (res == "480p") args << "-s" << "854x480";
            if (res == "720p") args << "-s" << "1280x720";
            if (res == "1080p") args << "-s" << "1920x1080";
            if (res == "2K") args << "-s" << "2560x1440";
            if (res == "4K") args << "-s" << "3840x2160";

            args << "-crf" << crf;

            // 滤镜
            QString vf;
            QString filter = cmbFilterPreset->currentText();
            if (filter == "黑白")        vf = "hue=s=0";
            else if (filter == "复古")   vf = "curves=vintage";
            else if (filter == "模糊")   vf = "boxblur=1";
            else if (filter == "水平翻转") vf = "hflip";
            else if (filter == "垂直翻转") vf = "vflip";
            else if (filter == "亮度+50") vf = "eq=brightness=0.5";
            else if (filter == "对比度+30") vf = "eq=contrast=1.3";

            // 水印
            if (cmbWatermark->currentText() == "自定义水印") {
                QString text = editWaterText->text().trimmed();
                if (!text.isEmpty()) {
                    text.replace("'", "");
                    QString color = "white";
                    if (cmbWaterColor->currentText() == "黑色") color = "black";
                    if (cmbWaterColor->currentText() == "红色") color = "red";
                    if (cmbWaterColor->currentText() == "绿色") color = "green";
                    if (cmbWaterColor->currentText() == "蓝色") color = "blue";

                    QString size = cmbWaterSize->currentText();
                    QString x = "20", y = "30";
                    QString pos = cmbWaterPos->currentText();
                    if (pos == "右上角") { x = "w-text_w-20"; y = "30"; }
                    if (pos == "左下角") { x = "20"; y = "h-30"; }
                    if (pos == "右下角") { x = "w-text_w-20"; y = "h-30"; }
                    if (pos == "居中")   { x = "(w-text_w)/2"; y = "(h-text_h)/2"; }

                    QString wm = QString("drawtext=text='%1':fontcolor=%2:fontsize=%3:x=%4:y=%5")
                    .arg(text).arg(color).arg(size).arg(x).arg(y);

                    if (vf.isEmpty()) vf = wm;
                    else vf += "," + wm;
                }
            }

            if (!vf.isEmpty()) {
                args << "-vf" << vf;
            }

            // 裁剪时间段
            QString ss = editStartTime->text().trimmed();
            QString to = editEndTime->text().trimmed();
            if (!ss.isEmpty() && !to.isEmpty() && to != "00:00:00")
            {
                args << "-ss" << ss;
                args << "-to" << to;
            }

            args << out;
            QProcess proc;
            proc.start("ffmpeg", args);
            proc.waitForFinished(-1);
            m_progress->setValue((i+1)*100/selected.size());
        }

        QMessageBox::information(this, "完成", "转换完成！");
        m_btnStart->setEnabled(true);
        return;
    }

    //------------------------------------------------------------------
    // 2. 音频转换（完整版：时间截取+编码+采样率+音效+预设 全部正常）
    //------------------------------------------------------------------
    if (action == "音频转换") {
        if (m_fileList->count() == 0 || m_outSuffix.isEmpty()) {
            QMessageBox::warning(this,"提示","请添加文件并选择格式");
            return;
        }
        auto selected = m_fileList->selectedItems();
        if (selected.isEmpty()) {
            QMessageBox::warning(this,"提示","请选择要转换的文件");
            return;
        }

        m_btnStart->setEnabled(false);
        m_progress->setValue(0);

        for (int i=0;i<selected.size();++i) {
            QString src = selected[i]->text();
            QFileInfo fi(src);
            QString out = m_editOutPath->text() + "/" + fi.baseName() + "." + m_outSuffix;

            QStringList args;
            args << "-y";

            // 裁剪时间段
            QString ss = editAudioStart->text().trimmed();
            QString to = editAudioEnd->text().trimmed();
            if (!ss.isEmpty() && ss != "") args << "-ss" << ss;
            if (!to.isEmpty() && to != "00:00:00") args << "-to" << to;

            args << "-i" << src << "-vn";

            // 清理旧参数
            args.removeAll("-c:v");
            args.removeAll("libx264");
            args.removeAll("libx265");
            args.removeAll("mpeg4");
            args.removeAll("libvpx-vp9");
            args.removeAll("libtheora");
            args.removeAll("-r");
            args.removeAll("-s");
            args.removeAll("-crf");
            args.removeAll("-vf");
            args.removeAll("-f");
            //args.removeAll("-ss");
            //args.removeAll("-to");

            args.removeAll("libmp3lame");
            args.removeAll("pcm_s16le");
            args.removeAll("flac");
            args.removeAll("aac");
            args.removeAll("libopus");
            args.removeAll("ac3");
            args.removeAll("ape");
            //args.removeAll("-ar");

            // 音频编码
            bool isLossless = false;
            if (m_outSuffix == "mp3") {
                args << "-c:a" << "libmp3lame";
            } else if (m_outSuffix == "wav") {
                args << "-c:a" << "pcm_s16le";
                isLossless = true;
            } else if (m_outSuffix == "flac") {
                args << "-c:a" << "flac";
                isLossless = true;
            } else if (m_outSuffix == "ape") {
                args << "-c:a" << "ape";
                isLossless = true;
            } else if (m_outSuffix == "aac" || m_outSuffix == "m4a") {
                args << "-c:a" << "aac";
            } else if (m_outSuffix == "opus") {
                args << "-c:a" << "libopus";
            } else if (m_outSuffix == "ac3") {
                args << "-c:a" << "ac3";
            } else {
                args << "-c:a" << "copy";
            }

            // 1. 采样率（下拉框生效）
            // --------------------------
            //QString sampleRate = cmbSampleRate->currentText().trimmed();
            //if (!sampleRate.isEmpty()) {
                //args << "-ar" << sampleRate;
            //}

            // 2. 比特率（生效）
            // --------------------------
            //QString bitRate = cmbBitrate->currentText().trimmed();
            //if (!bitRate.isEmpty()) {
                //args << "-b:a" << bitRate;
            //}


            // ====================== 【重要】手动参数优先！
            QString sampleRate = cmbSampleRate->currentText().trimmed();
            QString bitRate    = cmbBitrate->currentText().trimmed();
            QString channel    = cmbAudioChannel->currentText().trimmed();
            QString preset     = cmbAudioPreset->currentText().trimmed();
            QString filter     = cmbAudioFilter->currentText();

            bool useCustom = !sampleRate.isEmpty() || !bitRate.isEmpty() || channel != "立体声";

            if (!useCustom) {
                // 只有手动没改时，才用预设
                if (preset == "抖音")         { args << "-ar" << "44100" << "-b:a" << "128k" << "-ac" << "2"; }
                else if (preset == "标准音乐") { args << "-ar" << "44100" << "-b:a" << "192k" << "-ac" << "2"; }
                else if (preset == "无损")     { args << "-ar" << "48000" << "-b:a" << "320k" << "-ac" << "2"; }
                else if (preset == "录音")     { args << "-ar" << "32000" << "-b:a" << "96k"  << "-ac" << "1"; }
                else if (preset == "极速压缩") { args << "-ar" << "22050" << "-b:a" << "64k"  << "-ac" << "1"; }
            }

            // ====================== 手动设置（永远覆盖！必生效！）
            if (!sampleRate.isEmpty()) args << "-ar" << sampleRate;
            if (!bitRate.isEmpty() && !isLossless) args << "-b:a" << bitRate;

            if      (channel == "单声道")   args << "-ac" << "1";
            else if (channel == "立体声") args << "-ac" << "2";
            else if (channel == "5.1环绕") args << "-ac" << "6";

            // ====================== 音效（必生效）
            //QString filter = cmbAudioFilter->currentText();
            if (filter == "降噪") args << "-af" << "afftdn";
            else if (filter == "低音增强") args << "-af" << "bass=g=5";
            else if (filter == "人声增强") args << "-af" << "equalizer=f=1000:t=h:width=200:g=3";
            else if (filter == "消人声") args << "-af" << "pan=stereo|0.5*FL-0.5*FR|0.5*FR-0.5*FL";
            else if (filter == "淡入淡出") args << "-af" << "afade=t=in:st=0:d=2,afade=t=out:st=10:d=2";

            args << out;

            QProcess p;
            p.start("ffmpeg", args);
            p.waitForFinished(-1);
            m_progress->setValue((i+1)*100/selected.size());
        }

        QMessageBox::information(this,"完成","音频转换完成！");
        m_btnStart->setEnabled(true);
        return;
    }

    //------------------------------------------------------------------
    // 3. 视频合并
    //------------------------------------------------------------------
    if (action == "视频合并") {
        // 【安全判断：控件必须存在】
        if (!cmbMergeMode) {
            QMessageBox::warning(this, "错误", "控件未初始化");
            return;
        }

        QList<QListWidgetItem*> items = m_fileList->selectedItems();
        if (items.size() < 2) {
            QMessageBox::warning(this, "提示", "请选择至少2个视频！");
            return;
        }

        QString listPath = m_editOutPath->text() + "/list.txt";
        QFile listFile(listPath);
        if (!listFile.open(QIODevice::WriteOnly | QIODevice::Text)) return;

        QTextStream out(&listFile);
        for (auto* item : items) out << "file '" << item->text() << "'\n";
        listFile.close();

        QString mergeMode = cmbMergeMode->currentText();
        QString outFile = m_editOutPath->text() + "/merged.mp4";
        QStringList args;
        args << "-y" << "-f" << "concat" << "-safe" << "0" << "-i" << listPath;

        if (mergeMode == "默认合并(无损快切)") args << "-c" << "copy";
        else if (mergeMode == "兼容合并(重新编码)") args << "-c:v" << "libx264" << "-c:a" << "aac";
        else if (mergeMode == "仅合并视频(无声)") args << "-c" << "copy" << "-an";
        else if (mergeMode == "合并转MP4(标准)") args << "-c:v" << "libx264" << "-c:a" << "aac";

        args << outFile;
        QProcess p;
        p.start("ffmpeg", args);
        p.waitForFinished(-1);
        QFile::remove(listPath);
        QMessageBox::information(this, "完成", "合并完成！");
        return;
    }

    //------------------------------------------------------------------
    // 4. 提取视频帧
    //------------------------------------------------------------------
    if (action == "提取视频帧") {
        // 【安全判断】
        if (!cmbFrameMode) {
            QMessageBox::warning(this, "错误", "控件未初始化");
            return;
        }

        QList<QListWidgetItem*> items = m_fileList->selectedItems();
        if (items.isEmpty()) {
            QMessageBox::warning(this, "提示", "请选择视频！");
            return;
        }

        QString src = items.first()->text();
        QString outPattern = m_editOutPath->text() + "/frame_%04d.jpg";
        QString frameMode = cmbFrameMode->currentText();

        QStringList args;
        args << "-y" << "-i" << src;

        if (frameMode == "每秒1帧") args << "-r" << "1";
        else if (frameMode == "每秒5帧") args << "-r" << "5";
        else if (frameMode == "每秒10帧") args << "-r" << "10";
        else if (frameMode == "每秒20帧") args << "-r" << "20";
        else if (frameMode == "每秒30帧") args << "-r" << "30";
        else if (frameMode == "每5秒1帧") args << "-r" << "0.2";
        else if (frameMode == "每10秒1帧") args << "-r" << "0.1";
        else if (frameMode == "仅关键帧(I帧)") {
            args << "-vf" << "select='eq(pict_type,PICT_TYPE_I)'";
            args << "-vsync" << "vfr";
        }

        args << outPattern;
        QProcess p;
        p.start("ffmpeg", args);
        p.waitForFinished(-1);
        QMessageBox::information(this, "完成", "帧提取完成！");
        return;
    }

    //------------------------------------------------------------------
    // 5. 图片转换
    //------------------------------------------------------------------
    if (action == "图片转换") {
        if (m_fileList->count() == 0 || m_outSuffix.isEmpty()) {
            QMessageBox::warning(this, "提示", "请添加文件并选择格式");
            return;
        }
        auto selected = m_fileList->selectedItems();
        if (selected.isEmpty()) return;

        m_btnStart->setEnabled(false);
        m_progress->setValue(0);

        QString imgFormat    = cmbImageFormat->currentText().trimmed();
        int width            = editImgWidth->text().toInt();
        int height           = editImgHeight->text().toInt();
        int imgQuality       = cmbImageQuality->currentIndex() * 25 + 25;
        QString rotateSel    = cmbRotate->currentText();
        QString flipSel      = cmbFlip->currentText();
        QString effectSel    = cmbImageEffect->currentText();
        QString waterText    = editImageWaterText->text().trimmed();
        QString waterColor   = cmbImageWaterColor->currentText();
        QString waterPos     = cmbImageWaterPos->currentText();
        int     waterSize    = cmbImageWaterSize->currentText().toInt();

        if(!imgFormat.isEmpty())
            m_outSuffix = imgFormat.toLower();

        for (int i=0;i<selected.size();++i) {
            QString src = selected[i]->text();
            QFileInfo fi(src);
            QString out = m_editOutPath->text() + "/" + fi.baseName() + "." + m_outSuffix;

            cv::Mat img = cv::imread(src.toStdString());
            if (img.empty()) continue;

            // 宽高
            if (width > 0 && height > 0)
                cv::resize(img, img, cv::Size(width, height));

            // 旋转
            if (rotateSel == "90°")  cv::rotate(img, img, cv::ROTATE_90_CLOCKWISE);
            if (rotateSel == "180°") cv::rotate(img, img, cv::ROTATE_180);
            if (rotateSel == "270°") cv::rotate(img, img, cv::ROTATE_90_COUNTERCLOCKWISE);

            // 翻转
            if (flipSel == "水平") cv::flip(img, img, 1);
            if (flipSel == "垂直") cv::flip(img, img, 0);

            // ====================== 特效（必须放前面！）======================
            if (effectSel == "黑白") {
                cv::cvtColor(img, img, cv::COLOR_BGR2GRAY);
                cv::cvtColor(img, img, cv::COLOR_GRAY2BGR); // 保持3通道，水印可上色
            }
            if (effectSel == "灰度") {
                cv::cvtColor(img, img, cv::COLOR_BGR2GRAY);
                cv::cvtColor(img, img, cv::COLOR_GRAY2BGR);
            }
            if (effectSel == "反色") {
                cv::bitwise_not(img, img);
            }
            if (effectSel == "锐化") {
                cv::Mat kernel = (cv::Mat_<float>(3,3) << 0,-1,0, -1,5,-1, 0,-1,0);
                cv::filter2D(img, img, -1, kernel);
            }
            if (effectSel == "轻度模糊") cv::blur(img, img, cv::Size(3,3));
            if (effectSel == "高斯模糊") cv::GaussianBlur(img, img, cv::Size(5,5), 0);
            if (effectSel == "复古") {
                cv::Mat yuv;
                cv::cvtColor(img, yuv, cv::COLOR_BGR2YUV);
                std::vector<cv::Mat> ch;
                cv::split(yuv, ch);
                ch[1] *= 0.7;
                cv::merge(ch, yuv);
                cv::cvtColor(yuv, img, cv::COLOR_YUV2BGR);
            }
            if (effectSel == "亮度+10%") img.convertTo(img, -1, 1.0, 10);
            if (effectSel == "亮度+20%") img.convertTo(img, -1, 1.0, 20);
            if (effectSel == "对比度+20%") img.convertTo(img, -1, 1.2, 0);

            if (!waterText.isEmpty())
            {
                if (img.channels() != 3)
                    cv::cvtColor(img, img, cv::COLOR_GRAY2BGR);

                // 颜色
                cv::Scalar color = cv::Scalar(255,255,255);
                if (waterColor == "红色")    color = cv::Scalar(0,0,255);
                if (waterColor == "蓝色")    color = cv::Scalar(255,0,0);
                if (waterColor == "绿色")    color = cv::Scalar(0,255,0);
                if (waterColor == "黄色")    color = cv::Scalar(0,255,255);
                if (waterColor == "黑色")    color = cv::Scalar(0,0,0);

                // ==========================
                // ✅ 位置全部重新优化：不靠边、完整显示
                // ==========================
                int x = 0, y = 0;
                const int margin = 60; // 统一边距，再也不裁切

                if (waterPos == "左上角") {
                    x = margin;
                    y = margin + 20;   // 向下挪，不顶头
                }
                else if (waterPos == "右上角") {
                    x = img.cols - 220;  // 大幅向左缩
                    y = margin + 20;     // 向下挪
                }
                else if (waterPos == "左下角") {
                    x = margin;
                    y = img.rows - margin;
                }
                else if (waterPos == "右下角") {
                    x = img.cols - 220;  // 大幅向左缩
                    y = img.rows - margin;
                }
                else if (waterPos == "居中") {
                    x = img.cols / 2 - 80;
                    y = img.rows / 2;
                }

                // 字体大小（必生效）
                double scale = (double)waterSize / 24.0;

                // 绘制水印
                cv::putText(img,
                            waterText.toStdString(),
                            cv::Point(x, y),
                            cv::FONT_HERSHEY_SIMPLEX,
                            scale,
                            color,
                            4);
            }

            // ====================== 保存 ======================
            std::vector<int> params;

            // JPG / JPEG
            if (m_outSuffix == "jpg" || m_outSuffix == "jpeg" || m_outSuffix == "jpe" || m_outSuffix == "jfif") {
                params.push_back(cv::IMWRITE_JPEG_QUALITY);
                params.push_back(imgQuality);
            }
            // PNG
            else if (m_outSuffix == "png") {
                params.push_back(cv::IMWRITE_PNG_COMPRESSION);
                params.push_back(9 - (imgQuality / 10));
            }
            // WebP
            else if (m_outSuffix == "webp") {
                params.push_back(cv::IMWRITE_WEBP_QUALITY);
                params.push_back(imgQuality);
            }
            // TIFF / TIF
            else if (m_outSuffix == "tiff" || m_outSuffix == "tif") {
                params.push_back(cv::IMWRITE_TIFF_COMPRESSION);
                params.push_back(1); // 无压缩
            }
            // BMP / GIF / PPM / PGM / PBM / JP2 / SR / RAS → 无需参数
            else if (m_outSuffix == "bmp" ||
                m_outSuffix == "gif" ||
                m_outSuffix == "ppm" ||
                m_outSuffix == "pgm" ||
                m_outSuffix == "pbm" ||
                m_outSuffix == "jp2") {
                // 这些格式 OpenCV 不需要额外参数
                }

                cv::imwrite(out.toStdString(), img, params);
            m_progress->setValue((i+1)*100/selected.size());
        }

        QMessageBox::information(this, "完成", "图片转换完成！");
        m_btnStart->setEnabled(true);
        return;
    }
}

void MainWindow::updateVideoPreview()
{
    if (m_fileList->selectedItems().isEmpty())
        return;

    m_player->stop();
    m_player->setSource(QUrl());
    disconnect(m_player, nullptr, this, nullptr);
    disconnect(sliderTrim, nullptr, this, nullptr); // 清理旧信号

    QString path = m_fileList->selectedItems().first()->text();
    m_player->setSource(QUrl::fromLocalFile(path));

    connect(m_player, &QMediaPlayer::mediaStatusChanged, this, [this](QMediaPlayer::MediaStatus status) {
        if (status == QMediaPlayer::LoadedMedia || status == QMediaPlayer::BufferedMedia) {
            disconnect(m_player, nullptr, this, nullptr);
            qint64 duration = m_player->duration();
            sliderTrim->setRange(0, duration);
            applyVideoFilter();

            QTime start = QTime::fromString(editStartTime->text(), "HH:mm:ss");
            qint64 startMs = start.msecsSinceStartOfDay();
            if (startMs > duration) startMs = 0;
            m_player->setPosition(startMs);
            m_player->play();

            // 播放位置 → 更新进度条 + 同步开始时间
            connect(m_player, &QMediaPlayer::positionChanged, this, [this](qint64 pos) {
                sliderTrim->blockSignals(true);
                sliderTrim->setValue(pos);
                sliderTrim->blockSignals(false);

                // 👇 实时同步到【开始时间】框
                QTime current = QTime::fromMSecsSinceStartOfDay(pos);
                editStartTime->setText(current.toString("HH:mm:ss"));

                QTime end = QTime::fromString(editEndTime->text(), "HH:mm:ss");
                qint64 endMs = end.msecsSinceStartOfDay();
                qint64 duration = m_player->duration();
                if (endMs <= 0 || endMs > duration) endMs = duration;
                if (pos >= endMs) {
                    QTime s = QTime::fromString(editStartTime->text(), "HH:mm:ss");
                    m_player->setPosition(s.msecsSinceStartOfDay());
                }
            });

            // 拖动进度条 → 跳转播放 + 同步时间
            connect(sliderTrim, &QSlider::sliderMoved, this, [this](int value) {
                m_player->setPosition(value);

                // 👇 拖动也同步开始时间
                QTime current = QTime::fromMSecsSinceStartOfDay(value);
                editStartTime->setText(current.toString("HH:mm:ss"));
            });
        }
    });
}

void MainWindow::applyVideoFilter()
{
    if (!videoItem) return;

    static QGraphicsEffect *currentEffect = nullptr;
    delete currentEffect;
    currentEffect = nullptr;

    QString f = cmbFilterPreset->currentText();
    QTransform transform;
    transform.reset();

    if (f == "水平翻转") transform.scale(-1, 1);
    else if (f == "垂直翻转") transform.scale(1, -1);
    videoItem->setTransform(transform);

    if (f == "黑白") {
        QGraphicsColorizeEffect *e = new QGraphicsColorizeEffect;
        e->setColor(Qt::black);
        e->setStrength(1);
        currentEffect = e;
    } else if (f == "模糊") {
        QGraphicsBlurEffect *e = new QGraphicsBlurEffect;
        e->setBlurRadius(3);
        currentEffect = e;
    } else if (f == "复古") {
        QGraphicsColorizeEffect *e = new QGraphicsColorizeEffect;
        e->setColor(QColor(140, 80, 20));
        e->setStrength(0.4);
        currentEffect = e;
    } else if (f == "亮度+50") {
        QGraphicsOpacityEffect *e = new QGraphicsOpacityEffect;
        e->setOpacity(1.5);
        currentEffect = e;
    } else if (f == "对比度+30") {
        QGraphicsColorizeEffect *e = new QGraphicsColorizeEffect;
        e->setColor(Qt::white);
        e->setStrength(0.2);
        currentEffect = e;
    }

    videoItem->setGraphicsEffect(currentEffect);
}

QString MainWindow::getQualityArg()
{
    QString level = cmbQuality->currentText();
    if (level == "低画质") return "-preset fast -crf 30 -b:v 0";
    else if (level == "中等画质") return "-preset medium -crf 24 -b:v 0";
    else if (level == "高画质") return "-preset slow -crf 20 -b:v 0";
    else if (level == "超清") return "-preset slow -crf 18 -b:v 0";
    else if (level == "无损画质") return "-preset veryslow -crf 16 -b:v 0";
    return "-preset slow -crf 18 -b:v 0";
}
void MainWindow::playAudioFile()
{
    if (!m_fileList->currentItem()) return;
    QString path = m_fileList->currentItem()->text();

    audioPlayer->stop();
    audioPlayer->setSource(QUrl());
    disconnect(audioPlayer, nullptr, this, nullptr);
    disconnect(sliderTrim, nullptr, this, nullptr);

    audioPlayer->setSource(QUrl::fromLocalFile(path));

    connect(audioPlayer, &QMediaPlayer::mediaStatusChanged, this, [this](QMediaPlayer::MediaStatus status) {
        if (status == QMediaPlayer::LoadedMedia || status == QMediaPlayer::BufferedMedia) {

            disconnect(audioPlayer, nullptr, this, nullptr);
            qint64 duration = audioPlayer->duration();
            sliderTrim->setRange(0, duration);

            // 🔥 初始化进度条右边的时间显示：00:00 / 总时长
            QTime totalTime = QTime::fromMSecsSinceStartOfDay(duration);
            // 把下面的 labelAudioTime 换成你界面上进度条右边的标签名
            lblAudioTime->setText(QString("00:00 / %1").arg(totalTime.toString("HH:mm:ss")));

            // 🔥 结束时间框保持原样，不自动填充，只用于用户手动设置
            // editAudioEnd 不做任何修改！

            QTime startTime = QTime::fromString(editAudioStart->text(), "HH:mm:ss");
            qint64 startMs = startTime.msecsSinceStartOfDay();
            if (startMs > duration) startMs = 0;

            audioPlayer->setPosition(startMs);
            audioPlayer->play();

            connect(audioPlayer, &QMediaPlayer::positionChanged, this, [this, duration](qint64 pos) {
                sliderTrim->blockSignals(true);
                sliderTrim->setValue(pos);
                sliderTrim->blockSignals(false);

                QTime current = QTime::fromMSecsSinceStartOfDay(pos);
                editAudioStart->setText(current.toString("HH:mm:ss"));

                // 🔥 实时更新进度条右边的“当前时间 / 总时长”
                QTime total = QTime::fromMSecsSinceStartOfDay(duration);
                lblAudioTime->setText(QString("%1 / %2")
                .arg(current.toString("HH:mm:ss"))
                .arg(total.toString("HH:mm:ss")));

                QTime endTime = QTime::fromString(editAudioEnd->text(), "HH:mm:ss");
                qint64 endMs = endTime.msecsSinceStartOfDay();

                if (endMs <= 0 || endMs > duration)
                    endMs = duration;

                if (pos >= endMs) {
                    QTime s = QTime::fromString(editAudioStart->text(), "HH:mm:ss");
                    audioPlayer->setPosition(s.msecsSinceStartOfDay());
                }
            });

            connect(sliderTrim, &QSlider::sliderMoved, this, [this](int value) {
                audioPlayer->setPosition(value);

                QTime current = QTime::fromMSecsSinceStartOfDay(value);
                editAudioStart->setText(current.toString("HH:mm:ss"));
            });
        }
    });
}

void MainWindow::applyVideoPreset()
{
    QString preset = cmbPreset->currentText();
    if (preset == "自定义") return;

    if (preset == "抖音短视频") {
        cmbResolution->setCurrentText("720p");
        cmbFPS->setCurrentText("30");
        cmbCodec->setCurrentText("H.264");
        edtCRF->setText("24");
        edtVolume->setText("1.0");
    } else if (preset == "B站超清") {
        cmbResolution->setCurrentText("1080p");
        cmbFPS->setCurrentText("60");
        cmbCodec->setCurrentText("H.265");
        edtCRF->setText("23");
    } else if (preset == "高质量小体积") {
        cmbCodec->setCurrentText("H.265");
        edtCRF->setText("26");
    } else if (preset == "录屏专用") {
        cmbFPS->setCurrentText("30");
        cmbCodec->setCurrentText("H.264");
        edtCRF->setText("28");
    } else if (preset == "动漫专用") {
        cmbCodec->setCurrentText("H.265");
        edtCRF->setText("25");
    } else if (preset == "电影压制") {
        cmbCodec->setCurrentText("H.265");
        edtCRF->setText("22");
    } else if (preset == "超快压缩") {
        edtCRF->setText("30");
    }
}

void MainWindow::applyAudioPreset()
{
    QString preset = cmbAudioPreset->currentText();
    if (preset == "抖音短视频") {
        cmbAudioCodec->setCurrentText("AAC");
        cmbSampleRate->setCurrentText("44100");
        cmbBitrate->setCurrentText("128k");
        cmbAudioChannel->setCurrentText("立体声");
    } else if (preset == "标准音乐") {
        cmbAudioCodec->setCurrentText("MP3");
        cmbSampleRate->setCurrentText("44100");
        cmbBitrate->setCurrentText("320k");
    } else if (preset == "高清无损") {
        cmbAudioCodec->setCurrentText("FLAC无损");
        cmbSampleRate->setCurrentText("48000");
        cmbBitrate->setCurrentText("1000k");
    } else if (preset == "语音录音") {
        cmbAudioCodec->setCurrentText("AAC");
        cmbSampleRate->setCurrentText("32000");
        cmbBitrate->setCurrentText("96k");
        cmbAudioChannel->setCurrentText("单声道");
    } else if (preset == "极速压缩") {
        cmbAudioCodec->setCurrentText("MP3");
        cmbBitrate->setCurrentText("64k");
    }
}

void MainWindow::applyImagePreset()
{
    QString p = cmbImagePreset->currentText();
    cmbRotate->setCurrentIndex(0);
    cmbFlip->setCurrentIndex(0);

    if (p == "微信图") {
        cmbImageFormat->setCurrentText("JPG");
        editImgWidth->setText("400");
        editImgHeight->setText("400");
        cmbImageQuality->setCurrentIndex(1);
    } else if (p == "高清图") {
        cmbImageFormat->setCurrentText("PNG");
        editImgWidth->clear();
        editImgHeight->clear();
        cmbImageQuality->setCurrentIndex(3);
    } else if (p == "网页小图") {
        cmbImageFormat->setCurrentText("WebP");
        editImgWidth->clear();
        editImgHeight->clear();
        cmbImageQuality->setCurrentIndex(1);
    } else if (p == "超高压缩") {
        cmbImageFormat->setCurrentText("WebP");
        editImgWidth->clear();
        editImgHeight->clear();
        cmbImageQuality->setCurrentIndex(0);
    } else if (p == "无损PNG") {
        cmbImageFormat->setCurrentText("PNG");
        editImgWidth->clear();
        editImgHeight->clear();
        cmbImageQuality->setCurrentIndex(3);
    }
}

void MainWindow::startFontConvert()
{
    QString i = fontInputPath->text().trimmed();
    QString o = fontOutputPath->text().trimmed();
    QString ext = fontFormatCombo->currentText().toLower();

    if (i.isEmpty() || o.isEmpty()) {
        QMessageBox::warning(this, "⚠️ 错误", "请选择输入文件和输出目录！");
        return;
    }

    QFileInfo fi(i);
    QString out = o + "/" + fi.completeBaseName() + "." + ext;

    QStringList args;
    args << "-lang=ff";
    args << "-c";
    args << QString("Open('%1'); Generate('%2');").arg(i).arg(out);

    // ========== 🔥 开始转换，启动忙碌进度条 ==========
    m_progress->setMaximum(0); // 无限滚动模式
    m_progress->setValue(0);

    QProcess *p = new QProcess(this);
    p->start("fontforge", args);

    connect(p, &QProcess::finished, this, [=](int code) {
        // ========== 🔥 结束，恢复进度条并显示100% ==========
        m_progress->setMaximum(100);
        m_progress->setValue(100);

        bool ok = (code == 0 && QFile::exists(out));
        if (ok) {
            QMessageBox::information(this, "✅ 成功", "转换完成！\n" + out);
        } else {
            QMessageBox::critical(this, "❌ 失败", "错误输出：\n" + p->readAllStandardError());
        }

        // 重置进度条
        m_progress->reset();

        p->deleteLater();
    });
}

void MainWindow::contextMenuEvent(QContextMenuEvent *event)
{
    if (m_fileList->underMouse()) {
        m_fileMenu->exec(event->globalPos());
        event->accept();
    }
}
