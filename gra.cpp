 #include "gra.h"
#include<QApplication>
#include<QDesktopWidget>
#include<QBrush>
#include<windows.h>
#include"gracz.h"
#include"roslinka.h"
#include"mysz.h"
#include"kaczor.h"
#include"pierscien.h"
#include"troll.h"
#include"altanka.h"
#include"smok.h"
#include"skarb.h"
#include<QList>
#include"strona_ins.h"
#include "talia.h"
#include<QDebug>
Gra::Gra(QWidget *parent) :QGraphicsView(parent)
{
    QRect rec=QApplication::desktop()->screenGeometry();
    scene = new QGraphicsScene();
    scene->setSceneRect(0,0,rec.width(),rec.height());
    QImage image=QImage(":/images/bg.jpg");
    image=image.scaled(rec.width(),rec.height());
    setBackgroundBrush(QBrush(image));
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setFixedSize(rec.width(),rec.height());
    setScene(scene);
    gracz=new Gracz(this);
    poloczenie=new UdpSocket(this);
    connect(poloczenie,SIGNAL(karta(int)),this,SLOT(recive(int)));
    menu();
}


Karta *Gra::dobierz()
{
    if(!talia->empty())
    {
        Karta *k=talia->first();
        talia->removeFirst();
        return k;
    }
    return nullptr;
}

void Gra::keyPressEvent(QKeyEvent *event)
{
    if(event->key()==Qt::Key_Escape && state!="menu") menu();
}

QList<Przycisk *> Gra::getPrzyciskGracze()
{
    QList<Przycisk*> przyciski;
    //int j=0;
    //for(Karta *t:talia) continue;

    /*for(Gracz *g:*poloczeni)
    {
        Przycisk *przycisk=new Przycisk(g->getNazwa());
        przycisk->setX(width()/2-przycisk->boundingRect().width()/2);
        przycisk->setY(j++*height());
        przyciski.insert(przyciski.end(),przycisk);
    }*/
    return przyciski;
}


void Gra::start()
{
    if(state=="polacz") delete dodajPoloczenie;
    gracz=new Gracz(this);
    scene->clear();
    state="start";
    talia=new Talia(this);
    talia->tasuj();
    tura();

}

void Gra::stop()
{
    QApplication::quit();
}

void Gra::menu()
{
    if(state=="instrukcja") delete instru;
    else if(state=="start")
    {
        delete gracz;
        delete talia;
        nastole=nullptr;
        disconnect(poloczenie,SIGNAL(karta(int)),this,SLOT(recive(int)));
    }
    else if(state=="polacz") delete dodajPoloczenie;
    state="menu";
    scene->clear();
    Przycisk *przycisk=new Przycisk("Start");
    przycisk->setX(width()/2-przycisk->boundingRect().width()/2);
    connect(przycisk,SIGNAL(clicked(QString)),this,SLOT(start()));
    scene->addItem(przycisk);
    przycisk=new Przycisk("Połącz");
    przycisk->setX(width()/2-przycisk->boundingRect().width()/2);
    przycisk->setY(przycisk->boundingRect().height());
    connect(przycisk,SIGNAL(clicked(QString)),this,SLOT(polacz()));
    scene->addItem(przycisk);
    przycisk=new Przycisk("Instrukcja");
    przycisk->setX(width()/2-przycisk->boundingRect().width()/2);
    przycisk->setY(2*przycisk->boundingRect().height());
    connect(przycisk,SIGNAL(clicked(QString)),this,SLOT(instrukcja()));
    scene->addItem(przycisk);
    przycisk=new Przycisk("Wyjscie");
    przycisk->setX(width()/2-przycisk->boundingRect().width()/2);
    przycisk->setY(3*przycisk->boundingRect().height());
    connect(przycisk,SIGNAL(clicked(QString)),this,SLOT(stop()));
    scene->addItem(przycisk);
}

void Gra::tura()
{
    Karta *k=gracz->dobierz();
    if(k==nullptr)
    {
        gracz->tura=true;
        return;
    }
    send(k->getMoc()+10);
    k->setY(height()-k->boundingRect().height());
    scene->addItem(k);
    gracz->k1->setX(width()/2-gracz->k1->boundingRect().width());
    if(gracz->k2==nullptr) return;
    gracz->k2->setX(width()/2);
    gracz->tura=true;
}

void Gra::koniectury(Karta* k)
{
    //if(!gracz->tura) return;

    if(nastole!=nullptr)
    {
        scene->removeItem(nastole);
        delete nastole;
    }
    nastole=k;
    nastole->setX(width()/2-nastole->boundingRect().width()/2);
    nastole->setY(height()/2-nastole->boundingRect().height()/2);
    send(nastole->getMoc());
}

void Gra::kolejnatura(int m)
{
    Karta *k=talia->dobierzMoc(m);
    if(k!=nullptr)
    {
        if(nastole!=nullptr)
        {
            scene->removeItem(nastole);
            delete nastole;
        }
        nastole=k;
        nastole->setX(width()/2-nastole->boundingRect().width()/2);
        nastole->setY(height()/2-nastole->boundingRect().height()/2);
        scene->addItem(nastole);
    }
    tura();
}

void Gra::instrukcja()
{
    state="instrukcja";
    scene->clear();
     instru=new Instrukcja(scene);



}

void Gra::recive(int k)
{
    if(state!="start") start();
    if(k<10)
    {

        kolejnatura(k);
        return;
    }
    if(k<20)
    {
        talia->dobierzMoc(k%10);
        return;
    }
}

void Gra::send(int w)
{

    poloczenie->send(QString::number(w));
}

void Gra::polacz()
{
    state="polacz";
    scene->clear();
    dodajPoloczenie=new DodajPoloczenie(this);
    dodajPoloczenie->resize(width()/2,height()/2);
    dodajPoloczenie->move(width()/2-dodajPoloczenie->width()/2,height()/2-dodajPoloczenie->height()/2);
    dodajPoloczenie->setVisible(true);
    connect(dodajPoloczenie,SIGNAL(wyslij(QString)),poloczenie,SLOT(addConnection(QString)));
    connect(dodajPoloczenie,SIGNAL(wyslij(QString)),this,SLOT(start()));

}
