#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSplitter>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTreeView>
#include <QListView>
#include <QTimer>
#include <QTime>
#include <QPushButton>
#include "skelmodel.h"
#include "scene.h"

class Viewport;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void open ();
    void rtdraw ();
    void speedchange (int value);
    void toggled ();
    void bodyyaw (int value);
    void bodypitch (int value);

private:
    QSplitter *_root;
    QSlider *_playback;
    QSlider *_speed;
    QPushButton *_toggle;
    Viewport *_vp;
    skelmodel *_skel_mdl;
    Scene *_scene;
    QTimer *_timer;
    QTime _oldtime;
    int _tick;
    bool _paused;

};
#endif // MAINWINDOW_H
