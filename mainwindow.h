#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTreeWidget>
#include <QPushButton>
#include <QListWidget>
#include <QLineEdit>
#include <QComboBox>
#include <QGroupBox>
#include <QSlider>
#include <QLabel>
#include <QProgressBar>
#include <QMediaPlayer>
#include <QVideoWidget>
#include <QGraphicsVideoItem>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QAudioOutput>
#include <opencv2/opencv.hpp>
#include <QSvgRenderer>
#include <QPainter>
#include <QBuffer>
#include <QCheckBox>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QSpinBox>
#include <QCheckBox>
#include <QComboBox>


class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);

private slots:
    void applyAudioPreset();
    void addFile();
    void deleteSelectedFile();
    void clearList();
    void startBatchConvert();
    void playSelectedFile();
    void selectOutPath();
    void onMenuSelect(QTreeWidgetItem *item);
    void onSliderMoved(int val);
    void showFileListContextMenu(const QPoint &pos);
    void playAudioFile();
    void applyVideoPreset();
    void applyImagePreset();

private:
    void initUI();
    void initLeftMenu();
    void updateVideoPreview();  // 新增：刷新视频预览（滤镜、解码、渲染、时间）
    void applyVideoFilter();

    QTreeWidget     *m_leftTree;
    QListWidget     *m_fileList;
    QLineEdit       *m_editOutPath;
    QPushButton     *m_btnAdd;
    QPushButton     *m_btnDelete;
    QPushButton     *m_btnClear;
    QPushButton     *m_btnStart;
    QPushButton     *m_btnPlay;
    QProgressBar    *m_progress;

    QMediaPlayer    *m_player;
    QVideoWidget    *m_videoWidget;
    QPushButton     *m_btnPlayIn;

    QGroupBox       *grpAdv;
    QComboBox       *cmbResolution;
    QComboBox       *cmbFPS;
    QComboBox       *cmbCodec;
    QLineEdit       *edtCRF;
    QLineEdit       *edtWidth;
    QLineEdit       *edtHeight;
    QLineEdit       *edtVolume;
    QComboBox       *cmbDecode;
    QComboBox       *cmbRender;
    QComboBox       *cmbFilterPreset;
    QLineEdit       *editStartTime;
    QLineEdit       *editEndTime;
    QSlider         *sliderTrim;
    QComboBox *cmbQuality;
    QString getQualityArg();
    QGraphicsVideoItem *videoItem = nullptr;
    QGroupBox *grpAudioAdv;
    QComboBox *cmbPreset;

    // 音频高级配置
    QGroupBox *panelAudio;
    QComboBox *cmbSampleRate;
    QComboBox *cmbBitrate;
    QComboBox *cmbAudioChannel;
    QLineEdit *editAudioStart;
    QLineEdit *editAudioEnd;
    QMediaPlayer *audioPlayer;
    QAudioOutput *audioOutput;
    QSlider *audioSlider;
    QPushButton *btnAudioPlay;
    QLabel *lblAudioTime;
    bool m_audioLooping;
    QComboBox *cmbAudioCodec;
    QComboBox *cmbAudioFilter;
    QComboBox *cmbAudioPreset;

    QGroupBox *panelImage;
    QComboBox *cmbImageFormat;
    QLineEdit *editImgWidth;
    QLineEdit *editImgHeight;
    QComboBox *cmbImageQuality;
    QComboBox *cmbImagePreset;
    QComboBox *cmbRotate;   // 新增
    QComboBox *cmbFlip;
    QComboBox *cmbImageEffect;

    // ==========================
    // 字体高级面板（无错误版）
    // ==========================
    QGroupBox *panelFont;
    QDoubleSpinBox *spinFontSimplify;
    QSpinBox *spinFontCompress;
    QSpinBox *spinFontEm;
    QCheckBox *chkFontMeta;
    QCheckBox *chkFontFix;
    QCheckBox *chkFontAscii;
    QComboBox *cboFontPreset;

    // 字体配置参数
    double m_fontSimplify;
    int    m_fontCompress;
    int    m_fontEmUnit;
    bool   m_fontKeepMeta;
    bool   m_fontFixOutline;
    bool   m_fontAsciiOnly;

    QGroupBox *panelAbout; // 关于面板

    QString m_fmtArg;
    QString m_outSuffix;
};

#endif
