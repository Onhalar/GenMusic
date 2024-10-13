#ifndef PLAYERMAIN_H
#define PLAYERMAIN_H

#include <QMainWindow>
#include <QtMultimedia/QtMultimedia>

QT_BEGIN_NAMESPACE
namespace Ui {
class PlayerMain;
}
QT_END_NAMESPACE

class PlayerMain : public QMainWindow
{
    Q_OBJECT

public:
    PlayerMain(QWidget *parent = nullptr);
    ~PlayerMain();

private slots:
    void on_btnLoadFiles_clicked();

    void on_btnPausePlay_clicked();

    void on_loadedSongList_doubleClicked(const QModelIndex &index);

    void onMediaStatusChanged(QMediaPlayer::MediaStatus status);

    void onPositionChanged(qint64 position);

    void on_btnRepeat_clicked();

    void on_btnSkip_clicked();

    void on_btnReturn_clicked();

    void on_btnSettings_clicked();

    void on_settingsVolumeSlider_valueChanged(int value);

    void on_settingsLoopPlaylsitCBox_checkStateChanged(const Qt::CheckState &arg1);

    void on_btnCleanText_clicked();

private:
    Ui::PlayerMain *ui;
    QMediaPlayer *player;
    QAudioOutput *audioOutput;

    void playSong(unsigned int);
    void makePlaylist(bool, bool, bool);
};
#endif // PLAYERMAIN_H
