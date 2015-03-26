#include "mainwindow.h"
#include <QApplication>
#include <QtGlobal>
#include <QDesktopServices>

MainWindow::MainWindow(QWidget *parent) :
  QMainWindow(parent)
{    
  this->setWindowTitle(tr("Reproductor chachi"));
  this->setWindowIcon(QIcon::fromTheme("applications-multimedia"));
  createMenu();
  createToolBar();
  createMainWidget();
  createDialogs();
  userConfig();
  createConnections();
}

MainWindow::~MainWindow()
{

}

void MainWindow::closeEvent(QCloseEvent *event)
{
  if (QMessageBox::Yes == msgQuit_->exec()) {
    writeJSON();
    event->accept();
  } else {
    event->ignore();
  }
}

void MainWindow::onOpen()
{
  msgOpen_->exec();
  QAction *action(NULL);
  if ((QPushButton*) msgOpen_->clickedButton() == btnSourceLocal_) {
    // Local file source
    QString *selectedFilter = new QString;
    QString path = QFileDialog::getOpenFileName(this,
                                                tr("Abrir archivo"),
                                                "..",
                                                tr("Música (*.ogg *.mp3);;\
                                                    Vídeo (*.avi *.mp4);;\
                                                    Todos(*)"),
                                                selectedFilter);
    if (path.isEmpty()) {
      return;
    }
    mediaPlayer_->setMedia(QUrl::fromLocalFile(path));
    // Split path and fileName
    QString fileName;
    if (path.contains('/', Qt::CaseInsensitive)) {
      int index = path.size()- 1 - path.lastIndexOf('/');
      fileName = path.right(index);
    }
    // Add to recent files, if it doesn't exist
    if ((fileName.endsWith(".ogg") ||
         fileName.endsWith(".mp3")) &&
        !lstActsRecentMusic_->contains(fileName)) {
      // Add to recent music files
      action = new QAction(fileName, mnuRecentMusic_);
      action->setObjectName("QAction");
      action->setData(QVariant(path));
      connect(action, SIGNAL(triggered()), this, SLOT(onRecent()));
      lstActsRecentMusic_->insert(fileName, action);
      mnuRecentMusic_->addAction(action);
      mnuRecentMusic_->setEnabled(true);
      return;
    }
    if ((fileName.endsWith(".avi") ||
         fileName.endsWith(".mp4")) &&
        !lstActsRecentVideo_->contains(fileName)) {
      // Add to recent video files
      action = new QAction(fileName, mnuRecentVideo_);
      action->setObjectName("QAction");
      action->setData(QVariant(path));
      connect(action, SIGNAL(triggered()), this, SLOT(onRecent()));
      lstActsRecentVideo_->insert(fileName, action);
      mnuRecentVideo_->addAction(action);
      mnuRecentVideo_->setEnabled(true);
      return;
    }
  }
  else {
    // Streaming source

    mediaPlayer_->setMedia(QUrl("http://208.92.53.87:80/MAXIMAFM"));
  }
}

void MainWindow::onRecent()
{
  QAction *sender = (QAction *) QObject::sender();
  mediaPlayer_->setMedia(QUrl::fromLocalFile(sender->data().toString()));
}

void MainWindow::onAbout()
{
  dlgAbout_->exec();
}

void MainWindow::onMetaData()
{
  if (dlgMetaData_) {
    dlgMetaData_->exec();
  }
}

void MainWindow::onFullScreen()
{
  bool status = videoWidget_->isFullScreen();
  videoWidget_->setFullScreen(!status);
}

void MainWindow::onSeek()
{
  mediaPlayer_->setPosition(playerSlider_->sliderPosition());
}

void MainWindow::onDurationChanged(qint64 duration)
{
  playerSlider_->setRange(0, duration);
}

void MainWindow::onPositionChanged(qint64 position)
{
  playerSlider_->setSliderPosition(position);
}

void MainWindow::onVolumeChanged(int volume)
{
  mediaPlayer_->setVolume(volume);
}

void MainWindow::onMetaDataAvailable(bool available)
{
  actMetaData_->setEnabled(available);
  if (available) {
    // Get metadata keys
    QStringList keysMetaData_ = mediaPlayer_->availableMetaData();
    // Create Dialog, Layout, Table, ...
    createMetaData();
    QGridLayout *lyt = new QGridLayout(dlgMetaData_);
    tblWgtMetada_ = new QTableWidget(dlgMetaData_);
        tblWgtMetada_->setColumnCount(2);
    tblWgtMetada_->setRowCount(keysMetaData_.size());
    QStringList headers;
    headers << tr("Información") << tr("Descripción");
    tblWgtMetada_->setHorizontalHeaderLabels(headers);
    tblWgtMetada_->horizontalHeader()->setStretchLastSection(true);
    // Fill Table
    int i(0);
    QTableWidgetItem *newItem;
    foreach (QString key, (keysMetaData_)) {
      QVariant value = mediaPlayer_->metaData(key);
      newItem = new QTableWidgetItem(key);
      tblWgtMetada_->setItem(i, 0, newItem);
      newItem = new QTableWidgetItem(value.toString());
      tblWgtMetada_->setItem(i, 1, newItem);
      i++;
    }
    tblWgtMetada_->resizeColumnsToContents();
    tblWgtMetada_->resizeRowsToContents();
    // Fill Layout
    lyt->addWidget(new QLabel(tr("Tabla informativa de metadatos"), dlgMetaData_), 0, 0, 1, 5);
    lyt->addWidget(tblWgtMetada_, 1, 0, 5, 5);
    dlgMetaData_->setLayout(lyt);
  }
}

void MainWindow::createMenu()
{
  // Create Menu
  mainMenu_ = new QMenuBar(this);
  // File
  mnuFile_ = new QMenu(tr("&Archivo"), this);
  mainMenu_->addMenu(mnuFile_);
  // File-Open
  actFileOpen_ = new QAction(QIcon::fromTheme("document-open"), tr("&Abrir"), mnuFile_);
  actFileOpen_->setShortcut(QKeySequence::Open);
  mnuFile_->addAction(actFileOpen_);
  // File-Recent
  mnuRecent_ = new QMenu(tr("&Reciente"), mnuFile_);
  mnuRecent_->setIcon(QIcon::fromTheme("document-open-recent"));
  mnuRecentMusic_ = new QMenu(tr("&Música"), mnuRecent_);
  mnuRecent_->addMenu(mnuRecentMusic_);
  mnuRecentVideo_ = new QMenu(tr("&Vídeo"), mnuRecent_);
  mnuRecent_->addMenu(mnuRecentVideo_);
  mnuRecentStreaming_ = new QMenu(tr("&Streaming"), mnuRecent_);
  mnuRecent_->addMenu(mnuRecentStreaming_);
  mnuFile_->addMenu(mnuRecent_);
  // File-Quit
  mnuFile_->addSeparator();
  actFileQuit_ = new QAction(QIcon::fromTheme("application-exit"), tr("&Salir"), mnuFile_);
  actFileQuit_->setShortcut(QKeySequence::Quit);
  mnuFile_->addAction(actFileQuit_);
  // View
  mnuView_ = new QMenu(tr("&Ver"), this);
  mainMenu_->addMenu(mnuView_);
  // View-FullScreen
  actFullScreen_ = new QAction(QIcon::fromTheme("view-fullscreen"), tr("&Pantalla Completa"), mnuView_);
  actFullScreen_->setShortcut(QKeySequence::FullScreen);
  mnuView_->addAction(actFullScreen_);
  // View-MetaData
  actMetaData_ = new QAction(QIcon::fromTheme("document-properties"), tr("&Meta Datos"), mnuView_);
  actMetaData_->setShortcut(QKeySequence::Preferences);
  mnuView_->addAction(actMetaData_);
  // Help
  mnuHelp_ = new QMenu(tr("A&yuda"), this);
  mainMenu_->addMenu(mnuHelp_);
  // Help-AboutThis
  actHelpAbout_ = new QAction(QIcon::fromTheme("help-contents"), tr("&Acerca de"), mnuHelp_);
  actHelpAbout_->setShortcut(QKeySequence::WhatsThis);
  mnuHelp_->addAction(actHelpAbout_);
  // Help-AboutQt
  mnuHelp_->addAction(tr("Acerca de &Qt"), qApp, SLOT(aboutQt()));
  // Set Menu
  this->setMenuBar(mainMenu_);
}

void MainWindow::createToolBar()
{
  // ToolBar
  mainToolBar_ = new QToolBar(this);
  mainToolBar_->addAction(actFileOpen_);
  mainToolBar_->addSeparator();
  mainToolBar_->addAction(actFullScreen_);
  this->addToolBar(mainToolBar_);
}

void MainWindow::createMainWidget()
{
  //Create central widget and set main layout
  wgtMain_ = new QWidget(this);
  lytMain_ = new QGridLayout(wgtMain_);
  wgtMain_->setLayout(lytMain_);
  this->setCentralWidget(wgtMain_);
  //Initialize widgets
  mediaPlayer_  = new QMediaPlayer(this);
  playerSlider_ = new QSlider(Qt::Horizontal, this);
  videoWidget_  = new MyQVideoWidget(mediaPlayer_);
  volumeSlider_ = new QSlider(Qt::Horizontal, this);
  btnOpen_      = new QToolButton(this);
  btnPlay_      = new QToolButton(this);
  btnPause_     = new QToolButton(this);
  btnStop_      = new QToolButton(this);
  //Setup widwgets
  videoWidget_->setMinimumSize(400, 400);
  mediaPlayer_->setVideoOutput(videoWidget_);
  mediaPlayer_->setVolume(100);
  videoWidget_->setAspectRatioMode(Qt::KeepAspectRatio);
  volumeSlider_->setRange(0, 100);
  volumeSlider_->setSliderPosition(100);
  //Populate grid layout
  lytMain_->addWidget(videoWidget_,  0, 0, 1, 5);
  lytMain_->addWidget(playerSlider_, 1, 0, 1, 5);
  lytMain_->addWidget(btnOpen_,      2, 0, 1, 1);
  lytMain_->addWidget(btnPlay_,      2, 1, 1, 1);
  lytMain_->addWidget(btnPause_,     2, 2, 1, 1);
  lytMain_->addWidget(btnStop_,      2, 3, 1, 1);
  lytMain_->addWidget(volumeSlider_, 2, 4, 1, 1);
  //Buttons icons
  btnOpen_->setIcon(QIcon::fromTheme("media-eject"));
  btnPause_->setIcon(QIcon::fromTheme("media-playback-pause"));
  btnPlay_->setIcon(QIcon::fromTheme("media-playback-start"));
  btnStop_->setIcon(QIcon::fromTheme("media-playback-stop"));
}

void MainWindow::createConnections()
{
  // Actions connections
  connect(actFileOpen_,     SIGNAL(triggered()),              this,         SLOT(onOpen()));
  connect(actFullScreen_,   SIGNAL(triggered()),              this,         SLOT(onFullScreen()));
  connect(actMetaData_,     SIGNAL(triggered()),              this,         SLOT(onMetaData()));
  connect(actFileQuit_,     SIGNAL(triggered()),              this,         SLOT(close()));
  connect(actHelpAbout_,    SIGNAL(triggered()),              this,         SLOT(onAbout()));
  // Widget connections
  connect(btnOpen_,         SIGNAL(pressed()),                this,         SLOT(onOpen()));
  connect(btnPlay_,         SIGNAL(pressed()),                mediaPlayer_, SLOT(play()));
  connect(btnPause_,        SIGNAL(pressed()),                mediaPlayer_, SLOT(pause()));
  connect(btnStop_,         SIGNAL(pressed()),                mediaPlayer_, SLOT(stop()));
  connect(playerSlider_,    SIGNAL(sliderReleased()),         this,         SLOT(onSeek()));
  connect(mediaPlayer_,     SIGNAL(durationChanged(qint64)),  this,         SLOT(onDurationChanged(qint64)));
  connect(mediaPlayer_,     SIGNAL(metaDataAvailableChanged(bool)), this,   SLOT(onMetaDataAvailable(bool)));
  connect(mediaPlayer_,     SIGNAL(positionChanged(qint64)),  this,         SLOT(onPositionChanged(qint64)));
  connect(volumeSlider_,    SIGNAL(sliderMoved(int)),         this,         SLOT(onVolumeChanged(int)));
}

void MainWindow::createDialogs()
{
  // Open
  msgOpen_ = new QMessageBox(this);
  msgOpen_->setWindowTitle(tr("Seleccionar origen"));
  msgOpen_->setWindowIcon(QIcon::fromTheme("dialog-question"));
  msgOpen_->setText(tr("Has seleccionado abrir una nueva reproducción"));
  msgOpen_->setInformativeText(tr("Selecciona su origen."));
  btnSourceLocal_ = msgOpen_->addButton(tr("Local"), QMessageBox::ActionRole);
  btnSourceStreaming_ = msgOpen_->addButton(tr("Streaming"), QMessageBox::ActionRole);
  // Metadata
  dlgMetaData_ = NULL;
  actMetaData_->setDisabled(true);
  // Quit
  msgQuit_ = new QMessageBox(this);
  msgQuit_->setWindowTitle(tr("Salir"));
  msgQuit_->setWindowIcon(QIcon::fromTheme("application-exit"));
  msgQuit_->setText(tr("Vas a salir del Reprocutor chaci."));
  msgQuit_->setInformativeText(tr("¿Realmente quieres salir?"));
  msgQuit_->setStandardButtons(QMessageBox::Yes | QMessageBox::No);
  msgQuit_->setDefaultButton(QMessageBox::No);
  // About
  dlgAbout_ = new QDialog(this);
  dlgAbout_->setMinimumSize(200, 200);
  dlgAbout_->setWindowTitle(tr("Acerca de Chachi reproductor"));
  dlgAbout_->setWindowIcon(QIcon::fromTheme("help-about"));
  lytAbout_ = new QGridLayout(dlgAbout_);
  dlgAbout_->setLayout(lytAbout_);
  lblAboutIcon_ = new QLabel(dlgAbout_);
  lblAboutIcon_->setPixmap(QIcon::fromTheme("applications-multimedia").pixmap(100));
  lytAbout_->addWidget(lblAboutIcon_,  1, 2, 1, 1);
  lblAboutText_ = new QLabel(dlgAbout_);
  lblAboutText_->setText(tr("El reproductor más chachi de todos los tiempos"));
  lytAbout_->addWidget(lblAboutText_,  2, 1, 1, 3);
}

void MainWindow::userConfig()
{
  // Create folder container
  pathUser_ = new QString(".chachi-media-player");
  dirUser_ = new QDir(*pathUser_);
  if (!dirUser_->exists()) {
    dirUser_->mkpath(".");
  }
  // Create file
  fileUser_ = new QFile(dirUser_->filePath("user.cfg"));
  bool exist = fileUser_->exists();
  if (!fileUser_->open(QIODevice::ReadWrite)) {
    qWarning("Couldn't open file.");
    return;
  }
  mnuRecentMusic_->setEnabled(false);
  mnuRecentVideo_->setEnabled(false);
  mnuRecentStreaming_->setEnabled(false);
  if (!exist) {
    createJSON();
  }
  else {
    readJSON();
  }
  fileUser_->close();
}

void MainWindow::createMetaData()
{
  destroyMetaData();
  dlgMetaData_ = new QDialog(this);
  dlgMetaData_->setMinimumSize(200, 200);
  dlgMetaData_->setWindowTitle(tr("Metadatos"));
  dlgMetaData_->setWindowIcon(QIcon::fromTheme("document-properties"));
}

void MainWindow::destroyMetaData()
{
  if (dlgMetaData_) {
    dlgMetaData_->deleteLater();
    delete dlgMetaData_;
    dlgMetaData_ = NULL;
  }
}

void MainWindow::createJSON()
{
  lstActsRecentMusic_ = new QMap<QString, QAction*>;
  lstActsRecentVideo_ = new QMap<QString, QAction*>;
  lstActsRecentStreaming_ = new QMap<QString, QAction*>;
}

bool MainWindow::readJSON()
{
  if (!fileUser_) {
   qDebug() << "¿Por qué no está inicializado fileUser?";
   return false;
  }
  if (!fileUser_->isOpen()) {
   qDebug() << "¿Por qué no está abierto fileUser?";
   return false;
  }
  QByteArray saveData = fileUser_->readAll();
  QJsonDocument jsonDocument(QJsonDocument::fromJson(saveData));
  QJsonObject jsonRootObject = jsonDocument.object();
  jsonRootObject = jsonRootObject["recent"].toObject();
  QJsonObject jsonRecentMusic     = jsonRootObject["music"].toObject();
  QJsonObject jsonRecentVideo     = jsonRootObject["video"].toObject();
  QJsonObject jsonRecentStreaming = jsonRootObject["streaming"].toObject();
  // Music
  lstActsRecentMusic_ = new QMap<QString, QAction*>;
  foreach (QString key, jsonRecentMusic.keys()) {
    mnuRecentMusic_->setEnabled(true);
    QAction *action = mnuRecentMusic_->addAction(key, this, SLOT(onRecent()));
    action->setObjectName("QAction");
    action->setData(QVariant(jsonRecentMusic.take(key)));
    lstActsRecentMusic_->insert(key, action);
  }
  // Video
  lstActsRecentVideo_ = new QMap<QString, QAction*>;
  foreach (QString key, jsonRecentVideo.keys()) {
    mnuRecentVideo_->setEnabled(true);
    QAction *action = mnuRecentVideo_->addAction(key, this, SLOT(onRecent()));
    action->setObjectName("QAction");
    action->setData(QVariant(jsonRecentVideo.take(key)));
    lstActsRecentVideo_->insert(key, action);
  }
  // Streaming
  lstActsRecentStreaming_ = new QMap<QString, QAction*>;
  foreach (QString key, jsonRecentStreaming.keys()) {
    mnuRecentStreaming_->setEnabled(true);
    QAction *action = mnuRecentStreaming_->addAction(key, this, SLOT(onRecent()));
    action->setObjectName("QAction");
    action->setData(QVariant(jsonRecentStreaming.take(key)));
    lstActsRecentStreaming_->insert(key, action);
  }
  return true;
}

bool MainWindow::writeJSON()
{
  if (!fileUser_) {
   qDebug() << "¿Por qué no está inicializado fileUser?";
   return false;
  }
  if (!fileUser_->open(QIODevice::WriteOnly | QIODevice::Truncate)) {
    qWarning("Couldn't open file.");
    return false;
  }
  QAction *action(NULL);
  // Make music JsonObject
  QJsonObject *jsonRecentMusic = new QJsonObject;
  foreach (QString key, lstActsRecentMusic_->keys()) {
    action = (*lstActsRecentMusic_)[key];
    lstActsRecentMusic_->remove(key);
    jsonRecentMusic->insert(key, QJsonValue(action->data().toString()));
    delete action;
  }
  // Make video JsonObject
  QJsonObject *jsonRecentVideo = new QJsonObject;
  foreach (QString key, lstActsRecentVideo_->keys()) {
    action = (*lstActsRecentVideo_)[key];
    lstActsRecentVideo_->remove(key);
    jsonRecentVideo->insert(key, QJsonValue(action->data().toString()));
    delete action;
  }
  // Make streaming  JsonObject
  QJsonObject *jsonRecentStreaming= new QJsonObject;
  foreach (QString key, lstActsRecentStreaming_->keys()) {
    action = (*lstActsRecentStreaming_)[key];
    lstActsRecentStreaming_->remove(key);
    jsonRecentStreaming->insert(key, QJsonValue(action->data().toString()));
    delete action;
  }
  action = NULL;
  // Write JsonObjects
  QJsonObject *empty = new QJsonObject;
  empty->insert("music", *jsonRecentMusic);
  empty->insert("video", *jsonRecentVideo);
  empty->insert("streaming", *jsonRecentStreaming);
  QJsonObject *base = new QJsonObject;
  base->insert("recent", *empty);
  delete empty;
  QJsonDocument *jsonDocument = new QJsonDocument(*base);
  delete base;
  fileUser_->write(jsonDocument->toJson());
  delete jsonDocument;
  fileUser_->close();
  return true;
}
