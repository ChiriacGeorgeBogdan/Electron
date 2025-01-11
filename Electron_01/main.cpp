#include <graphics.h>
#include <winbgim.h>
#include <iostream>
#include <cstdio>
#include <fstream>
#include <vector>
#include <cstring>
#include <conio.h>
#include <thread>
#include <chrono>
#include <cstdlib>
#define FUNDAL 0
#include <windows.h>
#include <string>

using namespace std;
void redraw();
/// Constants
const int LATIME_ECRAN = 1800;
const int INALTIME_ECRAN = 950;

///Item barul este aflat langa marginea de sus a ecranului y=0
const int INALTIMEA_BAREI_DE_ITEME = 50;   /// inaltimea Item BAR
///Barul de tooluri este situat langa marginea stanga a ecranului (x=0) si sub barul de iteme
const int LATIME_TOOLBAR = 160;
const int NR_TOOLS=10;
const int NR_ITEME=13;
const int REFRESH_RATE=1000.0/30;

const int MAX_PIESE = 100;  /// Nr maxim de piese pe care le putem desena
const int MAX_INTRARI = 3;

const char Tool_Labels[NR_TOOLS][20]= {"Main Menu", "Make Connection", "Rotate","Move","Resize","Erase Shape", "Erase All", "Undo", "Redo","Save as"};
const char Item_Labels[NR_ITEME][15]= {"Shape 1"};

bool buffer=0;

//fisierele si structura aferenta figurilor
const char fisiere[20][15]= {"AMPLOP.PS","BATERIE.PS","CONDENS.PS","DIODA.PS","NOD.PS","POLARIZ.PS","REZIST.PS","SERVOMOT.PS","SINU.PS","STOP.PS","TRANZNPN.PS","TRANZPNP.PS","ZENNER.PS"};
struct figura
{
    char nume[50];
    int nr_intrari;
    double intrari[5][2];
    char descriere[100];
    int nr_bucati;
    double bucati[20][4];
    char tip_bucata[20];
    int marire;
} figuri[50];

const int CULOARE_LEGATURI=YELLOW;
int CULOARE_FIGURA=YELLOW;

void citire_figura(int index)
{
    ifstream fin(fisiere[index]);
    fin.getline(figuri[index].nume,50);
    fin>>figuri[index].nr_intrari;
    for (int i=0; i<figuri[index].nr_intrari; ++i)
    {
        fin>>figuri[index].intrari[i][0]>>figuri[index].intrari[i][1];
    }
    fin.get();
    fin.getline(figuri[index].descriere,100);
    fin>>figuri[index].nr_bucati;
    for (int i=0; i<figuri[index].nr_bucati; ++i)
    {
        fin>>figuri[index].tip_bucata[i];
        for(int j=0; j<4; ++j)
            fin>>figuri[index].bucati[i][j];
    }
}
void citire_figuri()
{
    for (int i=0; i<NR_ITEME; ++i)
    {
        citire_figura(i);
        figuri[i].marire=20;
    }
    figuri[0].marire=7;
    figuri[7].marire=figuri[8].marire=10;
    figuri[10].marire=figuri[11].marire=16;
    figuri[4].marire=40;
}




/// Memoreaza toate piesele aflate pe ecran

struct intrare
{
    int x;
    int y;
};
struct piesa
{
    int x1,y1,x2,y2;//colt stanga sus si colt dreapta jos
    int x,y,index;
    intrare intrari[MAX_INTRARI];
    int orientare; /// 0 <=> 0 grade; 1 <=> 90 grade; 2 <=> 180 grade; 3 <=> 270 grade
    double zoom;
    char nume [30];
    char valoare[30];
    int unit;
} piese[MAX_PIESE];

struct grafuri{
    int intrari[MAX_INTRARI][MAX_INTRARI];
}graf[MAX_PIESE][MAX_PIESE];

//Stiva modificarilor
pair<vector<int>, vector<piesa> >modificari[100];
vector<vector<int> >legaturi_modificate[100];
vector<vector<char> >caracteristici_modificate[100];
int p=-1,q=-1;


int nrPiese = -1; /// Nr de piese aflate pe ecran

void roteste (float x, float y, float & x_nou, float & y_nou)
{
    x_nou = y;
    y_nou = -x;
}
void myRectangle(int orientare, int index, int i, int &a, int &b, int &c, int &d, double zoom)
{
    float x1 = figuri[index].bucati[i][0];
    float y1 = figuri[index].bucati[i][1];
    float x2 = figuri[index].bucati[i][2];
    float y2 = figuri[index].bucati[i][3];

    float x1_nou = x1, y1_nou = y1, x2_nou = x2, y2_nou = y2;

    for (int j = 0; j < orientare; ++j)
    {
        roteste(x1_nou, y1_nou, x1_nou, y1_nou);
        roteste(x2_nou, y2_nou, x2_nou, y2_nou);
    }

    a = figuri[index].marire * x1_nou * zoom;
    b = figuri[index].marire * y1_nou * zoom;
    c = figuri[index].marire * x2_nou * zoom;
    d = figuri[index].marire * y2_nou * zoom;
    if(a>c) swap(a,c);
    if(b>d) swap(b,d);
}
void myLine(int orientare, int index, int i, int &a, int &b, int &c, int &d, double zoom)
{
    float x1 = figuri[index].bucati[i][0];
    float y1 = figuri[index].bucati[i][1];
    float x2 = figuri[index].bucati[i][2];
    float y2 = figuri[index].bucati[i][3];

    float x1_nou = x1, y1_nou = y1, x2_nou = x2, y2_nou = y2;

    for (int j = 0; j < orientare; ++j)
    {
        roteste(x1_nou, y1_nou, x1_nou, y1_nou);
        roteste(x2_nou, y2_nou, x2_nou, y2_nou);
    }

    a = figuri[index].marire * x1_nou * zoom;
    b = figuri[index].marire * y1_nou * zoom;
    c = figuri[index].marire * x2_nou * zoom;
    d = figuri[index].marire * y2_nou * zoom;
}


void myEllipse(int orientare, int index, int i, int &a, int &b, int &c, int &d, double zoom)
{
    float x1 = figuri[index].bucati[i][0];
    float y1 = figuri[index].bucati[i][1];
    float rx = figuri[index].bucati[i][2];
    float ry = figuri[index].bucati[i][3];

    float x1_nou = x1, y1_nou = y1;

    for (int j = 0; j < orientare; ++j)
    {
        roteste(x1_nou, y1_nou, x1_nou, y1_nou);
        swap(rx, ry);
    }

    a = figuri[index].marire * x1_nou * zoom;
    b = figuri[index].marire * y1_nou * zoom;
    c = figuri[index].marire * rx * zoom;
    d = figuri[index].marire * ry * zoom;
}

void myArc(int orientare, int index, int i, int &a, int &b, int &c, int &d, double zoom)
{
    float x1 = figuri[index].bucati[i][0];
    float y1 = figuri[index].bucati[i][1];
    float rx = figuri[index].bucati[i][2];
    float ry = figuri[index].bucati[i][3];

    float x1_nou = x1, y1_nou = y1;

    for (int j = 0; j < orientare; ++j)
    {
        roteste(x1_nou, y1_nou, x1_nou, y1_nou);
        swap(rx, ry);
    }

    a = figuri[index].marire * x1_nou * zoom;
    b = figuri[index].marire * y1_nou * zoom;
    c = figuri[index].marire * rx * zoom;
    d = figuri[index].marire * ry * zoom;
}
/// Deseneaza Bara de Iteme/Piese
void DeseneazaBaraDeIteme()
{
    int Lungimea_Barei_Iteme = LATIME_ECRAN / NR_ITEME;

    for (int i = 0; i < NR_ITEME; ++i)
    {

        setfillstyle(SOLID_FILL, LIGHTGRAY);
        bar(i * Lungimea_Barei_Iteme, 0, (i + 1) * Lungimea_Barei_Iteme, INALTIMEA_BAREI_DE_ITEME);

        setcolor(BLACK);
        rectangle(i * Lungimea_Barei_Iteme, 0, (i + 1) * Lungimea_Barei_Iteme, INALTIMEA_BAREI_DE_ITEME);

        /// Desenam figurile in meniul de iteme

        setcolor(COLOR(255, 255, 51));
        ///Centram coordonatele
        int x = i * Lungimea_Barei_Iteme + Lungimea_Barei_Iteme / 2;
        int y = INALTIMEA_BAREI_DE_ITEME / 2;
        int index = i;


        for (int j = 0; j < figuri[index].nr_bucati; ++j)
        {
            char type = figuri[index].tip_bucata[j];
            int a = figuri[index].marire * figuri[index].bucati[j][0];
            int b = figuri[index].marire * figuri[index].bucati[j][1];
            int c = figuri[index].marire * figuri[index].bucati[j][2];
            int d = figuri[index].marire * figuri[index].bucati[j][3];

            if (type == 'L')
            {
                line(x + a, y + b, x + c, y + d);
            }
            else if (type == 'O')
            {
                ellipse(x + a, y + b, 0, 360, c, d);
            }
            else if (type == 'R')
            {
                rectangle(x + a, y + b, x + c, y + d);
            }
            else if (type == 'A')
            {
                arc(x + a, y + b, 270, 90, c+d+0.5);
            }
        }
    }
}

/// Deseneaza Bara de Tooluri
void DeseneazaBaraDeTools()
{
    int TOOLS_Inaltime = (INALTIME_ECRAN - INALTIMEA_BAREI_DE_ITEME) / NR_TOOLS;
    for (int i = 0; i < NR_TOOLS; ++i)
    {
        setbkcolor(BLACK);
        setfillstyle(SOLID_FILL, DARKGRAY);
        bar(0, 1 + INALTIMEA_BAREI_DE_ITEME + i * TOOLS_Inaltime, LATIME_TOOLBAR, 1 + INALTIMEA_BAREI_DE_ITEME + (i + 1) * TOOLS_Inaltime);

        setcolor(WHITE);
        rectangle(0, 1 + INALTIMEA_BAREI_DE_ITEME + i * TOOLS_Inaltime, LATIME_TOOLBAR, 1 + INALTIMEA_BAREI_DE_ITEME + (i + 1) * TOOLS_Inaltime);

        setbkcolor(DARKGRAY);
        setcolor(BLACK);
        settextstyle(SANS_SERIF_FONT, HORIZ_DIR, 2);
        char label[20];
        strcpy(label, Tool_Labels[i]);

        int textWidth = textwidth(label);
        int textHeight = textheight(label);
        int textX = (LATIME_TOOLBAR - textWidth) / 2;
        int textY = 1 + INALTIMEA_BAREI_DE_ITEME + i * TOOLS_Inaltime + (TOOLS_Inaltime - textHeight) / 2;

        outtextxy(textX, textY, label);
    }
}

bool estePeTabla (int x, int y)
{
    return y>INALTIMEA_BAREI_DE_ITEME && y<=INALTIME_ECRAN && x>LATIME_TOOLBAR && x<= LATIME_ECRAN;
}
bool seIntersecteaza (piesa a, piesa b)
{
    return a.x2>=b.x1 && b.x2>=a.x1 && a.y2>=b.y1 && b.y2>=a.y1;
}
bool sePoateDesena(piesa piesaNoua, int x, int y, int index)
{
    for (int i=0; i<=nrPiese; ++i)
    {
       ///verificam daca se intersecteaza cu alte piese
        if (seIntersecteaza(piesaNoua,piese[i]))
        {
            cout<<"Se suprapune cu piesa ("<<i<<")\n";
            return false;
        }
    }
    if (!estePeTabla(piesaNoua.x1,piesaNoua.y1) || !estePeTabla(piesaNoua.x2,piesaNoua.y2)) return false;
    return true;
}
void golire_ecran ()
{
    setbkcolor(BLACK);
    setfillstyle(SOLID_FILL,BLACK);
    bar(LATIME_TOOLBAR+1,INALTIMEA_BAREI_DE_ITEME+1,LATIME_ECRAN,INALTIME_ECRAN);
}

const int MAX_PASI_RETINUTI = 5;


char nume[100];
int unit=-1;
int selectionColor=COLOR(255,255,153);
int unitColor=WHITE;
int campActiv=-1;

void drawOmega(int x, int y, int radius, int dir)
{
    setcolor(unitColor);
    arc(x, y, dir*90, (240+dir*90)%360 , radius);
    arc(x, y, (300 + dir*90)%360, (360+dir*90)%360, radius);
    if (dir==0){
        line(x-radius,y+radius,x-radius/2+1,y+radius);
        line(x+radius,y+radius,x+radius/2-1,y+radius);
    }
    else if (dir==1){
        line(x+radius,y+radius,x+radius,y+radius/2);
        line(x+radius,y-radius,x+radius,y-radius/2);
    }
    else if (dir==2){
        line(x-radius,y-radius,x-radius/2+1,y-radius);
        line(x+radius,y-radius,x+radius/2-1,y-radius);
    }
    else if (dir==3){
        line(x-radius,y+radius,x-radius,y+radius/2);
        line(x-radius,y-radius,x-radius,y-radius/2);
    }
}
void desenare_caracteristici (piesa P)
{
    char *nume=P.nume;
    int unit=P.unit;
    char val[30];
    strcpy(val,P.valoare);
    int xc=(P.x1+P.x2)/2;
    int yc=(P.y1+P.y2)/2;
    int gap=10;

    setcolor(WHITE);
    setbkcolor(BLACK);
    settextstyle(1,P.orientare,1);
    if (P.orientare==0)
    {
        outtextxy(xc-textwidth(nume)/2,P.y1-textheight(nume)-10,nume);
        outtextxy(xc-textwidth(val)/2,P.y2+10,val);
        if (unit==0)
            drawOmega(xc+textwidth(val)/2+10,P.y2+5+gap,7,0);
        else if (unit==1)
            outtextxy(xc+textwidth(val)/2,P.y2+gap,"A");
        else if (unit==2)
            outtextxy(xc+textwidth(val)/2,P.y2+gap,"V");
    }
    else if (P.orientare==1)
    {
        outtextxy(P.x1-textheight(nume)-gap,yc+textwidth(nume)/2,nume);
        outtextxy(P.x2+gap,yc+textwidth(val)/2,val);
        if (unit==0)
            drawOmega(P.x2+gap+10,yc-textwidth(val)/2-15,7,1);
        else if (unit==1)
            outtextxy(P.x2+gap,yc-textwidth(val)/2,"A");
        else if (unit==2)
            outtextxy(P.x2+gap,yc-textwidth(val)/2,"V");
    }
    else if (P.orientare==2)
    {
        outtextxy(xc+textwidth(nume)/2,P.y2+gap+textheight(nume),nume);
        outtextxy(xc+textwidth(val)/2,P.y1-gap,val);
        if (unit==0)
            drawOmega(xc-textwidth(val)/2-12,P.y1-gap-10,7,2);
        else if (unit==1)
            outtextxy(xc-textwidth(val)/2,P.y1-gap,"A");
        else if (unit==2)
            outtextxy(xc-textwidth(val)/2,P.y1-gap,"V");
    }
    else if (P.orientare==3)
    {
        outtextxy(P.x2+textheight(nume)+gap,yc-textwidth(nume)/2,nume);
        outtextxy(P.x1-gap,yc-textwidth(val)/2,val);
        if (unit==0)
            drawOmega(P.x1-gap-10,yc+textwidth(val)/2+12,7,3);
        else if (unit==1)
            outtextxy(P.x1-gap,yc+textwidth(val)/2,"A");
        else if (unit==2)
            outtextxy(P.x1-gap,yc+textwidth(val)/2,"V");
    }
}


struct chenare{
    int x1,y1,x2,y2;
}chenarModal,chenarNume,chenarValoare,chenarOhm,chenarAmp,chenarVolt,chenarButon,chenarX;

int LATIME_MODAL=900;
int INALTIME_MODAL=500;
int LATIME_BUTON_SALVARE=250;
int INALTIME_BUTON_SALVARE=50;
int BKMODAL=COLOR(20,20,20);

bool inchenar(int x, int y, chenare C)
{
    return (x>=C.x1 && x<=C.x2 && y>=C.y1 && y<=C.y2);
}

void generare_coordonate_chenare()
{

    int x=LATIME_ECRAN/2;
    int y=INALTIME_ECRAN/2;
    int L=LATIME_MODAL;
    int H=INALTIME_MODAL;
    int padding=30;
    char *numeStr="Nume:";
    char *valStr="Valoare:";
    char *unitStr="Unitate masura:";

    settextstyle(DEFAULT_FONT,0,3);
    chenarModal.x1=x-L/2; chenarModal.y1=y-H/2; chenarModal.x2=x+L/2; chenarModal.y2=y+H/2;
    chenarOhm.x1=x-L/2+padding+textwidth(unitStr);
    chenarOhm.y1=y-H/2+padding+250-5;
    chenarOhm.x2=x-L/2+padding+textwidth(unitStr)+25+5;
    chenarOhm.y2=y-H/2+padding+250+25;

    chenarAmp.x1=x-L/2+padding+textwidth(unitStr)+35;
    chenarAmp.y1=y-H/2+padding+250-5;
    chenarAmp.x2=x-L/2+padding+textwidth(unitStr)+25+5+45;
    chenarAmp.y2=y-H/2+padding+250+25;

    chenarVolt.x1=x-L/2+padding+textwidth(unitStr)+80;
    chenarVolt.y1=y-H/2+padding+250-5;
    chenarVolt.x2=x-L/2+padding+textwidth(unitStr)+25+5+85;
    chenarVolt.y2=y-H/2+padding+250+25;


    chenarButon.x1=x-LATIME_BUTON_SALVARE/2;
    chenarButon.y1=y+H/4;
    chenarButon.x2=x+LATIME_BUTON_SALVARE/2;
    chenarButon.y2=y+H/4+INALTIME_BUTON_SALVARE;


    chenarX.x1=(LATIME_ECRAN-LATIME_MODAL)/2+LATIME_MODAL-30;
    chenarX.y1=(INALTIME_ECRAN-INALTIME_MODAL)/2+10;
    chenarX.x2=chenarX.x1+textwidth("x");
    chenarX.y2=chenarX.y1+textheight("x");

    chenarNume.x1=x-L/2+padding;
    chenarNume.y1=y-H/2+padding+50;
    chenarNume.x2=(LATIME_ECRAN+LATIME_MODAL)/2;
    chenarNume.y2=chenarNume.y1+textheight(numeStr);

    chenarValoare.x1=x-L/2+padding;
    chenarValoare.y1=y-H/2+padding+150;
    chenarValoare.x2=(LATIME_ECRAN+LATIME_MODAL)/2;
    chenarValoare.y2=chenarValoare.y1+textheight(valStr);

}
void chenar (int x1, int y1, int x2, int y2, int grosime, int bkcolor, int framecolor)
{
    setfillstyle(SOLID_FILL,bkcolor);
    setcolor(framecolor);
    bar(x1,y1,x2,y2);
    for (int i=0; i<grosime; ++i)
        rectangle(x1+i,y1+i,x2-i,y2-i);
}
void desenare_piesa (piesa P, int CULOARE)
{
    int x=P.x; int y=P.y;
    int index=P.index;
    int orientare=P.orientare;
    setbkcolor(BLACK);
    setcolor(CULOARE);
    for (int i=0; i<figuri[index].nr_bucati; ++i)
    {
        char type=figuri[index].tip_bucata[i];
        int a=0; int b=0; int c=0; int d=0;
        if (type=='L')
        {
            myLine(orientare, index, i, a, b, c, d, P.zoom);
            line(x+a,y+b,x+c,y+d);
        }
        else if (type=='O')
        {
            myEllipse(orientare, index, i, a, b, c, d, P.zoom);
            ellipse(x+a,y+b,0,360,c,d);
        }
        else if (type=='R')
        {
            myRectangle(orientare, index, i, a, b, c, d, P.zoom);
            rectangle(x+a,y+b,x+c,y+d);
        }
        else if (type=='A')
        {
            myArc(orientare, index, i, a, b, c, d, P.zoom);
            arc(x+a, y+b, (270+orientare*90)%360, (90+orientare*90)%360, c+d);
        }
    }
    //desenare_caracteristici(P);
}
void myIntrari(int orientare, int index, int i, int &a, int &b, double zoom)
{
    float x1 = figuri[index].intrari[i][0];
    float y1 = figuri[index].intrari[i][1];

    float x1_nou = x1, y1_nou = y1;

    for (int j = 0; j < orientare; ++j)
    {
        roteste(x1_nou, y1_nou, x1_nou, y1_nou);
    }

    a = figuri[index].marire * x1_nou * zoom;
    b = figuri[index].marire * y1_nou * zoom;
}
void calcul_intrari(piesa &piesaNoua)
{
    int index=piesaNoua.index;
    int orientare=piesaNoua.orientare;
    for(int j=0; j<figuri[index].nr_intrari; ++j)
    {
        int a=0; int b=0;
        myIntrari(orientare, index, j, a, b, piesaNoua.zoom);
        piesaNoua.intrari[j].x=a+piesaNoua.x;
        piesaNoua.intrari[j].y=b+piesaNoua.y;
    }
}
void incadrare (piesa& piesaNoua, int x, int y, int index)
{
    piesaNoua.unit=-1;
    piesaNoua.orientare=0;
    piesaNoua.x1=piesaNoua.y1=9999;
    piesaNoua.x2=piesaNoua.y2=0;
    for (int i=0; i<figuri[index].nr_bucati; ++i)
    {
        char type=figuri[index].tip_bucata[i];
        int a=figuri[index].marire*figuri[index].bucati[i][0];
        int b=figuri[index].marire*figuri[index].bucati[i][1];
        int c=figuri[index].marire*figuri[index].bucati[i][2];
        int d=figuri[index].marire*figuri[index].bucati[i][3];
        if (type=='L')
        {
            piesaNoua.x1 = min(min(piesaNoua.x1, x + a), x + c);
            piesaNoua.y1 = min(min(piesaNoua.y1, y + b), y + d);
            piesaNoua.x2 = max(max(piesaNoua.x2, x + a), x + c);
            piesaNoua.y2 = max(max(piesaNoua.y2, y + b), y + d);
        }
        else if (type=='O')
        {
            piesaNoua.x1=min(piesaNoua.x1,x+a-c);
            piesaNoua.y1=min(piesaNoua.y1,y+b-d);
            piesaNoua.x2=max(piesaNoua.x2,x+a+c);
            piesaNoua.y2=max(piesaNoua.y2,y+b+d);
        }
        else if (type=='R')
        {
            piesaNoua.x1=min(piesaNoua.x1,x+a);
            piesaNoua.y1=min(piesaNoua.y1,y+b);
            piesaNoua.x2=max(piesaNoua.x2,x+c);
            piesaNoua.y2=max(piesaNoua.y2,y+d);
        }
        else if (type=='A')
        {
            piesaNoua.x1=min(piesaNoua.x1,x+a-c);
            piesaNoua.y1=min(piesaNoua.y1,y+b-c);
            piesaNoua.x2=max(piesaNoua.x2,x+a+c);
            piesaNoua.y2=max(piesaNoua.y2,y+b+c);
        }
    }
    piesaNoua.x=x;
    piesaNoua.y=y;
    piesaNoua.index=index;
    calcul_intrari(piesaNoua);
}
void drawLine(int x1, int y1, int x2, int y2, int cul)
{
    setcolor(cul);
    line(x1, y1, (x1 + x2) / 2, y1);
    line((x1 + x2) / 2, y1, (x1 + x2) / 2, y2);
    line((x1 + x2) / 2, y2, x2, y2);
}
void desenare_legaturi()
{
    for(int i=0; i<=nrPiese; ++i)
    {
        for(int j=0; j<=nrPiese; ++j)
        {
                for(int k=0; k<MAX_INTRARI; ++k)
                {
                    for(int u=0; u<MAX_INTRARI; ++u)
                        if(graf[i][j].intrari[k][u]==1)
                            drawLine(piese[i].intrari[k].x, piese[i].intrari[k].y, piese[j].intrari[u].x, piese[j].intrari[u].y , CULOARE_LEGATURI);
                }
        }
    }
}
int seconds_left_until_restart=30;
void redraw()
{
    DeseneazaBaraDeIteme();
    DeseneazaBaraDeTools();
    setbkcolor(BLACK);
    desenare_legaturi();
   // if (piesaPeCursor)
     //   desenare_piesa_cursor();
    int CULOARE=COLOR(255,255,51);
    for (int i=0; i<=nrPiese; ++i)
    {
        desenare_piesa(piese[i], CULOARE);
        desenare_caracteristici(piese[i]);
    }
}

string getExecutablePath()
{
    char buffer[MAX_PATH];
    GetModuleFileName(NULL, buffer, MAX_PATH);
    return string(buffer);
}


double total_redraw_time_seconds = 0.0; // Cumulative time spent in redraw_page
const double RESTART_THRESHOLD_SECONDS = 60.0;

void restart_application_if_needed()
{
    int val=max(5, int(RESTART_THRESHOLD_SECONDS)-nrPiese);

    if (total_redraw_time_seconds >= val)
    {
        setactivepage(0);
        setvisualpage(0);
        setbkcolor(BLACK);
        setcolor(WHITE);
        cleardevice();
        closegraph();
        string command = getExecutablePath();
        system(command.c_str());
        exit(0);
    }
}
void reopen_application()
{
        setactivepage(0);
        setvisualpage(0);
        setbkcolor(BLACK);
        setcolor(WHITE);
        cleardevice();
        closegraph();
        std::string command = getExecutablePath();
        std::system(command.c_str());
        std::exit(0);
}

void redraw_page() {

    setactivepage(1 - buffer);
    golire_ecran();
    redraw();
    delay(REFRESH_RATE);
    setvisualpage(1 - buffer);
    buffer = 1 - buffer;

    total_redraw_time_seconds += 0.1;

    restart_application_if_needed();
}
void desenare_intrari(int CULOARE);
void redraw_page_with_intrari()
{
    setactivepage(1 - buffer);
    golire_ecran();
    redraw();
    delay(REFRESH_RATE);
    desenare_intrari(WHITE);
    setvisualpage(1 - buffer);
    buffer = 1 - buffer;
    total_redraw_time_seconds += 0.1;
    restart_application_if_needed();
}
void incadrare_PiesaModificata(piesa& piesaVeche)
{
    ///re-initializarea colturilor
    piesaVeche.x1=piesaVeche.y1=9999;
    piesaVeche.x2=piesaVeche.y2=0;

    int index=piesaVeche.index;
    int orientare=piesaVeche.orientare;
    int x=piesaVeche.x;
    int y=piesaVeche.y;
    for (int i=0; i<figuri[index].nr_bucati; ++i)
    {
        char type=figuri[index].tip_bucata[i];
        int a, b, c, d;
        if (type=='L')
        {
            myLine(orientare, index, i, a, b, c, d, piesaVeche.zoom);
            piesaVeche.x1 = min(min(piesaVeche.x1, x + a), x + c);
            piesaVeche.y1 = min(min(piesaVeche.y1, y + b), y + d);
            piesaVeche.x2 = max(max(piesaVeche.x2, x + a), x + c);
            piesaVeche.y2 = max(max(piesaVeche.y2, y + b), y + d);
        }
        else if (type=='O')
        {
            myEllipse(orientare, index, i, a, b, c, d, piesaVeche.zoom);
            piesaVeche.x1=min(piesaVeche.x1,x+a-c);
            piesaVeche.y1=min(piesaVeche.y1,y+b-d);
            piesaVeche.x2=max(piesaVeche.x2,x+a+c);
            piesaVeche.y2=max(piesaVeche.y2,y+b+d);
        }
        else if (type=='R')
        {
            myRectangle(orientare, index, i, a, b, c, d, piesaVeche.zoom);
            piesaVeche.x1=min(piesaVeche.x1,x+a);
            piesaVeche.y1=min(piesaVeche.y1,y+b);
            piesaVeche.x2=max(piesaVeche.x2,x+c);
            piesaVeche.y2=max(piesaVeche.y2,y+d);
        }
        else if (type=='A')
        {
            myArc(orientare, index, i, a, b, c, d, piesaVeche.zoom);
            piesaVeche.x1=min(piesaVeche.x1,x+a-(c+d));
            piesaVeche.y1=min(piesaVeche.y1,y+b-(c+d));
            piesaVeche.x2=max(piesaVeche.x2,x+a+c+d);
            piesaVeche.y2=max(piesaVeche.y2,y+b+c+d);
        }
    }
    calcul_intrari(piesaVeche);
}
/// Functie care detecta ce Item a fost apasat
int getItemIndex(int x, int y)
{
    if (y > INALTIMEA_BAREI_DE_ITEME) return -1;
    return x / (LATIME_ECRAN / NR_ITEME);
}
int getToolIndex(int x, int y)
{
    if(x>LATIME_TOOLBAR || y<=INALTIMEA_BAREI_DE_ITEME) return -1;
    int TOOLS_Inaltime = (INALTIME_ECRAN - INALTIMEA_BAREI_DE_ITEME) / NR_TOOLS;
    return (y - INALTIMEA_BAREI_DE_ITEME) / TOOLS_Inaltime;
}



void AnimareChenar (int state)
{
    setbkcolor(BLACK);
    setcolor(WHITE);
    int gap=5;
    for (int i=0; i<=nrPiese; ++i)
    {
        for (int x=piese[i].x1-gap+5*state; x<= piese[i].x2; x+=10)
            line(x,piese[i].y1-gap,x+5,piese[i].y1-gap);
        state=1-state;
        for (int x=piese[i].x1-gap+5*state; x<= piese[i].x2; x+=10)
            line(x,piese[i].y2+gap,x+5,piese[i].y2+gap);
        state=1-state;
        for (int y=piese[i].y1-gap+5*state; y<= piese[i].y2; y+=10)
            line(piese[i].x2+gap,y,piese[i].x2+gap,y+5);
        state=1-state;
        for (int y=piese[i].y1-gap+5*state; y<= piese[i].y2; y+=10)
            line(piese[i].x1-gap,y,piese[i].x1-gap,y+5);
    }
}
void redraw_page_with_animare_chenar(int state)
{
    setactivepage(1 - buffer);
    delay(REFRESH_RATE);
    golire_ecran();
    redraw();
    AnimareChenar(state);

    setvisualpage(1 - buffer);
    buffer = 1 - buffer;

    total_redraw_time_seconds += 0.1;
    restart_application_if_needed();
}
const int Raza_Intrare=5;
const int Imprecizie=15;

void desenare_intrari(int CULOARE)
{
    setcolor(CULOARE);
    for (int i = 0; i <= nrPiese; ++i)
        for (int j = 0; j < figuri[piese[i].index].nr_intrari; ++j)
            ellipse(piese[i].intrari[j].x, piese[i].intrari[j].y, 0, 360, Raza_Intrare, Raza_Intrare);
}

void stergere_intrari()
{
    ///desenare_intrari(FUNDAL);
    redraw_page();
}

bool mouse_se_intersecteaza_cu_piesa(int x, int y, piesa a)
{
    return (((a.x2+Imprecizie)>=x && x>=(a.x1-Imprecizie)) && ((a.y2+Imprecizie)>=y && y>=(a.y1-Imprecizie)));
}
int mouse_se_interserteaza_cu_intrarea_unei_piese(int x, int y, piesa a)
{
    for(int j = 0; j < figuri[a.index].nr_intrari; ++j)
    {
        int x1=a.intrari[j].x-Raza_Intrare-Imprecizie/2;
        int x2=a.intrari[j].x+Raza_Intrare+Imprecizie/2;
        int y1=a.intrari[j].y-Raza_Intrare-Imprecizie/2;
        int y2=a.intrari[j].y+Raza_Intrare+Imprecizie/2;
        if((x>=x1 && x<=x2) && (y>=y1 && y<=y2))
            return j;
    }
    return -1;
}
int index_figura_apasata(int x, int y)
{
    if(getItemIndex(x, y)!=-1 || getToolIndex(x, y)!=-1)
        return -1;
    for(int i=0; i<=nrPiese; ++i)
    {
        if(mouse_se_intersecteaza_cu_piesa(x, y, piese[i]))
            return i;
    }
    return -1;
}

void identificare_nod(bool &running, int &index, int &index_intrare, int x, int y)
{
    index = index_figura_apasata(x, y);
    if (index == -1)
    {
        cout<<"Nu exista o piesa la ("<< x << ", " << y << ")"<<endl;
        running = false;
        return;
    }

    index_intrare = mouse_se_interserteaza_cu_intrarea_unei_piese(x, y, piese[index]);
    if (index_intrare == -1)
    {
        cout<<"Nu exista o intratre pentru piesa "<<index<<" la: (" << x << ", " << y << ")"<<endl;
        running = false;
        return;
    }

    cout<<"Nod identificat: pentru Piesa "<<index<<",la intrarea cu indexul: "<<index_intrare<<endl;
}


void trasare_legatura()
{
    int Index_01 = -1, Index_intrare_01 = -1;
    int Index_02 = -1, Index_intrare_02 = -1;
    int previousPiesa=-1, previousIntrare=-1;
    bool running_desenare_legaturi = true;

    while (running_desenare_legaturi)
    {
        if (ismouseclick(WM_LBUTTONDOWN))
        {
            int x = mousex();
            int y = mousey();
            clearmouseclick(WM_LBUTTONDOWN);
            cout<<"Primul click la: (" << x << ", " << y << ")"<<endl;

            identificare_nod(running_desenare_legaturi, Index_01, Index_intrare_01, x, y);

            if (Index_01 != -1 && Index_intrare_01 != -1)
            {
                cout<<"Primul nod a fost identificat: Index "<<Index_01<<", Intrare "<<Index_intrare_01<<endl;
                if(piese[Index_01].index!=4)
                    for(int i=0; i<=nrPiese; ++i)
                    {
                        for(int j=0; j<figuri[piese[i].index].nr_intrari; ++j)
                        {
                            if(graf[Index_01][i].intrari[Index_intrare_01][j]==1)
                                {previousPiesa=i; previousIntrare=j;}
                        }
                    }
                break;
            }
            else
            {
                cout<<"Nu a fost gasit un nod valid pentru primul click"<< endl;
            }
        }
    }

    while (running_desenare_legaturi)
    {
        while (!ismouseclick(WM_LBUTTONDOWN))
        {
            int x=mousex();
            int y=mousey();
            ///golire_ecran();
            ///redraw();
            redraw_page_with_intrari();
            drawLine(piese[Index_01].intrari[Index_intrare_01].x,piese[Index_01].intrari[Index_intrare_01].y,x,y,CULOARE_LEGATURI);

        }
        int previous_x=-1, previous_y=-1;
        int x = mousex();
        int y = mousey();
        clearmouseclick(WM_LBUTTONDOWN);


        cout<<"Coordonatele pentru al doilea click: ("<<x<<", "<<y<<")"<<endl;

        identificare_nod(running_desenare_legaturi, Index_02, Index_intrare_02, x, y);

        if (Index_02 != -1 && Index_intrare_02 != -1)
        {
            cout<<"Nodul al doilea a fost identificat la: Index "<<Index_02<< ", Intrare "<<Index_intrare_02<<endl;
            vector<int>suprascriePiesaPornire;
            vector<int>suprascriePiesaDestinatie;
            if(previousIntrare>=0 && previousPiesa>=0)
            {
                graf[Index_01][previousPiesa].intrari[Index_intrare_01][previousIntrare]=0;
                graf[previousPiesa][Index_01].intrari[previousIntrare][Index_intrare_01]=0;
                suprascriePiesaPornire={Index_01,previousPiesa,Index_intrare_01,previousIntrare};
                redraw_page();
                ///cleardevice();
                ///redraw();
            }
            if(piese[Index_02].index!=4)
            {
                previousPiesa=-1; previousIntrare=-1;
                for(int i=0; i<=nrPiese; ++i)
                {
                    for(int j=0; j<figuri[piese[i].index].nr_intrari; ++j)
                    {
                        if(graf[Index_02][i].intrari[Index_intrare_02][j]==1)
                            {previousPiesa=i; previousIntrare=j;}
                    }
                }
                if(previousIntrare>=0 && previousPiesa>=0)
                {
                    graf[Index_02][previousPiesa].intrari[Index_intrare_02][previousIntrare]=0;
                    graf[previousPiesa][Index_02].intrari[previousIntrare][Index_intrare_02]=0;
                    suprascriePiesaDestinatie={Index_02,previousPiesa,Index_intrare_02,previousIntrare};
                    redraw_page();
                    ///cleardevice();
                    ///redraw();
                }
            }
            graf[Index_02][Index_01].intrari[Index_intrare_02][Index_intrare_01]=1;
            graf[Index_01][Index_02].intrari[Index_intrare_01][Index_intrare_02]=1;
            modificari[q=++p]={vector<int>{4},vector<piesa>{}};
            legaturi_modificate[p]={{Index_01,Index_02,Index_intrare_01,Index_intrare_02}};
            if (suprascriePiesaPornire.size()>0) legaturi_modificate[p].push_back(suprascriePiesaPornire);
            if (suprascriePiesaDestinatie.size()>0) legaturi_modificate[p].push_back(suprascriePiesaDestinatie);
            break;
        }
        else if(Index_02==-1 && Index_intrare_02==-1) ///I.e daca x si y nu se intersercteaza cu o piesa sau o intrare de pe tabla
        {
            int IndexPiesa=4;
            piesa piesaNoua;
            piesaNoua.index=4;
            piesaNoua.orientare=0;
            piesaNoua.zoom=1;
            piesaNoua.nume[0]='\0';
            piesaNoua.valoare[0]='\0';
            incadrare(piesaNoua,x,y,IndexPiesa);
            if (sePoateDesena(piesaNoua,x,y,IndexPiesa))
            {
                int CULOARE=COLOR(255,255,51);
                desenare_piesa(piesaNoua, CULOARE);
                piese[++nrPiese]=piesaNoua;
                modificari[q=++p]=make_pair(vector<int>{0,nrPiese},vector<piesa>{piese[nrPiese]});
                Index_02=nrPiese; Index_intrare_02=0;
                graf[Index_01][Index_02].intrari[Index_intrare_01][Index_intrare_02]=1;
                graf[Index_02][Index_01].intrari[Index_intrare_02][Index_intrare_01]=1;
                modificari[q=++p]={{4},{}};
                legaturi_modificate[p]={{Index_01,Index_02,Index_intrare_01,Index_intrare_02}};
                if(previousIntrare>=0 && previousPiesa>=0)
                {
                    graf[Index_01][previousPiesa].intrari[Index_intrare_01][previousIntrare]=0;
                    graf[previousPiesa][Index_01].intrari[previousIntrare][Index_intrare_01]=0;
                    redraw_page();
                    ///cleardevice();
                    ///redraw();
                }
                //else
                    drawLine(piese[Index_01].intrari[Index_intrare_01].x, piese[Index_01].intrari[Index_intrare_01].y,piese[Index_02].intrari[Index_intrare_02].x,piese[Index_02].intrari[Index_intrare_02].y,CULOARE_LEGATURI);

            }
            else
            {
                cout<<"Niciun nod valid nu a fost gasit pentru al doilea click"<<endl;
            }
        }
    }
    redraw_page();
}
void modal (piesa &P)
{
    generare_coordonate_chenare();
    int x=LATIME_ECRAN/2;
    int y=INALTIME_ECRAN/2;
    int L=LATIME_MODAL;
    int H=INALTIME_MODAL;
    chenar(chenarModal.x1,chenarModal.y1,chenarModal.x2,chenarModal.y2,5,BKMODAL,GREEN);
    int padding=30;
    int COLOR_NA=YELLOW;
    int COLOR_A=COLOR(238, 75, 43);

    char *numeStr="Nume:";
    char *valStr="Valoare:";
    char *unitStr="Unitate masura:";
    settextstyle(DEFAULT_FONT,0,3);
    setcolor(COLOR_NA);
    setbkcolor(BKMODAL);
    if (campActiv==0) setcolor(COLOR_A);
    outtextxy(chenarNume.x1,chenarNume.y1,numeStr);
        outtextxy(chenarNume.x1+textwidth(numeStr)+5,chenarNume.y1,P.nume);
    if (campActiv==0) setcolor(COLOR_NA);
    if (campActiv==1) setcolor(COLOR_A);
    outtextxy(chenarValoare.x1,chenarValoare.y1,valStr);
        outtextxy(chenarValoare.x1+textwidth(valStr)+5,chenarValoare.y1,P.valoare);
    if (campActiv==1) setcolor(COLOR_NA);


    outtextxy(x-L/2+padding,y-H/2+padding+250,unitStr);
       // outtextxy(x-L/2+padding+textwidth(unitStr)+5,y-H/2+padding+250,nume);

    int optionBk=BKMODAL;
    if (P.unit==0) optionBk=selectionColor;
    chenar(chenarOhm.x1,chenarOhm.y1,chenarOhm.x2,chenarOhm.y2,2,optionBk,GREEN);
    drawOmega(x-L/2+padding+textwidth(unitStr)+15,y-H/2+padding+250+10,10,0);
    if (P.unit==0) optionBk=BKMODAL;
    if (P.unit==1) optionBk=selectionColor;
    chenar(chenarAmp.x1,chenarAmp.y1,chenarAmp.x2,chenarAmp.y2,2,optionBk,GREEN);
    setcolor(unitColor);
    setbkcolor(optionBk);
    outtextxy(x-L/2+padding+textwidth(unitStr)+45,y-H/2+padding+250,"A");
    if (P.unit==1) optionBk=BKMODAL;
    if (P.unit==2) optionBk=selectionColor;
    chenar(chenarVolt.x1,chenarVolt.y1,chenarVolt.x2,chenarVolt.y2,2,optionBk,GREEN);
    setcolor(unitColor);
    setbkcolor(optionBk);
    outtextxy(x-L/2+padding+textwidth(unitStr)+85,y-H/2+padding+250,"V");
    if (P.unit==2) optionBk=BKMODAL;


    //buton salvare
    char *salvareStr="Salvare";
    chenar(chenarButon.x1,chenarButon.y1,chenarButon.x2,chenarButon.y2,3,LIGHTGRAY,GREEN);
    setcolor(GREEN);
    setbkcolor(LIGHTGRAY);
    settextstyle(0,0,4);
    outtextxy((chenarButon.x1+chenarButon.x2)/2-textwidth(salvareStr)/2,(chenarButon.y1+chenarButon.y2)/2-textheight(salvareStr)/2,salvareStr);

    //x
    settextstyle(1,0,3);
    setcolor(WHITE);
    setbkcolor(BKMODAL);
    outtextxy(chenarX.x1,chenarX.y1,"x");
}

void copiereCuvant (vector<char>&V, char *W)
{
    V.resize(strlen(W)+1);
    for (int i=0; i<=strlen(W); ++i)
        V[i]=W[i];
}
void copiereCuvant (char *V, vector<char>W)
{
    int i=0;
    for (; W[i]!='\0'; ++i)
        V[i]=W[i];
    V[i]='\0';
}
void citire_modal (piesa &P, int indice_piesa)
{

    campActiv=-1;

    int x=LATIME_ECRAN/2;
    int y=INALTIME_ECRAN/2;
    int L=LATIME_MODAL;
    int H=INALTIME_MODAL;
    int padding=30;
    int COLOR_A=COLOR(238, 75, 43);
    int COLOR_NA=YELLOW;

    char *numeStr="Nume:";
    char *valStr="Valoare:";
    char *unitStr="Unitate masura:";

    char ch='A';
    char copieNume[100];
    int copieUnit=P.unit;
    char copieVal[100];
    strcpy(copieVal,P.valoare);
    strcpy(copieNume,P.nume);


    while (1)
    {

        if (ismouseclick(WM_LBUTTONDOWN))
        {
            int x=mousex();
            int y=mousey();
            clearmouseclick(WM_LBUTTONDOWN);
            if (inchenar(x,y,chenarNume))
            {
                cout<<chenarNume.x2<<" "<<chenarNume.y2<<'\n'<<chenarValoare.x2<<" "<<chenarValoare.y2<<'\n';
                int n=strlen(P.nume);
                campActiv=0;
                    ///golire_ecran();
                    ///redraw();
                    redraw_page();
                    modal(P);
                char ch='A';
                while (true)
                {
                    ch=getch();
                    if (ch=='\r') break;
                    else if (ch=='\b')
                    {
                        if (n>0)
                        {
                            n--;
                            P.nume[n]='\0';
                        }
                    }
                    else{
                        P.nume[n++]=ch;
                        P.nume[n]='\0';
                    }
                    ///golire_ecran();
                    ///redraw();
                    redraw_page();
                    modal(P);
                    delay(REFRESH_RATE);
                }
                campActiv=-1;
            }
            else if (inchenar(x,y,chenarValoare))
            {
                int n=strlen(P.valoare);
                campActiv=1;
                    ///golire_ecran();
                    ///redraw();
                    redraw_page();
                    modal(P);
                char ch='A';
                while (true)
                {
                    ch=getch();
                    if (ch=='\r') break;
                    else if (ch=='\b')
                    {
                        if (n>0)
                        {
                            n--;
                            P.valoare[n]='\0';
                        }
                    }
                    else{
                        P.valoare[n++]=ch;
                        P.valoare[n]='\0';
                    }
                    ///golire_ecran();
                    ///redraw();
                    redraw_page();
                    modal(P);
                    delay(REFRESH_RATE);
                }
                campActiv=-1;
            }
            else if (inchenar(x,y,chenarOhm))
            {
                P.unit=0;
            }
            else if (inchenar(x,y,chenarAmp))
            {
                P.unit=1;
            }
            else if (inchenar(x,y,chenarVolt))
            {
                P.unit=2;
            }
            else if (inchenar(x,y,chenarButon))
            {
                //salvare
                if (!(strcmp(copieNume,P.nume)==0 && strcmp(copieVal,P.valoare)==0 && P.unit==copieUnit))
                {
                    modificari[q=++p]={{7,indice_piesa, copieUnit,P.unit},{}};
                    caracteristici_modificate[q].clear();
                    caracteristici_modificate[q].resize(4);
                    for (int i=0; i<4; ++i)
                        caracteristici_modificate[q][i].resize(50);

                    copiereCuvant(caracteristici_modificate[q][0],copieNume);
                    copiereCuvant(caracteristici_modificate[q][1],P.nume);
                    copiereCuvant(caracteristici_modificate[q][2],copieVal);
                    copiereCuvant(caracteristici_modificate[q][3],P.valoare);
                }
                delay(REFRESH_RATE);
                setvisualpage(1 - buffer);
                buffer=1-buffer;
                break;
            }
            else if (inchenar(x,y,chenarX) || !inchenar(x,y,chenarModal))
            {
                strcpy(P.nume,copieNume);
                strcpy(P.valoare,copieVal);
                P.unit=copieUnit;
                delay(REFRESH_RATE);
                setvisualpage(1 - buffer);
                buffer=1-buffer;
                break;
            }

        }
        setvisualpage(buffer);
        setactivepage(1 - buffer);
        golire_ecran();
        redraw();
        setfillstyle(1,BLACK);
        bar(chenarModal.x1,chenarModal.y1,chenarModal.x2,chenarModal.y2);
        modal(P);
        delay(REFRESH_RATE);
        delay(REFRESH_RATE);
        setvisualpage(1 - buffer);
        buffer=1-buffer;

    }

    redraw_page();
}
int cauta_piesa()
{
    int x=mousex();
    int y=mousey();
    piesa pseudopiesa;
    pseudopiesa.x1=pseudopiesa.x2=x;
    pseudopiesa.y1=pseudopiesa.y2=y;
    for (int i=0; i<=nrPiese;++i)
        if (seIntersecteaza(piese[i],pseudopiesa))
            return i;
    return -1;
}
void stergere_piesa()
{
    int i=cauta_piesa();
    if (i!=-1)
    {
        modificari[q=++p]={vector<int>{1,i},vector<piesa>{piese[i]}};
        legaturi_modificate[p].resize(0);
        for (int j=0; j<=nrPiese; ++j)
            for (int e=0; e<MAX_INTRARI; ++e)
                for (int f=0; f<MAX_INTRARI; ++f)
                {
                    if (graf[i][j].intrari[e][f]){
                        legaturi_modificate[p].push_back({i,j,e,f});
                        legaturi_modificate[p].push_back({j,i,f,e});
                    }
                }
        for (int j=0; j<=nrPiese; ++j)
            for (int e=0; e<MAX_INTRARI; ++e)
                for (int f=0; f<MAX_INTRARI; ++f)
                {
                    graf[i][j].intrari[e][f]=graf[nrPiese][j].intrari[e][f]; //mutam legatura nrPiese pe pozitia i
                    graf[nrPiese][j].intrari[e][f]=0; //stergem ce era pe pozitia nrPiese


                    graf[j][i].intrari[f][e]=graf[j][nrPiese].intrari[f][e];
                    graf[j][nrPiese].intrari[f][e]=0;
                }
        piese[i]=piese[nrPiese--];
        cout<<"Piesa stearsa este"<<i<<"\n";
    }
    ///golire_ecran();
    ///redraw();
    redraw_page();
}
void AsteptareSelectie()
{
    int state=0;
    while (!ismouseclick(WM_LBUTTONDOWN))
    {
        AnimareChenar(state);
        delay(500);
        state=1-state;
        redraw_page_with_animare_chenar(state);
        ///golire_ecran();
        ///redraw();
    }
}
void rotire()
{
    AsteptareSelectie();
    int x = mousex();
    int y = mousey();
    clearmouseclick(WM_LBUTTONDOWN);
    int i=index_figura_apasata(x, y);
    //desenare_piesa(piese[i], FUNDAL);

    piesa piesaInitiala=piese[i];
    piese[i].orientare=(piese[i].orientare+1)%4;
    incadrare_PiesaModificata(piese[i]);

    piesa copiePiesa=piese[i];
    piese[i].x1=piese[i].x2=piese[i].y1=piese[i].y2=0;
    if (sePoateDesena(copiePiesa,piese[i].x,piese[i].y,piese[i].index))
    {
        piese[i]=copiePiesa;
        modificari[q=++p]={vector<int>{3,i},vector<piesa>{piesaInitiala,piese[i]}};
    }
    else
    {
        piese[i]=piesaInitiala;
        incadrare_PiesaModificata(piese[i]);
    }


    //int CULOARE=COLOR(255,255,51);

    ///golire_ecran();
    ///redraw();

}
void mutare_piesa ()
{

    AsteptareSelectie();
    int i=cauta_piesa();

    piesa piesaInitiala=piese[i];
    clearmouseclick(WM_LBUTTONDOWN);
    while (true && i!=-1)
    {
        int x=mousex();
        int y=mousey();
        piese[i].x=x;
        piese[i].y=y;
        incadrare_PiesaModificata(piese[i]);
        calcul_intrari(piese[i]);

        if (ismouseclick(WM_LBUTTONDOWN))
        {
            int x=mousex();
            int y=mousey();
            clearmouseclick(WM_LBUTTONDOWN);
            piese[i]=piesaInitiala;
            piesa copiePiesa=piesaInitiala;
            piese[i].x=piese[i].x2=piese[i].y1=piese[i].y2=0;
            copiePiesa.x=x;
            copiePiesa.y=y;
            incadrare_PiesaModificata(copiePiesa);
            if (sePoateDesena(copiePiesa,x,y,copiePiesa.index))
            {
                piese[i]=copiePiesa;
                cout<<"S a mutat piesa "<<i<<'\n';
                modificari[q=++p]={vector<int>{2,i},vector<piesa>{piesaInitiala,piese[i]}};
            }
            else piese[i]=piesaInitiala;
            break;
        }
        redraw_page();
    }

    redraw_page();
}
void plasare_piesa_noua(int Item_Selectat)
{
    //cream piesa noua
    piese[++nrPiese].index=Item_Selectat;
    piese[nrPiese].orientare=0;
    piese[nrPiese].zoom=1;
    piese[nrPiese].nume[0]='\0';
    piese[nrPiese].valoare[0]='\0';

    //asteptam plasarea
    while (!ismouseclick(WM_LBUTTONDOWN))
    {
        int x=mousex();
        int y=mousey();
        piese[nrPiese].x=x;
        piese[nrPiese].y=y;
        incadrare(piese[nrPiese],x,y,Item_Selectat);
        redraw_page();
    }
    int x=mousex();
    int y=mousey();
    clearmouseclick(WM_LBUTTONDOWN);
    incadrare(piese[nrPiese],x,y,Item_Selectat);
    piesa copiePiesa=piese[nrPiese--];
    if (!sePoateDesena(copiePiesa,x,y,Item_Selectat))
        ;
    else{
        piese[++nrPiese]=copiePiesa;
        modificari[q=++p]=make_pair(vector<int>{0,nrPiese},vector<piesa>{piese[nrPiese]});
    }
    redraw_page();
}
void erase_all()
{
    modificari[q=++p]={{6,nrPiese},{}};
    legaturi_modificate[p].resize(0);
    for (int i=0; i<=nrPiese; ++i)
        for (int j=i+1; j<=nrPiese; ++j)
            for (int e=0; e<MAX_INTRARI; ++e)
                for (int f=0; f<MAX_INTRARI; ++f)
                {
                    if (graf[i][j].intrari[e][f])
                    {
                        legaturi_modificate[p].push_back({i,j,e,f});
                        graf[i][j].intrari[e][f]=0;
                        graf[j][i].intrari[f][e]=0;
                    }
                }
    nrPiese=-1;
    redraw_page();
}
void undo()
{
    if (p>=0)
    {
        int caz=modificari[p].first[0];
        int poz=modificari[p].first[1];
        piesa piesaVeche;
        if (caz!=4 && caz!=6 && caz!=7) piesaVeche=modificari[p].second[0];
        piesa piesaNoua;
        if (caz==2 || caz==3 || caz==5) piesaNoua=modificari[p].second[1];
        switch(caz)
        {
            case 0: //piesa adaugata
                piese[poz]=piese[nrPiese--];
                break;
            case 1: //piesa stearsa
                piese[++nrPiese]=piese[poz];
                piese[poz]=piesaVeche;
                for (int j=0; j<=nrPiese; ++j)
                    for (int e=0; e<MAX_INTRARI; ++e)
                        for (int f=0;f<MAX_INTRARI; ++f){
                            graf[nrPiese][j].intrari[e][f]=graf[poz][j].intrari[e][f];
                            graf[poz][j].intrari[e][f]=0;
                            graf[j][nrPiese].intrari[e][f]=graf[j][poz].intrari[e][f];
                            graf[j][poz].intrari[e][f]=0;
                        }
                for (auto e:legaturi_modificate[p]){
                    graf[e[0]][e[1]].intrari[e[2]][e[3]]=1;
                    graf[e[1]][e[0]].intrari[e[3]][e[2]]=1;
                }
                break;
            case 2://piesa mutata
            case 3: //piesa rotita
                piese[poz]=piesaVeche;
                break;
            case 4:{ //adaugare legatura
                int p1=legaturi_modificate[p][0][0];
                int p2=legaturi_modificate[p][0][1];
                int i1=legaturi_modificate[p][0][2];
                int i2=legaturi_modificate[p][0][3];
                graf[p1][p2].intrari[i1][i2]=0;
                graf[p2][p1].intrari[i2][i1]=0;
                for (int i=1; i<legaturi_modificate[p].size(); ++i)
                {
                    p1=legaturi_modificate[p][i][0];
                    p2=legaturi_modificate[p][i][1];
                    i1=legaturi_modificate[p][i][2];
                    i2=legaturi_modificate[p][i][3];
                    graf[p1][p2].intrari[i1][i2]=1;
                    graf[p2][p1].intrari[i2][i1]=1;
                }
                break;
            }
            case 5: //redimensionare
                piese[poz]=piesaVeche;
                break;
            case 6://sterge tot
                nrPiese=poz;
                for (auto e:legaturi_modificate[p]){
                    graf[e[0]][e[1]].intrari[e[2]][e[3]]=1;
                    graf[e[1]][e[0]].intrari[e[3]][e[2]]=1;
                }
                break;
            case 7://redenumire
                copiereCuvant(piese[poz].nume,caracteristici_modificate[p][0]);
                copiereCuvant(piese[poz].valoare,caracteristici_modificate[p][2]);

                piese[poz].unit=modificari[p].first[2];
                break;
        }
        p--;
    }
    redraw_page();
}
void redo()
{
    if (p<q)
    {
        p++;
        int caz=modificari[p].first[0];
        int poz=modificari[p].first[1];
        piesa piesaVeche;
        if (caz!=4 && caz!=6 && caz!=7) piesaVeche=modificari[p].second[0];
        piesa piesaNoua;
        if (caz==2 || caz==3 || caz==5) piesaNoua=modificari[p].second[1];
        switch(caz)
        {
            case 0: //piesa adaugata
                piese[++nrPiese]=piese[poz];
                piese[poz]=piesaVeche;
                break;
            case 1: //piesa stearsa
                for (int j=0; j<=nrPiese; ++j)
                    for (int e=0; e<MAX_INTRARI; ++e)
                        for (int f=0;f<MAX_INTRARI; ++f)
                        {
                            graf[poz][j].intrari[e][f]=graf[nrPiese][j].intrari[e][f];
                            graf[nrPiese][j].intrari[e][f]=0;
                            graf[j][poz].intrari[e][f]=graf[j][nrPiese].intrari[e][f];
                            graf[j][nrPiese].intrari[e][f]=0;
                        }
                piese[poz]=piese[nrPiese--];
                break;
            case 2://piesa mutata
            case 3: //piesa rotita
                piese[poz]=piesaNoua;
                break;
            case 4:{ //adaugare legatura
                int p1=legaturi_modificate[p][0][0];
                int p2=legaturi_modificate[p][0][1];
                int i1=legaturi_modificate[p][0][2];
                int i2=legaturi_modificate[p][0][3];
                graf[p1][p2].intrari[i1][i2]=1;
                graf[p2][p1].intrari[i2][i1]=1;

                for (int i=1; i<legaturi_modificate[p].size(); ++i)
                {
                    p1=legaturi_modificate[p][i][0];
                    p2=legaturi_modificate[p][i][1];
                    i1=legaturi_modificate[p][i][2];
                    i2=legaturi_modificate[p][i][3];
                    graf[p1][p2].intrari[i1][i2]=0;
                    graf[p2][p1].intrari[i2][i1]=0;
                }
                break;
            }
            case 5: //redimensionare
                piese[poz]=piesaNoua;
                break;
            case 6://sterge tot
                nrPiese=-1;
                for (auto e:legaturi_modificate[p]){
                    graf[e[0]][e[1]].intrari[e[2]][e[3]]=0;
                    graf[e[1]][e[0]].intrari[e[3]][e[2]]=0;
                }
                break;
            case 7://redenumire
                copiereCuvant(piese[poz].nume,caracteristici_modificate[p][1]);
                copiereCuvant(piese[poz].valoare,caracteristici_modificate[p][3]);

                piese[poz].unit=modificari[p].first[3];
                break;
        }
    }
    redraw_page();
}
void slider ()
{
    AsteptareSelectie();
    int x = mousex();
    int y = mousey();
    clearmouseclick(WM_LBUTTONDOWN);
    int i=index_figura_apasata(x, y);
    if (i==-1) {
        redraw_page();
        return;
    }
    piesa beforeResizing=piese[i];

    int xSlider=piese[i].x,ySlider=piese[i].y+100;

    int lungimeSlider=180;
    int inaltimeSlider=12;
    int lungimeFereastra=200;
    int inaltimeFereastra=40;
    setfillstyle(1,LIGHTGRAY);

    char *str02="0,2x";
    char *str7="4x";

    bool isRunning=1;
    y=ySlider;
    while (isRunning)
    {
        //desenam sliderul
        chenar(xSlider-lungimeSlider/2,ySlider-inaltimeSlider/2,xSlider+lungimeSlider/2,ySlider+inaltimeSlider/2,3,LIGHTGRAY,BROWN);
        settextstyle(9,0,1);
        setcolor(GREEN);
        outtextxy(xSlider-lungimeSlider/2,ySlider-inaltimeSlider/2-textheight(str02),str02);
        outtextxy(xSlider+lungimeSlider/2-textwidth(str7),ySlider-inaltimeSlider/2-textheight(str7),str7);

        //slider
        if (piese[i].zoom<=1) x=((piese[i].zoom-0.2)/0.8+1.0*(xSlider-lungimeSlider/2)/(lungimeSlider/2))*(lungimeSlider/2);
        else x=((piese[i].zoom-1)/4.0)*(lungimeSlider/2)+xSlider;

        setfillstyle(1,GREEN);
        chenar(x-3,ySlider-15,x+3,ySlider+15,1,DARKGRAY,LIGHTGRAY);
        if (ismouseclick(WM_LBUTTONDOWN))
        {
            x=mousex();
            y=mousey();
            clearmouseclick(WM_LBUTTONDOWN);
            if (x>=xSlider-lungimeSlider/2 && x<=xSlider+lungimeSlider/2 && y>=ySlider-inaltimeSlider/2-5 && y<=ySlider+inaltimeSlider/2+5)
            {
                piesa piesaInitiala=piese[i];
                if (x<xSlider) ///<1
                {
                    piese[i].zoom=0.8*(x-(xSlider-lungimeSlider/2))/(lungimeSlider/2)+0.2;
                }
                else ///>=1
                {
                    piese[i].zoom=4.0*(x-xSlider)/(lungimeSlider/2)+1;
                }
                incadrare_PiesaModificata(piese[i]);
                piesa piesaModificata=piese[i];

                piese[i].x1=piese[i].x2=piese[i].y1=piese[i].y2=0;
                if (sePoateDesena(piesaModificata,piese[i].x,piese[i].y,piese[i].index))
                    piese[i]=piesaModificata;
                else
                {
                    piese[i]=piesaInitiala;
                }
            }
            else
            {
                isRunning=0;
            }
        }
        redraw_page();
    }
    if (piese[i].zoom!=beforeResizing.zoom)
        modificari[q=++p]={{5,i},{beforeResizing,piese[i]}};
}
int searchIndexByName (char *name)
{
    for (int i=0; i<NR_ITEME; ++i)
        if (0==strcmp(name,figuri[i].nume))
            return i;
    return -1;
}

void set_project_directory()
{
    char exePath[MAX_PATH];
    if (GetModuleFileNameA(NULL, exePath, MAX_PATH) == 0)
    {
        cout<<" eroare la gasirea fisierului .exe "<< endl;
        return;
    }

    int length = strlen(exePath);
    int lastSlash = -1;
    for (int i = length - 1; i >= 0; --i)
    {
        if (exePath[i] == '\\' || exePath[i] == '/')
        {
            lastSlash = i;
            break;
        }
    }

    if (lastSlash == -1)
    {
        cout << "Eroare: Nu s-a putut determina locatia proiectului" << endl;
        return;
    }

    exePath[lastSlash] = '\0';

    int secondLastSlash = -1;
    for (int i = lastSlash - 1; i >= 0; --i)
    {
        if (exePath[i] == '\\' || exePath[i] == '/')
        {
            secondLastSlash = i;
            break;
        }
    }

    if (secondLastSlash == -1)
    {
        cout << "Eroare: Nu s-a putut determina locatia proiectului" << endl;
        return;
    }
    int thirdLastSlash = -1;
    for (int i = secondLastSlash - 1; i >= 0; --i)
    {
        if (exePath[i] == '\\' || exePath[i] == '/')
        {
            thirdLastSlash = i;
            break;
        }
    }
    exePath[thirdLastSlash] = '\0';
    if (SetCurrentDirectoryA(exePath) == 0)
    {
        cout<< "Eroare la setarea directorului " <<endl;
        return;
    }

    cout << "directory-ul a fost setat la: " << exePath <<endl;
}


void salvare_circuit()
{
    OPENFILENAME ofn;
    char szFileName[MAX_PATH] = "";

    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = NULL;
    ofn.lpstrFilter = "Text Files (*.txt)\0*.txt\0All Files (*.*)\0*.*\0";
    ofn.lpstrFile = szFileName;
    ofn.nMaxFile = MAX_PATH;
    ofn.lpstrTitle = "Salveaza circuit...";
    ofn.Flags = OFN_OVERWRITEPROMPT;

    if (GetSaveFileNameA(&ofn))
    {
        ofstream fout(ofn.lpstrFile);
        if (!fout.is_open()) {
            cerr << "Error: Nu se poate deschide fisierul " << ofn.lpstrFile << " pentru citire." << '\n';
            return;
        }

        fout<<nrPiese<<"\n\n";

        for (int i=0; i<=nrPiese; ++i)
        {
            fout<<figuri[piese[i].index].nume<<'\n';
            fout<<piese[i].nume<<'\n'<<piese[i].valoare<<'\n'<<piese[i].unit<<'\n';
            fout<<piese[i].x<<" "<<piese[i].y<<'\n';
            fout<<piese[i].orientare<<" "<<piese[i].zoom<<'\n';
            fout<<'\n';
        }
        fout<<'\n';
        for (int i=0; i<=nrPiese;++i)
            for (int j=i+1; j<=nrPiese; ++j)
                for (int e=0; e<MAX_INTRARI; ++e)
                    for (int f=0; f<MAX_INTRARI; ++f)
                        if (graf[i][j].intrari[e][f])
                            fout<<i<<" "<<j<<" "<<e<<" "<<f<<'\n';
        fout.close();
    }
}


void import_circuit ()
{
    OPENFILENAME ofn;
    char szFileName[MAX_PATH] = "";

    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = NULL;
    ofn.lpstrFilter = "Text Files (*.txt)\0*.txt\0All Files (*.*)\0*.*\0";
    ofn.lpstrFile = szFileName;
    ofn.nMaxFile = MAX_PATH;
    ofn.lpstrTitle = "Load Circuit File";
    ofn.Flags = OFN_FILEMUSTEXIST;

    if (GetOpenFileNameA(&ofn))
    {
        ifstream fin(ofn.lpstrFile);
        if (!fin.is_open()) {
            cerr << "Error: Nu se poate deschide fisierul " << ofn.lpstrFile << " pentru citire." << '\n';
            return;
        }
        //reinitializare structuri
        for (int i=0; i<=nrPiese; ++i)
            for (int j=0; j<=nrPiese; ++j)
                for (int e=0; e<MAX_INTRARI; ++e)
                    for (int f=0; f<MAX_INTRARI; ++f)
                        graf[i][j].intrari[e][f]=0;
        nrPiese=-1;
        p=q=-1;

        //citire fisier
        fin>>nrPiese; fin.get();
        fin.get();
        for (int i=0; i<=nrPiese; ++i)
        {
            char name[30];
            fin.getline(name,30);
            piese[i].index=searchIndexByName(name);
            fin.getline(piese[i].nume,30);
            fin.getline(piese[i].valoare,30);
            fin>>piese[i].unit; fin.get();
            fin>>piese[i].x>>piese[i].y;
            fin>>piese[i].orientare>>piese[i].zoom;
            incadrare_PiesaModificata(piese[i]);
            calcul_intrari(piese[i]);
            fin.get();
            fin.get();
        }
        fin.get();
        int x,y,i,j;
        while (fin>>x>>y>>i>>j)
        {
            graf[x][y].intrari[i][j]=graf[y][x].intrari[j][i]=1;
        }
    }
    redraw_page();
}

const string AUTOSAVE_FILE = "autosave.txt";

void passive_save()
{
    ofstream fout(AUTOSAVE_FILE, std::ios::trunc);
    if (!fout.is_open()) {
        cout << "Eroare: Nu s-a putut deschide fisierul de 'autosalvare' ." << endl;
        return;
    }

    fout << nrPiese << "\n\n";

    for (int i = 0; i <= nrPiese; ++i) {
        fout << figuri[piese[i].index].nume << '\n';
        fout << piese[i].nume << '\n' << piese[i].valoare << '\n' << piese[i].unit << '\n';
        fout << piese[i].x << " " << piese[i].y << '\n';
        fout << piese[i].orientare << " " << piese[i].zoom << '\n';
        fout << '\n';
    }

    fout << '\n';

    for (int i = 0; i <= nrPiese; ++i) {
        for (int j = i + 1; j <= nrPiese; ++j) {
            for (int e = 0; e < MAX_INTRARI; ++e) {
                for (int f = 0; f < MAX_INTRARI; ++f) {
                    if (graf[i][j].intrari[e][f]) {
                        fout << i << " " << j << " " << e << " " << f << '\n';
                    }
                }
            }
        }
    }

    fout.close();
    cout<<"Starea circuitului a fost salvata in: " << AUTOSAVE_FILE<<endl;
}

void load_passive_save() {
    ifstream fin(AUTOSAVE_FILE);
    if (!fin.is_open()) {
        cout <<"Fisierul de 'auto-salvare' nu a fost gasit " <<endl;
        return;
    }

    for (int i = 0; i <= nrPiese; ++i) {
        for (int j = 0; j <= nrPiese; ++j) {
            for (int e = 0; e < MAX_INTRARI; ++e) {
                for (int f = 0; f < MAX_INTRARI; ++f) {
                    graf[i][j].intrari[e][f] = 0;
                }
            }
        }
    }
    nrPiese = -1;
    p = q = -1;
    fin >> nrPiese;
    fin.ignore();
    fin.ignore();

    for (int i = 0; i <= nrPiese; ++i) {
        char name[30];
        fin.getline(name, 30);
        piese[i].index = searchIndexByName(name);
        fin.getline(piese[i].nume, 30);
        fin.getline(piese[i].valoare, 30);
        fin >> piese[i].unit;
        fin.ignore();
        fin >> piese[i].x >> piese[i].y;
        fin >> piese[i].orientare >> piese[i].zoom;
        incadrare_PiesaModificata(piese[i]);
        calcul_intrari(piese[i]);
        fin.ignore();
        fin.ignore();
    }
    int x, y, i, j;
    while (fin >> x >> y >> i >> j) {
        graf[x][y].intrari[i][j] = graf[y][x].intrari[j][i] = 1;
    }

    fin.close();
    cout << "starea circuitului a fost incarcata din " << AUTOSAVE_FILE << std::endl;

    redraw_page();
}

const char START_MENU_IF_FILE[] = "start_menu_flag.txt";

void create_start_menu();
void main_application_loop();
void load_passive_save();
void import_circuit();

void Tool_Cases(int index)
{
    switch (index)
    {
        case 1:
            desenare_intrari(WHITE);
            trasare_legatura();
            passive_save();
            break;
        case 2:
        {
            rotire();
            redraw_page();
            passive_save();
            break;
        }
        case 3:
            mutare_piesa();
            passive_save();
            break;
        case 4:
            //redimensionare
            slider();
            passive_save();
            break;
        case 5:
            AsteptareSelectie();
            stergere_piesa();
            passive_save();
            break;
        case 6:
            erase_all();
            passive_save();
            break;
        case 7:
            undo();
            passive_save();
            break;
        case 8:
            redo();
            passive_save();
            break;
        case 9:
            salvare_circuit();
            set_project_directory();
            passive_save();
            break;
        case 0:
            passive_save();
            remove(START_MENU_IF_FILE);
            reopen_application();
            break;
        default:
            return;
    }
}


void DeseneazaItem(int index)
{
    if(index!=-1)
    {
        int Lungimea_Barei_Iteme = LATIME_ECRAN / NR_ITEME;
        int i=index;
        setfillstyle(SOLID_FILL, LIGHTGRAY);
        bar(i * Lungimea_Barei_Iteme, 0, (i + 1) * Lungimea_Barei_Iteme, INALTIMEA_BAREI_DE_ITEME);

        setcolor(BLACK);
        rectangle(i * Lungimea_Barei_Iteme, 0, (i + 1) * Lungimea_Barei_Iteme, INALTIMEA_BAREI_DE_ITEME);
        setcolor(COLOR(255, 255, 51));
        int new_x = i * Lungimea_Barei_Iteme + Lungimea_Barei_Iteme / 2;
        int new_y = INALTIMEA_BAREI_DE_ITEME / 2;
        int index = i;

        for (int j = 0; j < figuri[index].nr_bucati; ++j)
        {
            char type = figuri[index].tip_bucata[j];
            int a = figuri[index].marire * figuri[index].bucati[j][0];
            int b = figuri[index].marire * figuri[index].bucati[j][1];
            int c = figuri[index].marire * figuri[index].bucati[j][2];
            int d = figuri[index].marire * figuri[index].bucati[j][3];

            if (type == 'L')
            {
                line(new_x + a, new_y + b, new_x + c, new_y + d);
            }
            else if (type == 'O')
            {
                ellipse(new_x + a, new_y + b, 0, 360, c, d);
            }
            else if (type == 'R')
            {
                rectangle(new_x + a, new_y + b, new_x + c, new_y + d);
            }
            else if (type == 'A')
            {
                arc(new_x + a, new_y + b, 270, 90, c+d+0.5);
            }
        }
    }
    else
        return;
}
void DeseneazaTool(int index)
{
    if (index != -1)
    {
        int i = index;
        int TOOLS_Inaltime = (INALTIME_ECRAN - INALTIMEA_BAREI_DE_ITEME) / NR_TOOLS;
        setbkcolor(BLACK);
        setfillstyle(SOLID_FILL, DARKGRAY);
        bar(0, 1 + INALTIMEA_BAREI_DE_ITEME + i * TOOLS_Inaltime, LATIME_TOOLBAR, 1 + INALTIMEA_BAREI_DE_ITEME + (i + 1) * TOOLS_Inaltime);

        setcolor(WHITE);
        rectangle(0, 1 + INALTIMEA_BAREI_DE_ITEME + i * TOOLS_Inaltime, LATIME_TOOLBAR, 1 + INALTIMEA_BAREI_DE_ITEME + (i + 1) * TOOLS_Inaltime);

        setbkcolor(DARKGRAY);
        setcolor(BLACK);
        settextstyle(SANS_SERIF_FONT, HORIZ_DIR, 2);

        char label[20];
        strcpy(label, Tool_Labels[i]);

        int textWidth = textwidth(label);
        int textHeight = textheight(label);
        int textX = (LATIME_TOOLBAR - textWidth) / 2;
        int textY = 1 + INALTIMEA_BAREI_DE_ITEME + i * TOOLS_Inaltime + (TOOLS_Inaltime - textHeight) / 2;

        outtextxy(textX, textY, label);
    }
}

int lastHoveredTool = -1;
int lastHoveredItem = -1;
void hovering_on_menu(int x, int y)
{

    int currentTool = getToolIndex(x, y);
    int currentItem = getItemIndex(x, y);

    if (currentTool != lastHoveredTool)
    {
        if (lastHoveredTool != -1)
        {
            DeseneazaTool(lastHoveredTool);
        }

        if (currentTool != -1)
        {
            int i = currentTool;
            int TOOLS_Inaltime = (INALTIME_ECRAN - INALTIMEA_BAREI_DE_ITEME) / NR_TOOLS;
            setbkcolor(BLACK);
            setfillstyle(SOLID_FILL, COLOR(32, 32, 32));
            bar(0, 1 + INALTIMEA_BAREI_DE_ITEME + i * TOOLS_Inaltime, LATIME_TOOLBAR, 1 + INALTIMEA_BAREI_DE_ITEME + (i + 1) * TOOLS_Inaltime);

            setcolor(WHITE);
            rectangle(0, 1 + INALTIMEA_BAREI_DE_ITEME + i * TOOLS_Inaltime, LATIME_TOOLBAR, 1 + INALTIMEA_BAREI_DE_ITEME + (i + 1) * TOOLS_Inaltime);

            setbkcolor(COLOR(32, 32, 32));
            setcolor(YELLOW);
            settextstyle(SANS_SERIF_FONT, HORIZ_DIR, 2);

            char label[20];
            strcpy(label, Tool_Labels[i]);

            int textWidth = textwidth(label);
            int textHeight = textheight(label);
            int textX = (LATIME_TOOLBAR - textWidth) / 2;
            int textY = 1 + INALTIMEA_BAREI_DE_ITEME + i * TOOLS_Inaltime + (TOOLS_Inaltime - textHeight) / 2;

            outtextxy(textX, textY, label);
        }

        lastHoveredTool = currentTool;
    }

    if (currentItem != lastHoveredItem)
    {
        if (lastHoveredItem != -1)
        {
            DeseneazaItem(lastHoveredItem);
        }

        if (currentItem != -1)
        {
            int Lungimea_Barei_Iteme = LATIME_ECRAN / NR_ITEME;
            int i = currentItem;
            setfillstyle(SOLID_FILL, COLOR(32, 32, 32));
            bar(i * Lungimea_Barei_Iteme, 0, (i + 1) * Lungimea_Barei_Iteme, INALTIMEA_BAREI_DE_ITEME);

            setcolor(BLACK);
            rectangle(i * Lungimea_Barei_Iteme, 0, (i + 1) * Lungimea_Barei_Iteme, INALTIMEA_BAREI_DE_ITEME);
            setcolor(COLOR(255, 255, 51));
            int new_x = i * Lungimea_Barei_Iteme + Lungimea_Barei_Iteme / 2;
            int new_y = INALTIMEA_BAREI_DE_ITEME / 2;
            int index = i;

            for (int j = 0; j < figuri[index].nr_bucati; ++j) {
                char type = figuri[index].tip_bucata[j];
                int a = figuri[index].marire * figuri[index].bucati[j][0];
                int b = figuri[index].marire * figuri[index].bucati[j][1];
                int c = figuri[index].marire * figuri[index].bucati[j][2];
                int d = figuri[index].marire * figuri[index].bucati[j][3];

                if (type == 'L') {
                    line(new_x + a, new_y + b, new_x + c, new_y + d);
                } else if (type == 'O') {
                    ellipse(new_x + a, new_y + b, 0, 360, c, d);
                } else if (type == 'R') {
                    rectangle(new_x + a, new_y + b, new_x + c, new_y + d);
                } else if (type == 'A') {
                    arc(new_x + a, new_y + b, 270, 90, c + d + 0.5);
                }
            }
        }
        lastHoveredItem = currentItem;
    }
}
void clear_autosave()
{
    const string AUTOSAVE_FILE = "autosave.txt";
    ofstream fout(AUTOSAVE_FILE, ios::trunc);
    if (!fout.is_open())
    {
        cout << "Erroar: Fisierul 'autosave' nu s-a putut deschide" << endl;
        return;
    }
    fout.close();
    cout << "Fisierul 'autosave' a fost golit" << endl;
}


void clear_start_menu_file()
{
    remove(START_MENU_IF_FILE);
}

bool has_start_menu_been_shown()
{
    ifstream menu_IF_file(START_MENU_IF_FILE);
    return menu_IF_file.is_open();
}

void mark_start_menu_as_shown()
{
    ofstream menu_IF_file(START_MENU_IF_FILE);
    if (menu_IF_file.is_open())
    {
        menu_IF_file << "orice";
        menu_IF_file.close();
    } else {
        cout << "Eroare la crearea fisierului" << endl;
    }
}

void punct_de_pornire_al_alpicatiei() {
    if (!has_start_menu_been_shown()) {
        create_start_menu();
        mark_start_menu_as_shown();
    }
    main_application_loop();
}
void create_start_menu() {

    cleardevice();
    setbkcolor(COLOR(30, 30, 30));
    setfillstyle(SOLID_FILL, COLOR(30, 30, 30));
    bar(0, 0, getmaxx(), getmaxy());

    /// Title: "Electron"
    settextstyle(BOLD_FONT, HORIZ_DIR, 5);
    setcolor(COLOR(255, 255, 255));
    char title[] = "Electron";
    int title_x = (getmaxx() - textwidth(title)) / 2;
    int title_y = 50;
    outtextxy(title_x, title_y, title);

    const int LATIME_BUTON = 380;
    const int INALTIME_BUTON = 50;
    const int SPATIU_LIBER_INTRE_BUTOANE = 30;

    char buttons[][20] = { "New Project", "Last Project", "Import Project", "Exit" };
    int num_buttons = sizeof(buttons) / sizeof(buttons[0]);

    int start_x = (getmaxx() - LATIME_BUTON) / 2;
    int start_y = (getmaxy() - (num_buttons * INALTIME_BUTON + (num_buttons - 1) * SPATIU_LIBER_INTRE_BUTOANE)) / 2;

    for (int i = 0; i < num_buttons; ++i)
    {
        int x1 = start_x;
        int y1 = start_y + i * (INALTIME_BUTON + SPATIU_LIBER_INTRE_BUTOANE);
        int x2 = x1 + LATIME_BUTON;
        int y2 = y1 + INALTIME_BUTON;

        setfillstyle(SOLID_FILL, COLOR(50, 50, 50));
        bar(x1, y1, x2, y2);
        setcolor(COLOR(255, 255, 255));
        rectangle(x1, y1, x2, y2);
        setbkcolor(COLOR(50, 50, 50));
        int text_x = x1 + (LATIME_BUTON - textwidth(buttons[i])) / 2;
        int text_y = y1 + (INALTIME_BUTON - textheight(buttons[i])) / 2;
        outtextxy(text_x, text_y, buttons[i]);
    }

    settextstyle(DEFAULT_FONT, HORIZ_DIR, 2);
    setcolor(COLOR(200, 200, 200));
    setbkcolor(COLOR(30, 30, 30));
    char name1[] = "Cosmin Ciobanu";
    char name2[] = "Chiriac Bogdan";
    int name1_x = getmaxx() - textwidth(name1) - 10;
    int name1_y = getmaxy() - textheight(name1) * 2 - 10;
    int name2_x = getmaxx() - textwidth(name2) - 10;
    int name2_y = getmaxy() - textheight(name2) - 5;

    outtextxy(name1_x, name1_y, name1);
    outtextxy(name2_x, name2_y, name2);

    char text1[] = "Universitatea \"Alexandru Ioan Cuza\", Facultatea de Informatica, Iasi";
    char text2[] = "Proiect realizat sub coordonarea domnului profesor Bogdan Patrut";
    int text1_x = 10; // Left edge padding
    int text1_y = getmaxy() - textheight(text2) * 2 - 10; // Bottom padding
    int text2_x = 10;
    int text2_y = getmaxy() - textheight(text2) - 5;

    outtextxy(text1_x, text1_y, text1);
    outtextxy(text2_x, text2_y, text2);

    while (true)
    {
        if (ismouseclick(WM_LBUTTONDOWN))
        {
            int x = mousex();
            int y = mousey();
            clearmouseclick(WM_LBUTTONDOWN);

            for (int i = 0; i < num_buttons; ++i)
            {
                int x1 = start_x;
                int y1 = start_y + i * (INALTIME_BUTON + SPATIU_LIBER_INTRE_BUTOANE);
                int x2 = x1 + LATIME_BUTON;
                int y2 = y1 + INALTIME_BUTON;

                if (x >= x1 && x <= x2 && y >= y1 && y <= y2)
                {
                    switch (i)
                    {
                        case 0: /// New Project
                            clear_autosave();
                            return;
                        case 1: /// Last Project
                            load_passive_save();
                            return;
                        case 2: /// Import Project
                            clear_autosave();
                            import_circuit();
                            set_project_directory();
                            passive_save();
                            return;
                        case 3: ///Exit
                            closegraph();
                            exit(0);
                            return;
                    }
                }
            }
        }
    }
}


void main_application_loop()
{
    cleardevice();
    DeseneazaBaraDeIteme();
    DeseneazaBaraDeTools();
    redraw_page();
}

int main()
{
    citire_figuri();
    initwindow(LATIME_ECRAN, INALTIME_ECRAN, "Electron");
    ///drawing the Toolbar and ItemMenu
    punct_de_pornire_al_alpicatiei();
    load_passive_save();
    DeseneazaBaraDeIteme();
    DeseneazaBaraDeTools();
    redraw();
    setbkcolor(FUNDAL);

    /// Initializing the main() variables
    int Index_Rename = -1;
    int Tool_Selectat = -1;
    int Item_Selectat = -1;
    int Tool_Hovered= -1;
    int Item_Hovered= -1; // no item has yet been selected
    bool running = true;
    bool hover = false;
    while (running)
    {
        if(!ismouseclick(WM_LBUTTONDOWN) && !ismouseclick(WM_RBUTTONDOWN))
        {
            int x = mousex();
            int y = mousey();
            if(((x<=LATIME_TOOLBAR && y>INALTIMEA_BAREI_DE_ITEME) || (y <= INALTIMEA_BAREI_DE_ITEME)))
                    {hovering_on_menu(x, y); lastHoveredTool=getToolIndex(x, y); lastHoveredItem=getItemIndex(x, y); hover=true; continue;}
            else if(hover==true)
                {hover=false; DeseneazaItem(lastHoveredItem); DeseneazaTool(lastHoveredTool); lastHoveredItem=-1; lastHoveredTool=-1;}

        }
        if (ismouseclick(WM_LBUTTONDOWN))
        {
            int x = mousex();
            int y = mousey();
            clearmouseclick(WM_LBUTTONDOWN);

            if (x<=LATIME_TOOLBAR && y>INALTIMEA_BAREI_DE_ITEME)
            {
                Tool_Selectat=getToolIndex(x, y);
                cout << "Numarul Uneltei selectate " << Tool_Selectat << endl;
                Tool_Cases(Tool_Selectat);

            }
            else if (y <= INALTIMEA_BAREI_DE_ITEME)
            {

                /// Click in intem bar
                Item_Selectat = getItemIndex(x, y);
                plasare_piesa_noua(Item_Selectat);
                passive_save();
                cout << "Numarul itemului selectat " << Item_Selectat << endl;

                Tool_Selectat=NR_ITEME+1;
            }

        }
        if (ismouseclick(WM_RBUTTONDOWN))
        {
            int x=mousex();
            int y=mousey();
            clearmouseclick(WM_RBUTTONDOWN);
            Index_Rename=index_figura_apasata(x,y);
            if (Index_Rename!=-1)
                citire_modal(piese[Index_Rename],Index_Rename);
            Tool_Selectat=NR_ITEME;
            passive_save();
        }
        while(kbhit())
        {
            char key = getch();

            switch (key)
            {
            case 'r':
                if (Tool_Selectat == NR_ITEME + 1)
                {
                    plasare_piesa_noua(Item_Selectat);
                    passive_save();
                }
                else if (Tool_Selectat == NR_ITEME)
                {
                    citire_modal(piese[Index_Rename], Index_Rename);
                }
                else
                {
                    Tool_Cases(Tool_Selectat);
                }
            break;

            case 'g': ///Daca ecranul ingheata
            reopen_application();
            break;

            case 27: ///Inchidem ecranul si la urmatorul run al programului se deschide meniul de start
                cout << "Pressed Escape" << endl;
                running = false;
                break;

            case 8: ///Se intoarce la meniul de start
                cout << "Pressed Backspace" << endl;
                remove(START_MENU_IF_FILE);
                reopen_application();
                break;

            default:
                break;
        }
    }

    }
    remove(START_MENU_IF_FILE);
    closegraph();
    return 0;
}
