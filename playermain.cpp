#include "playermain.h"
#include "ui_playermain.h"
#include <QtMultimedia>
#include <QFileDialog>
#include <QDir>
#include <QFileInfo>
#include <QStyle>
#include <QPushButton>
#include <algorithm>
#include <QRegularExpression>

using namespace std;

PlayerMain::PlayerMain(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::PlayerMain)
{
    ui->setupUi(this);

    player = new QMediaPlayer;
    audioOutput = new QAudioOutput;

    audioOutput -> setVolume(0.85f);
    player -> setAudioOutput(audioOutput);

    connect(player, &QMediaPlayer::mediaStatusChanged, this, &PlayerMain::onMediaStatusChanged);
    connect(player, &QMediaPlayer::positionChanged, this, &PlayerMain::onPositionChanged);

    void playSong();
    void makePlaylist();

    ui -> settingsVolumeSlider -> setValue(85);
}

PlayerMain::~PlayerMain()
{
    delete ui;
}

string allowedExtensions[] = {"*.mp3", "*.vaw", "*.flac"};

map<string, string> songs;
vector<string> playlist;

unsigned int songIndex;

bool repeat = false;
bool loopPlaylist = true;

unsigned int getSongIndex(const string& songName) { // finds the index of a song from playlist by name
    auto iterator = std::find(playlist.begin(), playlist.end(), songName);
    return distance(playlist.begin(), iterator);
}

string shortenText(const string& text, const unsigned int& length = 30) { // shortens text to the desired length (if necesery)
    if (text.size() > length) {
        string outputText;
        for (unsigned int i = 0; i < length - 3; i++) {
            outputText += text[i];
        }
        outputText += "...";
        return outputText;
    }
    else {
        return text;
    }

}

void PlayerMain::makePlaylist(const bool removeHashtags = false, const bool removeBrackets = false, const bool removeSqareBrackets = false) { // creates a playlist and displays it onto the GUI
    static QRegularExpression hastag("#{1}[^#\\s]+");
    static QRegularExpression brackets("\\({1}.+\\)?");
    static QRegularExpression sqareBrackets("\\[{1}.+\\]?");

    player -> stop();
    ui -> btnPausePlay -> setIcon(QIcon::fromTheme(QIcon::ThemeIcon::MediaPlaybackStart));

    map<string, string> tempSongs;
    playlist.clear();
    ui -> loadedSongList -> clear();

    int emptySongIndex = 0;

    for (auto& song : songs) {
        QString index = QString::fromStdString(song.first);

        if (removeHashtags) {
            index = index.replace(hastag, "");
        }
        if (removeBrackets) {
            index = index.replace(brackets, "");
        }
        if (removeSqareBrackets) {
            index = index.replace(sqareBrackets, "");
        }

        index = index.trimmed();

        if (index == "") {
            index = "[emptyName" + QString::number(emptySongIndex) + "]";
            ++emptySongIndex;
        }

        tempSongs[index.toStdString()] = song.second;
        ui -> loadedSongList -> addItem(index);
        playlist.push_back(index.toStdString());
    }
    songs = tempSongs;
}

void PlayerMain::playSong(const unsigned int index = songIndex) {
    player -> setSource(QUrl::fromLocalFile(QString::fromStdString(songs[playlist[index]]))); // sets the playback source from a file path stored in songs
    ui -> songNameDisplay -> setText(QString::fromStdString(shortenText(playlist[index]))); // displays shortened (if necesery) name of the song to the songNameDisplay
    player -> play();
    ui -> btnPausePlay -> setIcon(QIcon::fromTheme(QIcon::ThemeIcon::MediaPlaybackPause));
}

void PlayerMain::onMediaStatusChanged(QMediaPlayer::MediaStatus status) {
    // is triggerd when the state of buffered media changes, usualy: NoMedia, MediaBuffered or EndOfMedia
    if (status == QMediaPlayer::EndOfMedia) {
        if (repeat) { // forces repeat if song repeat is enabled
            playSong();
            return;
        }

        if (songIndex == playlist.size() - 1) { // checks if the player has reached the end of the playlsit
            if (loopPlaylist) { // loopPlaylist is on
                songIndex = 0;
            }
            else { // loopPlaylist is off so it stops playback
                ui -> songNameDisplay -> setText("");
                player -> stop();
                return;
            }
        }
        else {
            ++songIndex;
        }
        playSong();
    }
}

void PlayerMain::onPositionChanged(qint64 position) {
    // updates the max position, current position and force updates the songProgressDisplay
    ui -> songProgressDisplay -> setMaximum(player -> duration());
    ui -> songProgressDisplay -> setValue(position);
    ui -> songProgressDisplay -> update();
}

void PlayerMain::on_btnLoadFiles_clicked()
{
    // loading filters
    QStringList filters;
    for (auto& extension : allowedExtensions) {
        filters << QString::fromStdString(extension);
    }

    player -> pause();
    ui -> btnPausePlay -> setIcon(QIcon::fromTheme(QIcon::ThemeIcon::MediaPlaybackStart));
    map<string, string> tempSongs;

    // reading files from dir and loading into a list
    QDir musicFolder(QFileDialog::getExistingDirectory());
    for (auto& file : musicFolder.entryList(filters, QDir::Files)) {
        tempSongs[QFileInfo(file).baseName().toStdString()] = (musicFolder.path() + "/" + file).toStdString(); // idex = songName; value = filePath
    }

    // checking if the dir was empty and loading songs into a playlist
    if (!tempSongs.empty()) {
        player -> stop();
        songs = tempSongs;
        ui -> songNameDisplay -> setText("");
        makePlaylist(); // makes playlist and displays into onto the GUI song selecter
    }
}


void PlayerMain::on_btnPausePlay_clicked()
{
    if (player -> mediaStatus() == QMediaPlayer::BufferedMedia) { // checks if a song is loaded into the player
        if (player -> isPlaying()) {
            player -> pause();
            ui -> btnPausePlay -> setIcon(QIcon::fromTheme(QIcon::ThemeIcon::MediaPlaybackStart));
        }
        else {
            player -> play();
            ui -> btnPausePlay -> setIcon(QIcon::fromTheme(QIcon::ThemeIcon::MediaPlaybackPause));
        }
    }
}

void PlayerMain::on_loadedSongList_doubleClicked(const QModelIndex &index)
{
    songIndex = getSongIndex(ui -> loadedSongList -> itemFromIndex(index) -> text().toStdString()); // sets up songIndex after clicking on a new song in a current playlist
    playSong();
}

void PlayerMain::on_btnRepeat_clicked()
{
    if (repeat) {
        repeat = false;
        ui -> btnRepeat -> setFlat(false);
    }
    else {
        repeat = true;
        ui -> btnRepeat -> setFlat(true);
    }
}


void PlayerMain::on_btnSkip_clicked()
{
    if (!repeat) {
        ++songIndex;
    }
    playSong();
}


void PlayerMain::on_btnReturn_clicked()
{
    // checks if (relative to the length of th song - min 3, max 5) seconds have passed. If yes, it moves the songIndex back by 1
    if (!repeat && player -> position() < min(5000.0, max(3000.0, player -> duration() * 0.025))) { // in miliseconds
        --songIndex;
    }
    playSong();
}

void PlayerMain::on_btnSettings_clicked()
{
    //switches the view between songList and settings and applies the coresponding in-built icon
    if (ui -> settingsSwitcher -> currentIndex() == 0) {
        ui -> settingsSwitcher -> setCurrentIndex(1);
        ui -> btnSettings -> setIcon(QIcon::fromTheme(QIcon::ThemeIcon::AudioCard));
    }
    else {
        ui -> settingsSwitcher -> setCurrentIndex(0);
        ui -> btnSettings -> setIcon(QIcon::fromTheme(QIcon::ThemeIcon::DocumentProperties));
    }
}


void PlayerMain::on_settingsVolumeSlider_valueChanged(int value)
{
    audioOutput -> setVolume(value / 100.0f);
}


void PlayerMain::on_settingsLoopPlaylsitCBox_checkStateChanged(const Qt::CheckState &arg1)
{
    // if is checked, loopPlaylist = true; else false
    loopPlaylist = arg1 == Qt::Checked ? true : false;
}


void PlayerMain::on_btnCleanText_clicked()
{
    bool removeHastags = ui -> cleanHastagsCBox -> isChecked();
    bool removeBrackets = ui -> cleanBracketsCBox -> isChecked();
    bool removeSqareBrackets = ui -> cleanSqareBracketsCBox -> isChecked();

    makePlaylist(removeHastags, removeBrackets, removeSqareBrackets);
}

