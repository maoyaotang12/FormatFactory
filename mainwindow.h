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
#include <QGraphicsVideoItem>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QAudioOutput>
#include <opencv2/opencv.hpp>
#include <QCheckBox>
#include <QDoubleSpinBox>
#include <QSpinBox>
#include <QMenu>
#include <QContextMenuEvent>

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
    //void startBatchConvert();
    void playSelectedFile();
    void selectOutPath();
    void onMenuSelect(QTreeWidgetItem *item);
    void showFileListContextMenu(const QPoint &pos);
    void playAudioFile();
    void applyVideoPreset();
    void applyImagePreset();
    void startFontConvert();
    void startBatchConvert(const QString &action);

private:
    void initUI();
    void initLeftMenu();
    void updateVideoPreview();
    void applyVideoFilter();
    void contextMenuEvent(QContextMenuEvent *event) override;
    QString getQualityArg();

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
    QGraphicsVideoItem *videoItem;
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
    QComboBox       *cmbQuality;
    QComboBox       *cmbPreset;

    QGroupBox       *panelAudio;
    QComboBox       *cmbAudioCodec;
    QComboBox       *cmbSampleRate;
    QComboBox       *cmbBitrate;
    QComboBox       *cmbAudioChannel;
    QComboBox       *cmbAudioFilter;
    QComboBox       *cmbAudioPreset;
    QLineEdit       *editAudioStart;
    QLineEdit       *editAudioEnd;
    QMediaPlayer    *audioPlayer;
    QAudioOutput    *audioOutput;
    QSlider         *audioSlider;
    QPushButton     *btnAudioPlay;
    QLabel          *lblAudioTime;

    QGroupBox       *panelImage;
    QComboBox       *cmbImageFormat;
    QLineEdit       *editImgWidth;
    QLineEdit       *editImgHeight;
    QComboBox       *cmbImageQuality;
    QComboBox       *cmbImagePreset;
    QComboBox       *cmbRotate;
    QComboBox       *cmbFlip;
    QComboBox       *cmbImageEffect;

    QComboBox       *cmbImageWatermark;
    QLineEdit       *editImageWaterText;
    QComboBox       *cmbImageWaterSize;
    QComboBox       *cmbImageWaterColor;
    QComboBox       *cmbImageWaterPos;

    QComboBox       *cmbWatermark;
    QLineEdit       *editWaterText;
    QComboBox       *cmbWaterSize;
    QComboBox       *cmbWaterColor;
    QComboBox       *cmbWaterPos;

    QGroupBox       *panelFont;
    QLineEdit       *fontInputPath;
    QLineEdit       *fontOutputPath;
    QComboBox       *fontFormatCombo;
    QDoubleSpinBox  *spinFontSimplify;
    QSpinBox        *spinFontCompress;
    QSpinBox        *spinFontEm;
    QCheckBox       *chkFontMeta;
    QCheckBox       *chkFontFix;
    QCheckBox       *chkFontAscii;
    QComboBox       *cboFontPreset;

    QComboBox *cmbMergeMode;   // 👈 新增
    QComboBox *cmbFrameMode;

    QComboBox *cmbSubtitle; // 字幕设置

    double           m_fontSimplify;
    int              m_fontCompress;
    int              m_fontEmUnit;
    bool             m_fontKeepMeta;
    bool             m_fontFixOutline;
    bool             m_fontAsciiOnly;

    QGroupBox       *panelAbout;
    QMenu           *m_fileMenu;
    QAction         *m_selectAll;
    QAction         *m_clearSelect;

    QString          m_fmtArg;
    QString          m_outSuffix;
};

#endif
