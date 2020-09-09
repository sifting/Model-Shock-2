#include <QtWidgets>
#include "mainwindow.h"
#include "viewport.h"
#include "skel.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , _timer (new QTimer (this))
    , _paused (false)
{
    /*GL attributes must be created and set before
    the widget is drawn, or else Bad Things™ happen*/
    QSurfaceFormat fmt;
    fmt.setDepthBufferSize(24);
    fmt.setStencilBufferSize (8);
    fmt.setVersion (4, 6);
    fmt.setProfile (QSurfaceFormat::CoreProfile);
    QSurfaceFormat::setDefaultFormat (fmt);

    /*Create skeleton model*/
    _skel_mdl = new skelmodel (nullptr);

    _timer->setInterval (1000/60);
    connect (_timer, &QTimer::timeout, this, &MainWindow::rtdraw);
    _timer->start ();

    /*File menu*/
    auto act_open = new QAction ("&Open");
    connect (act_open, &QAction::triggered, this, &MainWindow::open);
    auto act_save = new QAction ("&Save");
    auto act_saveas = new QAction ("&Save as...");
    auto act_close = new QAction ("&Close");
    auto act_quit = new QAction ("&Quit");
    auto filemenu = menuBar ()->addMenu ("&File");
    filemenu->addAction (act_open);
    filemenu->addSeparator ();
    filemenu->addAction (act_save);
    filemenu->addAction (act_saveas);
    filemenu->addSeparator ();
    filemenu->addAction (act_close);
    filemenu->addSeparator ();
    filemenu->addAction (act_quit);

    /*View menu*/
    auto act_skel = new QAction ("Show Skeleton");
    auto viewmenu = menuBar ()->addMenu ("&View");
    viewmenu->addAction (act_skel);

    /*Help menu*/
    auto act_about = new QAction ("&About");
    auto helpmenu = menuBar ()->addMenu ("&Help");
    helpmenu->addAction (act_about);

    auto side = new QVBoxLayout ();
    side->setMargin(4);
    side->setSpacing(0);
    {
        auto tree = new QTreeView ();
        tree->setModel (_skel_mdl);
        tree->setHeaderHidden (true);
        tree->setExpandsOnDoubleClick (true);
        tree->setAutoScroll (true);

        auto tmp = new QVBoxLayout ();
        auto group = new QGroupBox ("Skeleton");
        tmp->addWidget (tree);
        group->setLayout (tmp);
        side->addWidget(group);
    }
    {
        auto anims = new QComboBox ();
        auto al = new QLabel ();
        al->setText ("Animation Cycle");
        al->setBuddy ((QWidget *)anims);

        auto seek = _playback = new QSlider ();
        seek->setOrientation(Qt::Orientation::Horizontal);
        seek->setMinimum (0);
        seek->setMaximum (100);
        seek->setValue (0);

        auto sl = new QLabel ();
        sl->setText ("Playback Position");
        sl->setBuddy ((QWidget *)seek);

        auto speed = _speed = new QSlider ();
        speed->setMinimum (-50);
        speed->setMaximum (50);
        speed->setOrientation (Qt::Orientation::Horizontal);
        speed->setValue (0);
        connect (_speed, &QSlider::valueChanged, this, &MainWindow::speedchange);

        auto spl = new QLabel ();
        spl->setText ("Playback Speed");
        spl->setBuddy ((QWidget *)speed);

        _toggle = new QPushButton ();
        _toggle->setText ("Pause");
        connect (_toggle, &QPushButton::clicked, this, &MainWindow::toggled);

        auto tmp = new QVBoxLayout ();
        auto group = new QGroupBox ("Animation");
        tmp->addWidget (anims);
        tmp->addWidget (sl);
        tmp->addWidget (seek);
        tmp->addWidget (spl);
        tmp->addWidget (speed);
        tmp->addWidget (_toggle);

        group->setLayout (tmp);
        side->addWidget(group);
    }
    auto adaptor2 = new QWidget ();
    adaptor2->setLayout (side);

    _vp = new Viewport (parent);

    _root = new QHBoxLayout;
    _root->setMargin(0);
    _root->setSpacing(0);

    _root->addWidget (_vp);
    _root->setStretchFactor(_vp, 4);
    _root->addWidget (adaptor2);
    _root->setStretchFactor(adaptor2, 1);

    auto adaptor = new QWidget ();
    adaptor->setLayout (_root);

    setCentralWidget(adaptor);
    setWindowTitle ("Model Shock 2");
    setMinimumSize (800, 600);
}

MainWindow::~MainWindow()
{
}

void MainWindow::open ()
{
    auto fn = QFileDialog::getOpenFileName(this, "Select a model or animation", "", "*.cal;*.mc");
    auto fi = QFileInfo (fn);
    auto tmp = fn.toStdString ();
    auto ext = fi.suffix ().toLower ();
    if ("cal" == ext)
    {
        auto skel = Skel::from_file (tmp);
        _skel_mdl->set_skel (skel);
        g_scene->animator ()->set_skel (skel);
        g_scene->set_skel (skel);
    }
    else if ("mc" == ext)
    {
        auto anim = Anim::from_file (tmp);
        g_scene->animator ()->set_anim (anim);
        g_scene->add_anim (anim);
    }
}
void MainWindow::rtdraw ()
{   /*Compute frame tick*/
    auto curr = QTime::currentTime ();
    _tick = _oldtime.msecsTo (curr);
    _oldtime = curr;

    if (!_paused)
    {
        static int time = 0;
        time += g_scene->animator ()->speed ()*_tick;
        if (time >= 2000)
        {
            time -= 2000;
        }

        _playback->setMaximum(2000);
        _playback->setValue(time);
    }
    _vp->update ();
}
void MainWindow::speedchange (int value)
{
    auto speed = (100.0 + value)/100.0;
    g_scene->animator ()->set_speed (speed);
}
void MainWindow::toggled ()
{
    if (!_paused) _toggle->setText ("Resume");
    else _toggle->setText ("Pause");
    _paused = !_paused;
}
