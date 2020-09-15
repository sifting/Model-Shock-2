#include <QtWidgets>
#include <QSplitter>
#include "mainwindow.h"
#include "viewport.h"
#include "mesh.h"
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
        auto hy = new QSlider ();
        hy->setOrientation(Qt::Orientation::Horizontal);
        hy->setMinimum (-90);
        hy->setMaximum ( 90);
        hy->setValue (0);

        auto hp = new QSlider ();
        hp->setOrientation(Qt::Orientation::Horizontal);
        hp->setMinimum (-90);
        hp->setMaximum ( 90);
        hp->setValue (0);

        auto head = new QLabel ();
        head->setText ("Head");

        auto by = new QSlider ();
        by->setOrientation(Qt::Orientation::Horizontal);
        by->setMinimum (-90);
        by->setMaximum ( 90);
        by->setValue (0);
        connect (by, &QSlider::valueChanged, this, &MainWindow::bodyyaw);

        auto bp = new QSlider ();
        bp->setOrientation(Qt::Orientation::Horizontal);
        bp->setMinimum (-90);
        bp->setMaximum ( 90);
        bp->setValue (0);
        connect (bp, &QSlider::valueChanged, this, &MainWindow::bodypitch);

        auto body = new QLabel ();
        body->setText ("Body");

        auto tmp = new QVBoxLayout ();
        auto group = new QGroupBox ("Controllers");
        tmp->addWidget (head);
        tmp->addWidget (hy);
        tmp->addWidget (hp);
        tmp->addWidget (body);
        tmp->addWidget (by);
        tmp->addWidget (bp);

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

    _root = new QSplitter;
    _root->addWidget (_vp);
    _root->addWidget (adaptor2);

    setCentralWidget(_root);
    setWindowTitle ("Model Shock 2");
    setMinimumSize (800, 600);
}

MainWindow::~MainWindow()
{
}

void MainWindow::open ()
{
    auto fn = QFileDialog::getOpenFileName(
                this,
                "Select a model or animation",
                "",
                "*.bin;*.mc");
    auto fi = QFileInfo (fn);
    auto ext = fi.suffix ().toLower ();
    auto prefix = QDir (fi.dir ().path ()
                        + "/"
                        + fi.completeBaseName ()).path ();
    if ("bin" == ext)
    {
        auto bin = (prefix + ".bin").toStdString ();
        auto mdl = Model::from_file (bin);
        g_scene->set_model (mdl);

        auto cal = (prefix + ".cal").toStdString ();
        auto skel = Skel::from_file (cal);
        g_scene->animator ()->set_skel (skel);
        g_scene->set_skel (skel);

        _skel_mdl->set_skel (skel);
    }
    else if ("mc" == ext)
    {
        auto tmp = fn.toStdString ();
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
void MainWindow::bodyyaw (int value)
{
    g_scene->animator ()->body_yaw = (float)value;
}
void MainWindow::bodypitch (int value)
{
    g_scene->animator ()->body_pitch = (float)value;
}
