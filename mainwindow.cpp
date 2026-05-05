#include "mainwindow.h"
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
#include <QGraphicsVideoItem>
#include <QTimer>
#include <QGraphicsOpacityEffect>
#include <QTime>
#include <QTimer>
#include <QTime>
#include <QGraphicsBlurEffect>
#include <QGraphicsColorizeEffect>
#include <QGraphicsOpacityEffect>
#include <QAudioOutput>
#include <QSvgRenderer>
#include <QPainter>
#include <QBuffer>

MainWindow::MainWindow(QWidget *parent)
: QMainWindow(parent)
{
    setWindowTitle("万能格式转换器Linux");
    resize(1000, 750);
    initUI();
    initLeftMenu();
}

void MainWindow::initUI()
{
    setWindowIcon(QIcon::fromTheme("FormatFactory"));
    // ========================
    // 字体变量初始化
    // ========================
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

    // ====================== 视频高级配置 ======================
    grpAdv = new QGroupBox("视频高级配置");
    QVBoxLayout *advLayout = new QVBoxLayout(grpAdv);
    advLayout->setContentsMargins(12,12,12,12);
    advLayout->setSpacing(10);

    int labelCtrlSpace = 0;
    int itemSpacing    = 12;

    // ====================== 第一行 ======================
    QHBoxLayout *row1 = new QHBoxLayout;
    row1->setContentsMargins(0,0,0,0);

    cmbResolution = new QComboBox;
    cmbResolution->addItems({"原分辨率","360p","480p","720p","1080p","2K","4K"});
    cmbResolution->setFixedWidth(100);
    row1->addWidget(new QLabel("分辨率"));
    row1->addSpacing(labelCtrlSpace);
    row1->addWidget(cmbResolution);
    row1->addSpacing(itemSpacing);

    cmbFPS = new QComboBox;
    cmbFPS->setEditable(true);
    cmbFPS->addItems({"原帧率","视频帧率","24","30","60","90","120","144"});
    cmbFPS->setFixedWidth(85);
    row1->addWidget(new QLabel("帧率"));
    row1->addSpacing(labelCtrlSpace);
    row1->addWidget(cmbFPS);
    row1->addSpacing(itemSpacing);

    cmbCodec = new QComboBox;
    cmbCodec->addItems({"默认","视频编码","H.264","H.265","MPEG-4","VP9","Theora"});
    cmbCodec->setFixedWidth(105);
    row1->addWidget(new QLabel("编码"));
    row1->addSpacing(labelCtrlSpace);
    row1->addWidget(cmbCodec);
    row1->addSpacing(itemSpacing);

    edtCRF = new QLineEdit("23");
    edtCRF->setFixedWidth(55);
    row1->addWidget(new QLabel("CRF"));
    row1->addSpacing(labelCtrlSpace);
    row1->addWidget(edtCRF);
    row1->addSpacing(itemSpacing);

    edtWidth = new QLineEdit;
    edtWidth->setPlaceholderText("宽度");
    edtWidth->setFixedWidth(65);
    row1->addWidget(new QLabel("宽"));
    row1->addSpacing(labelCtrlSpace);
    row1->addWidget(edtWidth);
    row1->addSpacing(itemSpacing);

    edtHeight = new QLineEdit;
    edtHeight->setPlaceholderText("高度");
    edtHeight->setFixedWidth(65);
    row1->addWidget(new QLabel("高"));
    row1->addSpacing(labelCtrlSpace);
    row1->addWidget(edtHeight);
    row1->addSpacing(itemSpacing);

    edtVolume = new QLineEdit("1.0");
    edtVolume->setPlaceholderText("音量");
    edtVolume->setFixedWidth(55);
    row1->addWidget(new QLabel("音量"));
    row1->addSpacing(labelCtrlSpace);
    row1->addWidget(edtVolume);
    row1->addStretch();

    // ====================== 第二行 ======================
    QHBoxLayout *row2 = new QHBoxLayout;
    row2->setContentsMargins(0,0,0,0);

    cmbDecode = new QComboBox;
    cmbDecode->addItems({"默认解码","硬件解码","软件解码","FFmpeg解码"});
    cmbDecode->setFixedWidth(100);
    row2->addWidget(new QLabel("视频解码"));
    row2->addSpacing(labelCtrlSpace);
    row2->addWidget(cmbDecode);
    row2->addSpacing(itemSpacing);

    cmbRender = new QComboBox;
    cmbRender->addItems({"默认渲染","OpenGL","D3D","软件渲染"});
    cmbRender->setFixedWidth(90);
    row2->addWidget(new QLabel("视频渲染"));
    row2->addSpacing(labelCtrlSpace);
    row2->addWidget(cmbRender);
    row2->addSpacing(itemSpacing);

    cmbFilterPreset = new QComboBox;
    cmbFilterPreset->addItems({
        "无预设","锐化","模糊","黑白","复古","水平翻转","垂直翻转",
        "亮度+50","对比度+30","高斯模糊","镜像效果","灰度反转"
    });
    cmbFilterPreset->setFixedWidth(110);
    row2->addWidget(new QLabel("滤镜预设"));
    row2->addSpacing(labelCtrlSpace);
    row2->addWidget(cmbFilterPreset);
    row2->addStretch();

    // ====================== 第三行：时间 + 预览 ======================
    QHBoxLayout *row3 = new QHBoxLayout;
    row3->setContentsMargins(0,0,0,0);

    editStartTime = new QLineEdit("00:00:00");
    editEndTime = new QLineEdit("00:00:15");
    editStartTime->setFixedWidth(120);
    editEndTime->setFixedWidth(120);

    row3->addWidget(new QLabel("开始时间"));
    row3->addSpacing(labelCtrlSpace);
    row3->addWidget(editStartTime);
    row3->addSpacing(itemSpacing);

    row3->addWidget(new QLabel("结束时间"));
    row3->addSpacing(labelCtrlSpace);
    row3->addWidget(editEndTime);
    row3->addSpacing(itemSpacing);

    QPushButton *btnPlayIn = new QPushButton("▶ 实时预览");
    btnPlayIn->setFixedWidth(110);
    row3->addWidget(btnPlayIn);
    row3->addStretch();

    // ====================== 第四行：进度条 + 画质 + 快速预设 ======================
    QHBoxLayout *row4 = new QHBoxLayout;
    row4->setContentsMargins(0,0,0,0);

    sliderTrim = new QSlider(Qt::Horizontal);
    sliderTrim->setRange(0, 100);
    sliderTrim->setFixedWidth(106);

    cmbQuality = new QComboBox;
    cmbQuality->addItems({"低画质", "中等画质", "高画质", "超清", "无损画质"});
    cmbQuality->setCurrentIndex(3);
    cmbQuality->setFixedWidth(110);

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
    cmbPreset->setFixedWidth(120);

    row4->addWidget(new QLabel("进度条"));
    row4->addWidget(sliderTrim);
    row4->addSpacing(15);
    row4->addWidget(new QLabel("画质等级"));
    row4->addWidget(cmbQuality);
    row4->addSpacing(15);
    row4->addWidget(new QLabel("快速预设"));
    row4->addWidget(cmbPreset);
    row4->addStretch();

    // ====================== 加入布局 ======================
    advLayout->addLayout(row1);
    advLayout->addLayout(row2);
    advLayout->addLayout(row3);
    advLayout->addLayout(row4);

    // ====================== 悬浮视频播放器 ======================
    QGraphicsView *videoView = new QGraphicsView(grpAdv);
    videoView->setFixedSize(340, 120);
    videoView->setStyleSheet("background:transparent; border:none;");
    videoView->setFrameStyle(QFrame::NoFrame);
    videoView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    videoView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    videoView->move(570, 80);

    QGraphicsScene *scene = new QGraphicsScene(this);
    videoView->setScene(scene);

    videoItem = new QGraphicsVideoItem;
    scene->addItem(videoItem);
    videoItem->setSize(QSizeF(340, 120));

    m_player = new QMediaPlayer(this);
    QAudioOutput *videoAudio = new QAudioOutput(this);
    m_player->setAudioOutput(videoAudio);
    m_player->setVideoOutput(videoItem);

    // ====================== 绑定 ======================
    connect(btnPlayIn, &QPushButton::clicked, this, [this]() {
        updateVideoPreview();
    });

    connect(sliderTrim, &QSlider::sliderMoved, this, [this](qint64 pos) {
        m_player->setPosition(pos);
    });

    connect(cmbPreset, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::applyVideoPreset);

    connect(editStartTime, &QLineEdit::editingFinished, this, [this]() {
        updateVideoPreview();
    });
    connect(editEndTime, &QLineEdit::editingFinished, this, [this]() {
        updateVideoPreview();
    });

    connect(cmbFilterPreset, &QComboBox::currentTextChanged, this, [this]() {
        applyVideoFilter();
    });


    // ================== 音频高级配置 ==================
    panelAudio = new QGroupBox("音频高级配置");
    QVBoxLayout *lyAudio = new QVBoxLayout(panelAudio);
    lyAudio->setContentsMargins(12,12,12,12);
    lyAudio->setSpacing(10);

    // ================== 控件全部原样创建（不动代码）==================
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
    editAudioEnd   = new QLineEdit("00:00:15");
    editAudioStart->setFixedWidth(110);
    editAudioEnd->setFixedWidth(110);

    btnAudioPlay = new QPushButton("播放音频");
    audioSlider = new QSlider(Qt::Horizontal);
    lblAudioTime = new QLabel("00:00 / 00:00");
    audioSlider->setMinimumWidth(220);

    // ================== 第一排：编码 + 采样率 + 比特率 + 声道 + 音效 + 快速预设 ==================
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

    // ================== 第二排：时间裁剪 + 播放条 ==================
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

    // ================== 【图片转换配置】 ==================
    panelImage = new QGroupBox("图片转换配置");
    QVBoxLayout *lyImage = new QVBoxLayout(panelImage);
    lyImage->setContentsMargins(12,12,12,12);
    lyImage->setSpacing(10);

    // ================== 图片第一行（改名：imgRow1） ==================
    QHBoxLayout *imgRow1 = new QHBoxLayout;
    imgRow1->setSpacing(8);

    cmbImageFormat = new QComboBox;
    cmbImageFormat->addItems({"JPG","PNG","WebP","BMP","GIF","TIFF","JPEG","TIF","PPM","PGM","PBM","JP2"});
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

    // ================== 图片第二行（改名：imgRow2） ==================
    QHBoxLayout *imgRow2 = new QHBoxLayout;
    imgRow2->setSpacing(8);

    cmbImagePreset = new QComboBox;
    cmbImagePreset->addItems({"自定义","微信图","高清图","网页小图","超高压缩","无损PNG"});
    cmbImagePreset->setFixedWidth(140);
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
    cmbImageEffect->setFixedWidth(130);
    imgRow2->addWidget(new QLabel("图片特效"));
    imgRow2->addWidget(cmbImageEffect);
    imgRow2->addStretch();

    lyImage->addLayout(imgRow1);
    lyImage->addLayout(imgRow2);
    mainLayout->addWidget(panelImage);

    // 信号绑定
    connect(cmbImagePreset, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::applyImagePreset);
    connect(cmbImageFormat, &QComboBox::currentTextChanged, this, [this](const QString &text){
        m_outSuffix = text.toLower();
    });

    // ============================
    // 字体高级设置面板
    // ============================
    panelFont = new QGroupBox("字体高级设置", this);
    QVBoxLayout *lyFont = new QVBoxLayout(panelFont);
    lyFont->setContentsMargins(12,12,12,12);
    lyFont->setSpacing(10);

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
    spinFontSimplify->setSingleStep(0.1);
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
    fontLine2->setSpacing(10);

    chkFontMeta = new QCheckBox("保留版权", this);
    chkFontMeta->setChecked(true);

    chkFontFix = new QCheckBox("修复轮廓", this);
    chkFontFix->setChecked(true);

    chkFontAscii = new QCheckBox("仅英文", this);
    chkFontAscii->setChecked(false);

    fontLine2->addWidget(chkFontMeta);
    fontLine2->addWidget(chkFontFix);
    fontLine2->addWidget(chkFontAscii);
    fontLine2->addStretch();

    lyFont->addLayout(fontPresetLine);
    lyFont->addLayout(fontLine1);
    lyFont->addLayout(fontLine2);

    mainLayout->addWidget(panelFont);

    // ============================
    // 字体面板绑定
    // ============================
    connect(spinFontSimplify, &QDoubleSpinBox::valueChanged, this, [this](double v){
        m_fontSimplify = v;
    });
    connect(spinFontCompress, &QSpinBox::valueChanged, this, [this](int v){
        m_fontCompress = v;
    });
    connect(spinFontEm, &QSpinBox::valueChanged, this, [this](int v){
        m_fontEmUnit = v;
    });
    connect(chkFontMeta, &QCheckBox::toggled, this, [this](bool v){
        m_fontKeepMeta = v;
    });
    connect(chkFontFix, &QCheckBox::toggled, this, [this](bool v){
        m_fontFixOutline = v;
    });
    connect(chkFontAscii, &QCheckBox::toggled, this, [this](bool v){
        m_fontAsciiOnly = v;
    });

    connect(cboFontPreset, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int idx){
        if (idx == 1) {
            m_fontSimplify = 1.0;
            m_fontCompress = 6;
            m_fontEmUnit = 1000;
            m_fontKeepMeta = true;
            m_fontFixOutline = true;
            m_fontAsciiOnly = false;
        } else if (idx == 2) {
            m_fontSimplify = 2.0;
            m_fontCompress = 9;
            m_fontEmUnit = 1000;
            m_fontKeepMeta = false;
            m_fontFixOutline = true;
            m_fontAsciiOnly = false;
        } else if (idx == 3) {
            m_fontSimplify = 0.5;
            m_fontCompress = 6;
            m_fontEmUnit = 2048;
            m_fontKeepMeta = true;
            m_fontFixOutline = true;
            m_fontAsciiOnly = false;
        } else if (idx == 4) {
            m_fontSimplify = 1.5;
            m_fontCompress = 9;
            m_fontEmUnit = 1000;
            m_fontKeepMeta = true;
            m_fontFixOutline = true;
            m_fontAsciiOnly = true;
        }

        spinFontSimplify->setValue(m_fontSimplify);
        spinFontCompress->setValue(m_fontCompress);
        spinFontEm->setValue(m_fontEmUnit);
        chkFontMeta->setChecked(m_fontKeepMeta);
        chkFontFix->setChecked(m_fontFixOutline);
        chkFontAscii->setChecked(m_fontAsciiOnly);
    });

    // ==========================
    // 关于面板（完美跟随主题）
    // ==========================
    panelAbout = new QGroupBox("关于软件", this);
    QVBoxLayout *aboutLayout = new QVBoxLayout(panelAbout);

    aboutLayout->setAlignment(Qt::AlignCenter);
    aboutLayout->setSpacing(25);
    aboutLayout->setContentsMargins(40, 40, 40, 40);

    // 面板标题样式
    panelAbout->setStyleSheet(R"(
    QGroupBox::title {
        font-size: 22px;
        font-weight: bold;
        subcontrol-origin: margin;
        subcontrol-position: top center;
        padding: 10px;
    }
    )");

    // 标题
    QLabel *labTitle = new QLabel("万能格式转换器");
    labTitle->setAlignment(Qt::AlignCenter);
    labTitle->setStyleSheet("font-size: 30px; font-weight: bold;");
    aboutLayout->addWidget(labTitle);

    // 版本
    QLabel *labVer = new QLabel("版本：v1.0.1");
    labVer->setAlignment(Qt::AlignCenter);
    labVer->setStyleSheet("font-size: 20px;");
    aboutLayout->addWidget(labVer);

    // 作者
    QLabel *labAuthor = new QLabel("作者：Maoyaotang");
    labAuthor->setAlignment(Qt::AlignCenter);
    labAuthor->setStyleSheet("font-size: 20px;");
    aboutLayout->addWidget(labAuthor);

    // 引擎
    QLabel *labEngine = new QLabel("基于：FFmpeg + Qt + OpenCV + FontForge");
    labEngine->setAlignment(Qt::AlignCenter);
    labEngine->setStyleSheet("font-size: 18px;");
    aboutLayout->addWidget(labEngine);

    // ========== 可点击链接（自动跟随主题，无蓝色）==========
    QLabel *link = new QLabel("访问 GitHub 主页");
    link->setAlignment(Qt::AlignCenter);
    link->setStyleSheet("font-size: 20px;");
    link->setCursor(Qt::PointingHandCursor);

    // 正确的点击连接方式
    connect(link, &QLabel::linkActivated, this, [this](const QString&) {
        QDesktopServices::openUrl(QUrl("https://github.com/maoyaotang12"));
    });
    link->setText("<a href=\"#\">访问 GitHub 主页</a>");
    link->setOpenExternalLinks(false);

    aboutLayout->addWidget(link);
    aboutLayout->addStretch();
    mainLayout->addWidget(panelAbout);


    // 音频播放器
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
        //connect(cmbAudioPreset, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::applyAudioPreset);


    // ====================== 按钮 ======================
    QHBoxLayout *btnLayout = new QHBoxLayout;
    btnLayout->setSpacing(8);
    btnLayout->setContentsMargins(0,5,0,5);

    m_btnAdd = new QPushButton("添加文件");
    m_btnDelete = new QPushButton("删除选中");
    m_btnClear = new QPushButton("清空列表");
    m_btnStart = new QPushButton("开始转换");
    m_btnPlay = new QPushButton("播放选中");

    btnLayout->addWidget(m_btnAdd, 1);
    btnLayout->addWidget(m_btnDelete, 1);
    btnLayout->addWidget(m_btnClear, 1);
    btnLayout->addWidget(m_btnStart, 1);
    btnLayout->addWidget(m_btnPlay, 1);

    // ====================== 文件列表 ======================
    m_fileList = new QListWidget;
    m_fileList->setContextMenuPolicy(Qt::CustomContextMenu);

    // ====================== 输出路径 ======================
    QHBoxLayout *outLayout = new QHBoxLayout;
    m_editOutPath = new QLineEdit(QDir::homePath() + "/FormatFactory");
    QPushButton *btnOutPath = new QPushButton("输出目录");
    outLayout->addWidget(m_editOutPath);
    outLayout->addWidget(btnOutPath);

    m_progress = new QProgressBar;

    // ====================== 主布局 ======================
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

    // ====================== 信号绑定 ======================
    connect(m_btnAdd, &QPushButton::clicked, this, &MainWindow::addFile);
    connect(m_btnDelete, &QPushButton::clicked, this, &MainWindow::deleteSelectedFile);
    connect(m_btnClear, &QPushButton::clicked, this, &MainWindow::clearList);
    connect(m_btnStart, &QPushButton::clicked, this, &MainWindow::startBatchConvert);
    connect(m_btnPlay, &QPushButton::clicked, this, &MainWindow::playSelectedFile);
    connect(m_leftTree, &QTreeWidget::itemClicked, this, &MainWindow::onMenuSelect);
    connect(btnOutPath, &QPushButton::clicked, this, &MainWindow::selectOutPath);
    connect(m_fileList, &QListWidget::customContextMenuRequested, this, &MainWindow::showFileListContextMenu);
    connect(sliderTrim, &QSlider::valueChanged, this, &MainWindow::onSliderMoved);

    grpAdv->hide();
    panelAudio->hide();
    panelImage->hide();
    panelFont->hide();
}

void MainWindow::clearList()
{
    m_fileList->clear();
}

void MainWindow::onSliderMoved(int val)
{
    int totalSec = 60;
    int nowSec = totalSec * val / 100;
    int h = nowSec / 3600;
    int m = (nowSec % 3600) / 60;
    int s = nowSec % 60;
    editStartTime->setText(QString("%1:%2:%3")
    .arg(h,2,10,QChar('0'))
    .arg(m,2,10,QChar('0'))
    .arg(s,2,10,QChar('0')));
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

// ====================== 视频（全部格式 100% 补齐）======================
QTreeWidgetItem *videoRoot = new QTreeWidgetItem(m_leftTree);
videoRoot->setIcon(0, icoVideo);
videoRoot->setText(0, "视频转换");
QStringList vlist = {
    "转为 MP4",
    "转为 MP4 高清",
    "转为 MP4 超清",
    "转为 MKV",
    "转为 AVI",
    "转为 MOV",
    "转为 FLV",
    "转为 WMV",
    "转为 WEBM",
    "转为 MPEG",
    "转为 MPG",
    "转为 3GP",
    "转为 3GP 高清",
    "转为 3GP 超清",
    "转为 3G2",
    "转为 3G2 高清",
    "转为 AMV",
    "转为 OGV",
    "转为 TS",
    "转为 M2TS",
    "转为 VOB",
    "转为 ASF",
    "转为 M4V",
    "转为 F4V",
    "转为 RM",
    "转为 RMVB",
    "转为 DAT",
    "转为 GIF"
};
for (const QString& s : vlist) {
    QTreeWidgetItem *item = new QTreeWidgetItem(videoRoot);
    item->setText(0, s);
}

// ====================== 音频（完整）======================
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

// ====================== 图片（完整）======================
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

// ====================== 字体（新增）======================
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
// ======================================
// ✅ 就在这里加！！！！！！！！！！！！！
// ======================================
new QTreeWidgetItem(m_leftTree, QStringList() << "关于软件");

videoRoot->setExpanded(true);
audioRoot->setExpanded(true);
imageRoot->setExpanded(true);
fontRoot->setExpanded(true);

// 面板切换
connect(m_leftTree, &QTreeWidget::itemClicked, this, [this](QTreeWidgetItem* item, int) {
    if (!item || !item->parent()) return;

    grpAdv->hide();
    panelAudio->hide();
    panelImage->hide();
    panelFont->hide();

    if (panelAbout) {
        panelAbout->hide();
    }

    QString t = item->parent()->text(0);
    if (t == "视频转换")       grpAdv->show();
    else if (t == "音频转换")  panelAudio->show();
    else if (t == "图片转换")  panelImage->show();
    else if (t == "字体转换")  panelFont->show();
    else if (t == "关于软件")  panelAbout->show();

});
}

void MainWindow::onMenuSelect(QTreeWidgetItem *item)
{
    if (!item) return; // 【加这句】防止空指针闪退

    QTreeWidgetItem *root = item;
    while (root->parent())
        root = root->parent();

    // 2. 判断当前选中了什么
    bool isVideo = (root->text(0) == "视频转换");
    bool isAudio = (root->text(0) == "音频转换");
    bool isImage = (root->text(0) == "图片转换");
    bool isFont  = (root->text(0) == "字体转换");

    // ==============================================
    // ✅ 强制全部关闭！一个不留 + 安全关闭关于面板
    // ==============================================
    grpAdv->setVisible(false);
    panelAudio->setVisible(false);
    panelImage->setVisible(false);
    panelFont->setVisible(false);

    if (panelAbout) {
        panelAbout->setVisible(false);
    }

    // ==============================================
    // ✅【关键】点击 关于软件 → 显示关于，直接返回
    // ==============================================
    QString currentItemText = item->text(0);
    if (currentItemText == "关于软件") {
        if (panelAbout) {
            panelAbout->setVisible(true);
        }
        return; // 不执行后面的转换逻辑
    }

    // ==============================================
    // ✅ 只打开当前需要的
    // ==============================================
    if (isFont) {
        panelFont->setVisible(true);
    }
    else if (isVideo) {
        grpAdv->setVisible(true);
    }
    else if (isAudio) {
        panelAudio->setVisible(true);
    }
    else if (isImage) {
        panelImage->setVisible(true);
    }

    // ==============================================
    // 下面是你原来的所有格式逻辑，完全不动
    // ==============================================
    QString txt = item->text(0);
    m_fmtArg.clear();
    m_outSuffix.clear();

    // ================== 视频 ==================
    if (txt == "转为 MP4") {
        m_fmtArg = "-c:v libx264 -crf 24 -c:a aac -y";
        m_outSuffix = "mp4";
    }
    else if (txt == "转为 MP4 高清") {
        m_fmtArg = "-c:v libx264 -s 1280x720 -b:v 2500k -c:a aac -b:a 192k -y";
        m_outSuffix = "mp4";
    }
    else if (txt == "转为 MP4 超清") {
        m_fmtArg = "-c:v libx264 -s 1920x1080 -b:v 5000k -c:a aac -b:a 192k -y";
        m_outSuffix = "mp4";
    }
    else if (txt == "转为 MKV") {
        m_fmtArg = "-c:v libx264 -crf 24 -c:a aac -y";
        m_outSuffix = "mkv";
    }
    else if (txt == "转为 AVI") {
        m_fmtArg = "-c:v libxvid -qscale 4 -c:a mp3 -b:a 128k -y";
        m_outSuffix = "avi";
    }
    else if (txt == "转为 MOV") {
        m_fmtArg = "-c:v libx264 -crf 24 -c:a aac -y";
        m_outSuffix = "mov";
    }
    else if (txt == "转为 FLV") {
        m_fmtArg = "-c:v libx264 -c:a aac -f flv -y";
        m_outSuffix = "flv";
    }
    else if (txt == "转为 WMV") {
        m_fmtArg = "-c:v wmv2 -b:v 1500k -c:a wmav2 -b:a 128k -y";
        m_outSuffix = "wmv";
    }
    else if (txt == "转为 WEBM") {
        m_fmtArg = "-c:v libvpx -c:a libvorbis -y";
        m_outSuffix = "webm";
    }
    else if (txt == "转为 MPEG") {
        m_fmtArg = "-c:v mpeg2video -b:v 2000k -c:a mp2 -b:a 128k -y";
        m_outSuffix = "mpeg";
    }
    else if (txt == "转为 MPG") {
        m_fmtArg = "-c:v mpeg2video -b:v 2000k -c:a mp2 -b:a 128k -y";
        m_outSuffix = "mpg";
    }
    else if (txt == "转为 3GP") {
        m_fmtArg = "-f 3gp -s 320x240 -b:v 128k -c:a amr_nb -y";
        m_outSuffix = "3gp";
    }
    else if (txt == "转为 3GP 高清") {
        m_fmtArg = "-f 3gp -s 480x360 -b:v 384k -c:a aac -y";
        m_outSuffix = "3gp";
    }
    else if (txt == "转为 3GP 超清") {
        m_fmtArg = "-f 3gp -s 720x480 -b:v 768k -c:a aac -y";
        m_outSuffix = "3gp";
    }
    else if (txt == "转为 3G2") {
        m_fmtArg = "-f 3gp -s 320x240 -b:v 128k -c:a aac -y";
        m_outSuffix = "3g2";
    }
    else if (txt == "转为 3G2 高清") {
        m_fmtArg = "-f 3gp -s 480x360 -b:v 384k -c:a aac -y";
        m_outSuffix = "3g2";
    }
    else if (txt == "转为 AMV") {
        m_fmtArg = "-f amv -s 160x120 -c:a adpcm_ima_amv -y";
        m_outSuffix = "amv";
    }
    else if (txt == "转为 OGV") {
        m_fmtArg = "-c:v libtheora -c:a libvorbis -y";
        m_outSuffix = "ogv";
    }
    else if (txt == "转为 TS") {
        m_fmtArg = "-c:v libx264 -c:a aac -f mpegts -y";
        m_outSuffix = "ts";
    }
    else if (txt == "转为 M2TS") {
        m_fmtArg = "-c:v libx264 -c:a aac -f mpegts -y";
        m_outSuffix = "m2ts";
    }
    else if (txt == "转为 VOB") {
        m_fmtArg = "-c:v mpeg2video -b:v 4000k -c:a ac3 -b:a 128k -y";
        m_outSuffix = "vob";
    }
    else if (txt == "转为 ASF") {
        m_fmtArg = "-c:v msmpeg4v2 -b:v 1000k -c:a mp3 -b:a 128k -y";
        m_outSuffix = "asf";
    }
    else if (txt == "转为 M4V") {
        m_fmtArg = "-c:v libx264 -crf 24 -c:a aac -y";
        m_outSuffix = "m4v";
    }
    else if (txt == "转为 F4V") {
        m_fmtArg = "-c:v libx264 -c:a aac -f f4v -y";
        m_outSuffix = "f4v";
    }
    else if (txt == "转为 RM") {
        m_fmtArg = "-c:v h263 -b:v 800k -c:a aac -y";
        m_outSuffix = "rm";
    }
    else if (txt == "转为 RMVB") {
        m_fmtArg = "-c:v libx264 -b:v 1500k -c:a aac -y";
        m_outSuffix = "rmvb";
    }
    else if (txt == "转为 DAT") {
        m_fmtArg = "-c:v mpeg1video -b:v 1500k -c:a mp2 -y";
        m_outSuffix = "dat";
    }
    else if (txt == "转为 GIF") {
        m_fmtArg = "-f gif -loop 0 -y";
        m_outSuffix = "gif";
    }

    // ================== 音频 ==================
    else if (txt == "转为 MP3")  { m_fmtArg = "-vn -c:a libmp3lame -b:a 192k -y"; m_outSuffix = "mp3"; }
    else if (txt == "转为 WAV")  { m_fmtArg = "-vn -c:a pcm_s16le -y"; m_outSuffix = "wav"; }
    else if (txt == "转为 FLAC") { m_fmtArg = "-vn -c:a flac -compression_level 6 -y"; m_outSuffix = "flac"; }
    else if (txt == "转为 AAC")  { m_fmtArg = "-vn -c:a aac -b:a 192k -y"; m_outSuffix = "aac"; }
    else if (txt == "转为 M4A")  { m_fmtArg = "-vn -c:a alac -y"; m_outSuffix = "m4a"; }
    else if (txt == "转为 OGG")  { m_fmtArg = "-vn -c:a libvorbis -b:a 160k -y"; m_outSuffix = "ogg"; }
    else if (txt == "转为 WMA")  { m_fmtArg = "-vn -c:a wmav2 -b:a 192k -y"; m_outSuffix = "wma"; }
    else if (txt == "转为 ALAC") { m_fmtArg = "-vn -c:a alac -y"; m_outSuffix = "alac"; }
    else if (txt == "转为 AC3")  { m_fmtArg = "-vn -c:a ac3 -b:a 192k -y"; m_outSuffix = "ac3"; }
    else if (txt == "转为 DTS")  { m_fmtArg = "-vn -c:a dca -b:a 192k -y"; m_outSuffix = "dts"; }
    else if (txt == "转为 OPUS") { m_fmtArg = "-vn -c:a libopus -b:a 160k -y"; m_outSuffix = "opus"; }
    else if (txt == "转为 AMR")  { m_fmtArg = "-vn -c:a amr_nb -y"; m_outSuffix = "amr"; }
    else if (txt == "转为 MKA")  { m_fmtArg = "-vn -c:a copy -y"; m_outSuffix = "mka"; }
    else if (txt == "转为 AIFF") { m_fmtArg = "-vn -c:a pcm_s16be -y"; m_outSuffix = "aiff"; }
    else if (txt == "转为 CAF")  { m_fmtArg = "-vn -c:a alac -y"; m_outSuffix = "caf"; }

    // ================== 图片 ==================
    else if (isImage)
    {
        QString fmt = txt.replace("转为 ", "").toLower();
        m_outSuffix = fmt;
        m_fmtArg = "USE_OPENCV";

        QString displayFmt = fmt;
        if (displayFmt == "jpg")      displayFmt = "JPG";
        else if (displayFmt == "jpeg") displayFmt = "JPEG";
        else if (displayFmt == "png")  displayFmt = "PNG";
        else if (displayFmt == "webp") displayFmt = "WebP";
        else if (displayFmt == "bmp")  displayFmt = "BMP";
        else if (displayFmt == "gif")  displayFmt = "GIF";
        else if (displayFmt == "tiff") displayFmt = "TIFF";
        else if (displayFmt == "tif") displayFmt = "TIF";
        else if (displayFmt == "ppm") displayFmt = "PPM";
        else if (displayFmt == "pgm") displayFmt = "PGM";
        else if (displayFmt == "pbm") displayFmt = "PBM";
        else if (displayFmt == "jp2") displayFmt = "JP2";

        int idx = cmbImageFormat->findText(displayFmt);
        if (idx != -1) {
            cmbImageFormat->setCurrentIndex(idx);
        }
    }

    // ================== 字体（真实参数）==================
    else if (isFont)
    {
        if (txt == "TTF→OTF") {
            m_fmtArg = "-c \"Open('%1'); Generate('%2');\"";
            m_outSuffix = "otf";
        }
        else if (txt == "OTF→TTF") {
            m_fmtArg = "-c \"Open('%1'); Generate('%2');\"";
            m_outSuffix = "ttf";
        }
        else if (txt == "WOFF→TTF") {
            m_fmtArg = "-c \"Open('%1'); Generate('%2');\"";
            m_outSuffix = "ttf";
        }
        else if (txt == "WOFF2→TTF") {
            m_fmtArg = "-c \"Open('%1'); Generate('%2');\"";
            m_outSuffix = "ttf";
        }
        else if (txt == "TTF→WOFF") {
            m_fmtArg = "-c \"Open('%1'); Generate('%2');\"";
            m_outSuffix = "woff";
        }
        else if (txt == "TTF→WOFF2") {
            m_fmtArg = "-c \"Open('%1'); Generate('%2');\"";
            m_outSuffix = "woff2";
        }
        else if (txt == "EOT→TTF") {
            m_fmtArg = "-c \"Open('%1'); Generate('%2');\"";
            m_outSuffix = "ttf";
        }
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

void MainWindow::startBatchConvert()
{
    if (m_fileList->count() == 0 || m_outSuffix.isEmpty()) {
        QMessageBox::warning(this, "提示", "请添加文件并选择格式");
        return;
    }

    m_btnStart->setEnabled(false);
    m_progress->setValue(10);

    for (int i = 0; i < m_fileList->count(); ++i) {
        QString src = m_fileList->item(i)->text();
        QFileInfo fi(src);
        QString out = m_editOutPath->text() + "/" + fi.baseName() + "." + m_outSuffix;

        // ========================== OPENCV 图片转换（全格式完整版）==========================
        if (m_fmtArg == "USE_OPENCV")
        {
            cv::Mat img = cv::imread(src.toStdString());
            if (img.empty()) {
                continue;
            }

            cv::Mat dst = img.clone();

            // 旋转
            QString rot = cmbRotate->currentText();
            if (rot == "90°")      cv::rotate(dst, dst, cv::ROTATE_90_CLOCKWISE);
            else if (rot == "180°") cv::rotate(dst, dst, cv::ROTATE_180);
            else if (rot == "270°") cv::rotate(dst, dst, cv::ROTATE_90_COUNTERCLOCKWISE);

            // 翻转
            QString flip = cmbFlip->currentText();
            if (flip == "水平") cv::flip(dst, dst, 1);
            if (flip == "垂直") cv::flip(dst, dst, 0);

            // 缩放
            int w = editImgWidth->text().toInt();
            int h = editImgHeight->text().toInt();
            if (w > 0 && h > 0) {
                cv::resize(dst, dst, cv::Size(w, h));
            }

            // ====================== 图片特效 ======================
            QString effect = cmbImageEffect->currentText();

            if (effect == "黑白") {
                cv::cvtColor(dst, dst, cv::COLOR_BGR2GRAY);
                cv::cvtColor(dst, dst, cv::COLOR_GRAY2BGR);
            }
            else if (effect == "灰度") {
                cv::cvtColor(dst, dst, cv::COLOR_BGR2GRAY);
                cv::cvtColor(dst, dst, cv::COLOR_GRAY2BGR);
            }
            else if (effect == "反色") {
                cv::bitwise_not(dst, dst);
            }
            else if (effect == "锐化") {
                cv::Mat kernel = (cv::Mat_<float>(3,3) << 0, -1, 0, -1, 5.5, -1, 0, -1, 0);
                cv::filter2D(dst, dst, -1, kernel);
            }
            else if (effect == "轻度模糊") {
                cv::blur(dst, dst, cv::Size(3, 3));
            }
            else if (effect == "高斯模糊") {
                cv::GaussianBlur(dst, dst, cv::Size(5, 5), 0);
            }
            else if (effect == "复古") {
                cv::Mat ycrcb;
                cv::cvtColor(dst, ycrcb, cv::COLOR_BGR2YCrCb);
                std::vector<cv::Mat> channels;
                cv::split(ycrcb, channels);
                channels[1] = channels[1] * 0.7 + 20;
                channels[2] = channels[2] * 0.6 + 40;
                cv::merge(channels, ycrcb);
                cv::cvtColor(ycrcb, dst, cv::COLOR_YCrCb2BGR);
            }
            else if (effect == "亮度+10%") {
                dst.convertTo(dst, -1, 1.0, 25);
            }
            else if (effect == "亮度+20%") {
                dst.convertTo(dst, -1, 1.0, 50);
            }
            else if (effect == "对比度+20%") {
                dst.convertTo(dst, -1, 1.2, 0);
            }

            // ====================== 全格式输出 ======================
            if (m_outSuffix == "jpg" || m_outSuffix == "jpeg" || m_outSuffix == "jpe" || m_outSuffix == "jif" || m_outSuffix == "jfif") {
                int q = 95;
                QString qLevel = cmbImageQuality->currentText();
                if (qLevel == "低画质") q = 50;
                else if (qLevel == "中等") q = 75;
                else if (qLevel == "高画质") q = 90;
                else if (qLevel == "无损") q = 100;
                std::vector<int> p = {cv::IMWRITE_JPEG_QUALITY, q};
                cv::imwrite(out.toStdString(), dst, p);
            }
            else if (m_outSuffix == "png") {
                std::vector<int> p = {cv::IMWRITE_PNG_COMPRESSION, 1};
                cv::imwrite(out.toStdString(), dst, p);
            }
            else if (m_outSuffix == "webp") {
                int q = 90;
                QString qLevel = cmbImageQuality->currentText();
                if (qLevel == "低画质") q = 40;
                else if (qLevel == "中等") q = 60;
                else if (qLevel == "高画质") q = 85;
                else if (qLevel == "无损") q = 100;
                std::vector<int> p = {cv::IMWRITE_WEBP_QUALITY, q};
                cv::imwrite(out.toStdString(), dst, p);
            }
            else if (m_outSuffix == "bmp") {
                cv::imwrite(out.toStdString(), dst);
            }
            else if (m_outSuffix == "tif" || m_outSuffix == "tiff") {
                std::vector<int> p = {cv::IMWRITE_TIFF_COMPRESSION, 1};
                cv::imwrite(out.toStdString(), dst, p);
            }
            else if (m_outSuffix == "ppm" || m_outSuffix == "pgm" || m_outSuffix == "pbm") {
                cv::imwrite(out.toStdString(), dst);
            }
            else if (m_outSuffix == "jp2") {
                cv::imwrite(out.toStdString(), dst);
            }
            else if (m_outSuffix == "sr" || m_outSuffix == "ras") {
                cv::imwrite(out.toStdString(), dst);
            }
            else {
                cv::imwrite(out.toStdString(), dst);
            }

            continue;
        }


        QStringList args;
        args << "-y" << "-i" << src;

        if (grpAdv->isVisible()) {
            QString sVid = editStartTime->text().trimmed();
            QString eVid = editEndTime->text().trimmed();

            bool sValid = QTime::fromString(sVid, "hh:mm:ss").isValid();
            bool eValid = QTime::fromString(eVid, "hh:mm:ss").isValid();

            if (sValid && eValid)
            {
                QTime t1 = QTime::fromString(sVid, "hh:mm:ss");
                QTime t2 = QTime::fromString(eVid, "hh:mm:ss");

                // 结束时间00:00:00 只留开始
                if (eVid == "00:00:00")
                {
                    args << "-ss" << sVid;
                }
                // 开始大于结束 自动互换
                else if (t1 > t2)
                {
                    args << "-ss" << eVid << "-to" << sVid;
                }
                else
                {
                    args << "-ss" << sVid << "-to" << eVid;
                }
            }

            QString r = cmbResolution->currentText();
            if (r == "360p") args << "-s" << "640x360";
            else if (r == "480p") args << "-s" << "854x480";
            else if (r == "720p") args << "-s" << "1280x720";
            else if (r == "1080p") args << "-s" << "1920x1080";
            else if (r == "2K") args << "-s" << "2560x1440";
            else if (r == "4K") args << "-s" << "3840x2160";

            int w = edtWidth->text().toInt();
            int h = edtHeight->text().toInt();
            if (w > 0 && h > 0) args << "-s" << QString("%1x%2").arg(w).arg(h);

            QString c = cmbCodec->currentText();
            if (c == "H.264") args << "-c:v" << "libx264";
            if (c == "H.265") args << "-c:v" << "libx265";
            if (c == "MPEG-4") args << "-c:v" << "libxvid";
            if (c == "VP9") args << "-c:v" << "libvpx";
            if (c == "Theora") args << "-c:v" << "libtheora";

            QString fps = cmbFPS->currentText();
            if (fps != "原帧率" && fps != "视频帧率") args << "-r" << fps;

            if (!edtCRF->text().isEmpty()) args << "-crf" << edtCRF->text();
            if (!edtVolume->text().isEmpty()) args << "-filter:a" << QString("volume=%1").arg(edtVolume->text());

            QString pre = cmbFilterPreset->currentText();
            QString filter;
            if (pre == "锐化") filter = "unsharp=5:5:1.0";
            else if (pre == "模糊") filter = "boxblur=2:2";
            else if (pre == "黑白") filter = "colorchannelmixer=.3:.4:.3:0:.3:.4:.3:0:.3:.4:.3";
            else if (pre == "复古") filter = "curves=vintage";
            else if (pre == "水平翻转") filter = "hflip";
            else if (pre == "垂直翻转") filter = "vflip";
            else if (pre == "亮度+50") filter = "eq=brightness=0.5";
            else if (pre == "对比度+30") filter = "eq=contrast=1.3";
            else if (pre == "高斯模糊") filter = "gblur=2";
            else if (pre == "镜像效果") filter = "split[left][right];[left]hflip[left];[left][right]hstack";
            else if (pre == "灰度反转") filter = "negate";
            if (!filter.isEmpty()) args << "-vf" << filter;
        }

        // ================== 音频高级参数生效 ==================
        if (panelAudio->isVisible()) {
            // 时间裁剪
            QString sAud = editAudioStart->text().trimmed();
            QString eAud = editAudioEnd->text().trimmed();

            bool sAudOk = QTime::fromString(sAud, "hh:mm:ss").isValid();
            bool eAudOk = QTime::fromString(eAud, "hh:mm:ss").isValid();

            if (sAudOk && eAudOk)
            {
                QTime t1 = QTime::fromString(sAud, "hh:mm:ss");
                QTime t2 = QTime::fromString(eAud, "hh:mm:ss");

                if (eAud == "00:00:00")
                {
                    args << "-ss" << sAud;
                }
                else if (t1 > t2)
                {
                    args << "-ss" << eAud << "-to" << sAud;
                }
                else
                {
                    args << "-ss" << sAud << "-to" << eAud;
                }
            }

            // 采样率
            args << "-ar" << cmbSampleRate->currentText();

            // 比特率
            args << "-b:a" << cmbBitrate->currentText();

            // 声道
            if (cmbAudioChannel->currentText() == "单声道") {
                args << "-ac" << "1";
            } else {
                args << "-ac" << "2";
            }
        }

        args.append(m_fmtArg.split(" ", Qt::SkipEmptyParts));
        args << out;

        QProcess p;
        p.start("ffmpeg", args);
        p.waitForFinished(-1);

        m_progress->setValue((i+1)*100/m_fileList->count());
    }

    QMessageBox::information(this, "完成", "转换成功！");
    m_progress->setValue(0);
    m_btnStart->setEnabled(true);
}

// ====================== 刷新视频预览 ======================
void MainWindow::updateVideoPreview()
{
    if (m_fileList->selectedItems().isEmpty())
        return;

    // 彻底重置播放器
    m_player->stop();
    m_player->setSource(QUrl());
    disconnect(m_player, nullptr, this, nullptr);

    QString path = m_fileList->selectedItems().first()->text();
    m_player->setSource(QUrl::fromLocalFile(path));

    // 视频加载完成
    connect(m_player, &QMediaPlayer::mediaStatusChanged, this, [this](QMediaPlayer::MediaStatus status) {
        if (status == QMediaPlayer::LoadedMedia || status == QMediaPlayer::BufferedMedia) {
            disconnect(m_player, nullptr, this, nullptr);

            // 设置进度条范围
            qint64 duration = m_player->duration();
            sliderTrim->setRange(0, duration);

            // 应用翻转、滤镜
            applyVideoFilter();

            // -------------- 【开始时间】自动跳转 --------------
            QTime start = QTime::fromString(editStartTime->text(), "HH:mm:ss");
            qint64 startMs = start.msecsSinceStartOfDay();
            if (startMs > duration) startMs = 0;

            m_player->setPosition(startMs);
            m_player->play();

            // -------------- 进度条实时同步 --------------
            connect(m_player, &QMediaPlayer::positionChanged, this, [this](qint64 pos) {
                sliderTrim->blockSignals(true);
                sliderTrim->setValue(pos);
                sliderTrim->blockSignals(false);

                // -------------- 【结束时间】自动限制 --------------
                QTime end = QTime::fromString(editEndTime->text(), "HH:mm:ss");
                qint64 endMs = end.msecsSinceStartOfDay();
                qint64 duration = m_player->duration();

                if (endMs <= 0 || endMs > duration)
                    endMs = duration;

                // 到达结束时间 → 跳回开始时间循环播放
                if (pos >= endMs) {
                    QTime s = QTime::fromString(editStartTime->text(), "HH:mm:ss");
                    qint64 sMs = s.msecsSinceStartOfDay();
                    m_player->setPosition(sMs);
                }
            });
        }
    });
}

// ====================== 应用滤镜 ======================
void MainWindow::applyVideoFilter()
{
    if (!videoItem) return;

    static QGraphicsEffect *currentEffect = nullptr;
    delete currentEffect;
    currentEffect = nullptr;

    QString f = cmbFilterPreset->currentText();

    // ====================== 翻转（必生效）======================
    QTransform transform;
    transform.reset();

    if (f == "水平翻转") {
        transform.scale(-1, 1);
    }
    else if (f == "垂直翻转") {
        transform.scale(1, -1);
    }
    else if (f == "水平+垂直翻转") {
        transform.scale(-1, -1);
    }

    videoItem->setTransform(transform);

    // ====================== 滤镜 ======================
    if (f == "黑白") {
        QGraphicsColorizeEffect *e = new QGraphicsColorizeEffect;
        e->setColor(Qt::black);
        e->setStrength(1);
        currentEffect = e;
    }
    else if (f == "模糊") {
        QGraphicsBlurEffect *e = new QGraphicsBlurEffect;
        e->setBlurRadius(3);
        currentEffect = e;
    }
    else if (f == "复古") {
        QGraphicsColorizeEffect *e = new QGraphicsColorizeEffect;
        e->setColor(QColor(140, 80, 20));
        e->setStrength(0.4);
        currentEffect = e;
    }
    else if (f == "亮度+50") {
        QGraphicsOpacityEffect *e = new QGraphicsOpacityEffect;
        e->setOpacity(1.5);
        currentEffect = e;
    }
    else if (f == "对比度+30") {
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
    if (level == "低画质")
        return "-preset fast -crf 30 -b:v 0";
    else if (level == "中等画质")
        return "-preset medium -crf 24 -b:v 0";
    else if (level == "高画质")
        return "-preset slow -crf 20 -b:v 0";
    else if (level == "超清")
        return "-preset slow -crf 18 -b:v 0";
    else if (level == "无损画质")
        return "-preset veryslow -crf 16 -b:v 0";

    // 默认超清
    return "-preset slow -crf 18 -b:v 0";
}

void MainWindow::playAudioFile()
{
    if (!m_fileList->currentItem()) return;
    QString path = m_fileList->currentItem()->text();

    // 强制重置播放器
    audioPlayer->stop();
    audioPlayer->setSource(QUrl());

    // 重新设置文件
    audioPlayer->setSource(QUrl::fromLocalFile(path));

    // 取出开始时间
    QTime sTime = QTime::fromString(editAudioStart->text(), "hh:mm:ss");
    qint64 startMs = (sTime.hour()*3600 + sTime.minute()*60 + sTime.second()) * 1000;

    // 加载后立即跳转并播放
    connect(audioPlayer, &QMediaPlayer::sourceChanged, this, [=]() {
        QTimer::singleShot(50, this, [=]() {
            audioPlayer->setPosition(startMs);
            audioPlayer->play();
        });
    }, Qt::SingleShotConnection);
}

// ================== 视频预设自动应用（稳步扩展，不影响任何功能） ==================
void MainWindow::applyVideoPreset()
{
    // 获取当前选择的预设
    QString preset = cmbPreset->currentText();

    // 自定义：不改动任何参数
    if (preset == "自定义") {
        return;
    }

    // 抖音短视频
    else if (preset == "抖音短视频") {
        cmbResolution->setCurrentText("720p");
        cmbFPS->setCurrentText("30");
        cmbCodec->setCurrentText("H.264");
        edtCRF->setText("24");
        edtVolume->setText("1.0");
    }

    // B站超清
    else if (preset == "B站超清") {
        cmbResolution->setCurrentText("1080p");
        cmbFPS->setCurrentText("60");
        cmbCodec->setCurrentText("H.265");
        edtCRF->setText("23");
    }

    // 高质量小体积
    else if (preset == "高质量小体积") {
        cmbCodec->setCurrentText("H.265");
        edtCRF->setText("26");
    }

    // 录屏专用
    else if (preset == "录屏专用") {
        cmbFPS->setCurrentText("30");
        cmbCodec->setCurrentText("H.264");
        edtCRF->setText("28");
    }

    // 动漫专用
    else if (preset == "动漫专用") {
        cmbCodec->setCurrentText("H.265");
        edtCRF->setText("25");
    }

    // 电影压制
    else if (preset == "电影压制") {
        cmbCodec->setCurrentText("H.265");
        edtCRF->setText("22");
    }

    // 超快压缩
    else if (preset == "超快压缩") {
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
    }
    else if (preset == "标准音乐") {
        cmbAudioCodec->setCurrentText("MP3");
        cmbSampleRate->setCurrentText("44100");
        cmbBitrate->setCurrentText("320k");
    }
    else if (preset == "高清无损") {
        cmbAudioCodec->setCurrentText("FLAC无损");
        cmbSampleRate->setCurrentText("48000");
        cmbBitrate->setCurrentText("1000k");
    }
    else if (preset == "语音录音") {
        cmbAudioCodec->setCurrentText("AAC");
        cmbSampleRate->setCurrentText("32000");
        cmbBitrate->setCurrentText("96k");
        cmbAudioChannel->setCurrentText("单声道");
    }
    else if (preset == "极速压缩") {
        cmbAudioCodec->setCurrentText("MP3");
        cmbBitrate->setCurrentText("64k");
    }
}

void MainWindow::applyImagePreset()
{
    QString p = cmbImagePreset->currentText();

    if (p == "微信图") {
        cmbImageFormat->setCurrentText("JPG");
        editImgWidth->setText("400");
        editImgHeight->setText("400");
        cmbImageQuality->setCurrentIndex(1);
        cmbRotate->setCurrentIndex(0);
        cmbFlip->setCurrentIndex(0);
    }
    else if (p == "高清图") {
        cmbImageFormat->setCurrentText("PNG");
        editImgWidth->clear();
        editImgHeight->clear();
        cmbImageQuality->setCurrentIndex(3);
    }
    else if (p == "网页小图") {
        cmbImageFormat->setCurrentText("WebP");
        cmbImageQuality->setCurrentIndex(1);
    }
    else if (p == "超高压缩") {
        cmbImageFormat->setCurrentText("WebP");
        cmbImageQuality->setCurrentIndex(0);
    }
    else if (p == "无损PNG") {
        cmbImageFormat->setCurrentText("PNG");
        cmbImageQuality->setCurrentIndex(3);
    }
}
