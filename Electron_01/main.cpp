#include <graphics.h>
#include <winbgim.h>
#include <iostream>
#include <cstdio>
#include <fstream>
#include <vector>


#define FUNDAL 0

using namespace std;
void redraw();
/// Constants
const int LATIME_ECRAN = 1800;
const int INALTIME_ECRAN = 950;

///Item barul este aflat langa marginea de sus a ecranului y=0
const int INALTIMEA_BAREI_DE_ITEME = 50;   /// inaltimea Item BAR
///Barul de tooluri este situat langa marginea stanga a ecranului (x=0) si sub barul de iteme
const int LATIME_TOOLBAR = 150;
const int NR_TOOLS=9;
const int NR_ITEME=13;
const int REFRESH_RATE=1000.0/10;

const int MAX_PIESE = 100;  /// Nr maxim de piese pe care le putem desena
const int MAX_INTRARI = 3;

const char Tool_Labels[NR_TOOLS][20]= {"Make Connection", "Rotate","Move","Increase","Decrease","Erase Shape", "Erase All", "Undo", "Redo"};
const char Item_Labels[NR_ITEME][15]= {"Shape 1"};

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

const int CULOARE_LEGATURI=GREEN;
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
pair<vector<int>, vector<piesa> >modificari[1000];
vector<vector<int> >legaturi_modificate[1000];
int p=-1,q=-1;


int nrPiese = -1; /// Nr de piese aflate pe ecran

void roteste (float x, float y, float & x_nou, float & y_nou)
{
    x_nou = y;
    y_nou = -x;
}
void myRectangle(int orientare, int index, int i, int &a, int &b, int &c, int &d)
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

    a = figuri[index].marire * x1_nou;
    b = figuri[index].marire * y1_nou;
    c = figuri[index].marire * x2_nou;
    d = figuri[index].marire * y2_nou;
    if(a>c) swap(a,c);
    if(b>d) swap(b,d);
}
void myLine(int orientare, int index, int i, int &a, int &b, int &c, int &d)
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

    a = figuri[index].marire * x1_nou;
    b = figuri[index].marire * y1_nou;
    c = figuri[index].marire * x2_nou;
    d = figuri[index].marire * y2_nou;
}


void myEllipse(int orientare, int index, int i, int &a, int &b, int &c, int &d)
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

    a = figuri[index].marire * x1_nou;
    b = figuri[index].marire * y1_nou;
    c = figuri[index].marire * rx;
    d = figuri[index].marire * ry;
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
        }
    }
}

/// Deseneaza Bara de Tooluri
void DeseneazaBaraDeTools()
{
    int TOOLS_Inaltime = (INALTIME_ECRAN-INALTIMEA_BAREI_DE_ITEME)/NR_TOOLS;
    for ( int i=0; i< NR_TOOLS; ++i)
    {
        setbkcolor(BLACK);
        setfillstyle(SOLID_FILL, DARKGRAY);
        bar(0, 1+INALTIMEA_BAREI_DE_ITEME+i*TOOLS_Inaltime, LATIME_TOOLBAR,  1+INALTIMEA_BAREI_DE_ITEME+(i+1)*TOOLS_Inaltime);

        setcolor(WHITE);
        rectangle(0, 1+INALTIMEA_BAREI_DE_ITEME+i*TOOLS_Inaltime, LATIME_TOOLBAR,  1+INALTIMEA_BAREI_DE_ITEME+(i+1)*TOOLS_Inaltime);

        /// Numirea Toolurilor
        //Tool_Labels[NR_TOOLS];
        setbkcolor(DARKGRAY);
        settextstyle(DEFAULT_FONT, HORIZ_DIR, 1);
        setcolor(BLACK);
        char label[20];
        strcpy(label, Tool_Labels[i]);
        /*
        int textWidth = textwidth(label);
        int textHeight = textheight(label);
        int textX = (LATIME_TOOLBAR - textWidth) / 2; // Center horizontally
        int textY = INALTIMEA_BAREI_DE_ITEME + i * TOOLS_Inaltime + (TOOLS_Inaltime - textHeight) / 2;
        */
        outtextxy(20, 10+INALTIMEA_BAREI_DE_ITEME+i*TOOLS_Inaltime, label);
    }


    //for(int i=0; i < )

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
        //verificam daca se intersecteaza cu alte piese
        if (seIntersecteaza(piesaNoua,piese[i]))
        {
            cout<<"Se suprapune cu piesa ("<<i<<")\n";
            return false;
        }
    }

    for (int i=0; i<figuri[index].nr_bucati; ++i)
    {
        char type=figuri[index].tip_bucata[i];
        int a=figuri[index].marire*figuri[index].bucati[i][0];
        int b=figuri[index].marire*figuri[index].bucati[i][1];
        int c=figuri[index].marire*figuri[index].bucati[i][2];
        int d=figuri[index].marire*figuri[index].bucati[i][3];
        if (type=='L' || type=='R')
        {
            if (!(estePeTabla(x+a,y+b) && estePeTabla(x+c,y+d)))
                return false;
        }
        else if (type=='O')
        {
            if (!(estePeTabla(x+a-c,y+b-d) && estePeTabla(x+a+c,y+b-d) && estePeTabla(x+a-c,y+b-d) && estePeTabla(x+a+c,y+b+d)))
                return false;
        }
    }
    return true;
}
void golire_ecran ()
{
    /*
    setcolor(BLACK);
    setbkcolor(BLACK);
    setfillstyle(SOLID_FILL,BLACK);
    floodfill(0,0,RGB(102,495,203));*/
  //  rectangle(0,0,LATIME_ECRAN, INALTIME_ECRAN);
    setbkcolor(BLACK);
    setfillstyle(SOLID_FILL,BLACK);
    bar(LATIME_TOOLBAR+1,INALTIMEA_BAREI_DE_ITEME+1,LATIME_ECRAN,INALTIME_ECRAN);
}

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
/*
        int a=figuri[index].marire*figuri[index].bucati[i][0];
        int b=figuri[index].marire*figuri[index].bucati[i][1];
        int c=figuri[index].marire*figuri[index].bucati[i][2];
        int d=figuri[index].marire*figuri[index].bucati[i][3];
*/
        int a=0; int b=0; int c=0; int d=0;
        if (type=='L')
        {
            myLine(orientare, index, i, a, b, c, d);
            line(x+a,y+b,x+c,y+d);
        }
        else if (type=='O')
        {
            myEllipse(orientare, index, i, a, b, c, d);
            ellipse(x+a,y+b,0,360,c,d);
        }
        else if (type=='R')
        {
            myRectangle(orientare, index, i, a, b, c, d);
            rectangle(x+a,y+b,x+c,y+d);
        }
    }
    //desenare_caracteristici(P);
}
void myIntrari(int orientare, int index, int i, int &a, int &b)
{
    float x1 = figuri[index].intrari[i][0];
    float y1 = figuri[index].intrari[i][1];

    float x1_nou = x1, y1_nou = y1;

    for (int j = 0; j < orientare; ++j)
    {
        roteste(x1_nou, y1_nou, x1_nou, y1_nou);
    }

    a = figuri[index].marire * x1_nou;
    b = figuri[index].marire * y1_nou;
}
void calcul_intrari(piesa &piesaNoua)
{
    int index=piesaNoua.index;
    int orientare=piesaNoua.orientare;
    for(int j=0; j<figuri[index].nr_intrari; ++j)
    {
        int a=0; int b=0;
        myIntrari(orientare, index, j, a, b);
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
    }
    piesaNoua.x=x;
    piesaNoua.y=y;
    piesaNoua.index=index;
    calcul_intrari(piesaNoua);
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
            myLine(orientare, index, i, a, b, c, d);
            piesaVeche.x1 = min(min(piesaVeche.x1, x + a), x + c);
            piesaVeche.y1 = min(min(piesaVeche.y1, y + b), y + d);
            piesaVeche.x2 = max(max(piesaVeche.x2, x + a), x + c);
            piesaVeche.y2 = max(max(piesaVeche.y2, y + b), y + d);
        }
        else if (type=='O')
        {
            myEllipse(orientare, index, i, a, b, c, d);
            piesaVeche.x1=min(piesaVeche.x1,x+a-c);
            piesaVeche.y1=min(piesaVeche.y1,y+b-d);
            piesaVeche.x2=max(piesaVeche.x2,x+a+c);
            piesaVeche.y2=max(piesaVeche.y2,y+b+d);
        }
        else if (type=='R')
        {
            myRectangle(orientare, index, i, a, b, c, d);
            piesaVeche.x1=min(piesaVeche.x1,x+a);
            piesaVeche.y1=min(piesaVeche.y1,y+b);
            piesaVeche.x2=max(piesaVeche.x2,x+c);
            piesaVeche.y2=max(piesaVeche.y2,y+d);
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

const int Raza_Intrare=5;
const int Imprecizie=25;

void desenare_intrari(int CULOARE)
{
    setcolor(CULOARE);
    for (int i = 0; i <= nrPiese; ++i)
        for (int j = 0; j < figuri[piese[i].index].nr_intrari; ++j)
            ellipse(piese[i].intrari[j].x, piese[i].intrari[j].y, 0, 360, Raza_Intrare, Raza_Intrare);
}

void stergere_intrari()
{
    desenare_intrari(FUNDAL);
    redraw();
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
            golire_ecran();
            redraw();
            desenare_intrari(WHITE);
            drawLine(piese[Index_01].intrari[Index_intrare_01].x,piese[Index_01].intrari[Index_intrare_01].y,x,y,CULOARE_LEGATURI);
            delay(REFRESH_RATE);
        }
        int previous_x=-1, previous_y=-1;
      //  if (ismouseclick(WM_LBUTTONDOWN))
        {

            int x = mousex();
            int y = mousey();
            clearmouseclick(WM_LBUTTONDOWN);


            cout<<"Coordonatele pentru al doilea click: ("<<x<<", "<<y<<")"<<endl;

            identificare_nod(running_desenare_legaturi, Index_02, Index_intrare_02, x, y);

            if (Index_02 != -1 && Index_intrare_02 != -1)
            {
                cout<<"Nodul al doilea a fost identificat la: Index "<<Index_02<< ", Intrare "<<Index_intrare_02<<endl;

                graf[Index_02][Index_01].intrari[Index_intrare_02][Index_intrare_01]=1;
                graf[Index_01][Index_02].intrari[Index_intrare_01][Index_intrare_02]=1;
                modificari[q=++p]={vector<int>{4},vector<piesa>{}};
                legaturi_modificate[p]={{Index_01,Index_02,Index_intrare_01,Index_intrare_02}};

                if(previousIntrare>=0 && previousPiesa>=0)
                {
                    graf[Index_01][previousPiesa].intrari[Index_intrare_01][previousIntrare]=0;
                    cleardevice();
                    redraw();
                }
                else
                    drawLine(piese[Index_01].intrari[Index_intrare_01].x, piese[Index_01].intrari[Index_intrare_01].y,piese[Index_02].intrari[Index_intrare_02].x,piese[Index_02].intrari[Index_intrare_02].y,CULOARE_LEGATURI);
                break;
            }
            else if(Index_02==-1 && Index_intrare_02==-1) ///I.e daca x si y nu se intersercteaza cu o piesa sau o intrare de pe tabla
            {
                int IndexPiesa=4;
                piesa piesaNoua;
                incadrare(piesaNoua,x,y,IndexPiesa);
                if (sePoateDesena(piesaNoua,x,y,IndexPiesa))
                {
                    int CULOARE=COLOR(255,255,51);
                    desenare_piesa(piesaNoua, CULOARE);
                    piese[++nrPiese]=piesaNoua;
                    Index_02=nrPiese; Index_intrare_02=0;
                    graf[Index_01][Index_02].intrari[Index_intrare_01][Index_intrare_02]=1;
                    graf[Index_02][Index_01].intrari[Index_intrare_02][Index_intrare_01]=1;
                    modificari[q=++p]={{4},{}};
                    legaturi_modificate[p]={{Index_01,Index_02,Index_intrare_01,Index_intrare_02}};
                    if(previousIntrare>=0 && previousPiesa>=0)
                    {
                        graf[Index_01][previousPiesa].intrari[Index_intrare_01][previousIntrare]=0;
                        cleardevice();
                        redraw();
                    }
                    else
                        drawLine(piese[Index_01].intrari[Index_intrare_01].x, piese[Index_01].intrari[Index_intrare_01].y,piese[Index_02].intrari[Index_intrare_02].x,piese[Index_02].intrari[Index_intrare_02].y,CULOARE_LEGATURI);

                }
                else
                {
                    cout<<"Niciun nod valid nu a fost gasit pentru al doilea click"<<endl;
                }
            }

        }
        /*else
        {
            int x=mousex();
            int y=mousey();
            drawLine(piese[Index_01].intrari[Index_intrare_01].x, piese[Index_01].intrari[Index_intrare_01].y, x, y, CULOARE_LEGATURI);
            if(previous_x!=-1 && previous_y!=-1)
            {
                drawLine(piese[Index_01].intrari[Index_intrare_01].x, piese[Index_01].intrari[Index_intrare_01].y, previous_x, previous_y, BLACK);
            }
            previous_x=x; previous_y=y;
        }*/
    }
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
void redraw()
{
    setbkcolor(BLACK);
    DeseneazaBaraDeIteme();
    DeseneazaBaraDeTools();
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
void citire_modal (piesa &P)
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
                    golire_ecran();
                    redraw();
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
                    golire_ecran();
                    redraw();
                    modal(P);
                    delay(REFRESH_RATE);
                }
                campActiv=-1;
            }
            else if (inchenar(x,y,chenarValoare))
            {
                int n=strlen(P.valoare);
                campActiv=1;
                    golire_ecran();
                    redraw();
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
                    golire_ecran();
                    redraw();
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
                return;
            }
            if (inchenar(x,y,chenarX) || !inchenar(x,y,chenarModal))
            {
                strcpy(P.nume,copieNume);
                strcpy(P.valoare,copieVal);
                P.unit=copieUnit;
                return;
            }
        }
        setfillstyle(1,BLACK);
        bar(chenarModal.x1,chenarModal.y1,chenarModal.x2,chenarModal.y2);
        modal(P);
        delay(REFRESH_RATE);
    }


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
                    /*
                    adaug intr o noua structura legaturile pe care le sterg
                    */
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
    golire_ecran();
    redraw();
}
void AsteptareSelectie()
{
    int state=0;
    while (!ismouseclick(WM_LBUTTONDOWN))
    {
        AnimareChenar(state);
        //desenare_intrari();
        delay(500);
        state=1-state;
        golire_ecran();
        redraw();
    }
}
void rotire()
{
    AsteptareSelectie();
    int x = mousex();
    int y = mousey();
    clearmouseclick(WM_LBUTTONDOWN);
    int i=index_figura_apasata(x, y);
    desenare_piesa(piese[i], FUNDAL);

    piesa piesaInitiala=piese[i];
    piese[i].orientare=(piese[i].orientare+1)%4;
    modificari[q=++p]={vector<int>{3,i},vector<piesa>{piesaInitiala,piese[i]}};

    //int CULOARE=COLOR(255,255,51);
    incadrare_PiesaModificata(piese[i]);
    golire_ecran();
    redraw();

}
void mutare_piesa ()
{

    AsteptareSelectie();
    int i=cauta_piesa();
    // piesaPeCursor=i;
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
        golire_ecran();
        redraw();
        delay(REFRESH_RATE);
    }
   // piesaPeCursor=-1;
    golire_ecran();
    redraw();
}
void plasare_piesa_noua(int Item_Selectat)
{
    //cream piesa noua
    piese[++nrPiese].index=Item_Selectat;
    piese[nrPiese].orientare=0;
    piese[nrPiese].zoom=1;
    //asteptam plasarea
    while (!ismouseclick(WM_LBUTTONDOWN))
    {
        int x=mousex();
        int y=mousey();
        piese[nrPiese].x=x;
        piese[nrPiese].y=y;
        incadrare(piese[nrPiese],x,y,Item_Selectat);
        golire_ecran();
        redraw();
        delay(REFRESH_RATE);
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

}
void erase_all()
{
    modificari[q=++p]={{6,nrPiese},{}};
    legaturi_modificate[p].resize(0);
    for (int i=0; i<=nrPiese; ++i)
        for (int j=0; j<=nrPiese; ++j)
            for (int e=0; e<MAX_INTRARI; ++e)
                for (int f=0; f<MAX_INTRARI; ++f)
                {
                    if (graf[i][j].intrari[e][f])
                    {
                        legaturi_modificate[p].push_back({i,j,e,f});
                        graf[i][j].intrari[e][f]=0;
                    }
                }
    nrPiese=-1;
    golire_ecran();
    redraw();
}
void undo()
{
    if (p>=0)
    {
        int caz=modificari[p].first[0];
        int poz=modificari[p].first[1];
        piesa piesaVeche;
        if (caz!=4 && caz!=6) piesaVeche=modificari[p].second[0];
        piesa piesaNoua;
        if (caz==2 || caz==3) piesaNoua=modificari[p].second[1];
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
                /*int p1=modificari[q].first[1];
                int p2=modificari[q].first[2];
                int i1=modificari[q].first[3];
                int i2=modificari[q].first[4];*/
                int p1=legaturi_modificate[p][0][0];
                int p2=legaturi_modificate[p][0][1];
                int i1=legaturi_modificate[p][0][2];
                int i2=legaturi_modificate[p][0][3];
                graf[p1][p2].intrari[i1][i2]=0;
                graf[p2][p1].intrari[i2][i1]=0;
                break;
            }
            case 5: //stergere legatura
                break;
            case 6://sterge tot
                nrPiese=poz;
                for (auto e:legaturi_modificate[p]){
                    graf[e[0]][e[1]].intrari[e[2]][e[3]]=1;
                    graf[e[1]][e[0]].intrari[e[3]][e[2]]=1;
                }
                break;
        }
        p--;
    }
}
void redo()
{
    if (p<q)
    {
        p++;
        int caz=modificari[p].first[0];
        int poz=modificari[p].first[1];
        piesa piesaVeche;
        if (caz!=4 && caz!=6) piesaVeche=modificari[p].second[0];
        piesa piesaNoua;
        if (caz==2 || caz==3) piesaNoua=modificari[p].second[1];
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
                /*int p1=modificari[q].first[1];
                int p2=modificari[q].first[2];
                int i1=modificari[q].first[3];
                int i2=modificari[q].first[4];*/
                int p1=legaturi_modificate[p][0][0];
                int p2=legaturi_modificate[p][0][1];
                int i1=legaturi_modificate[p][0][2];
                int i2=legaturi_modificate[p][0][3];
                graf[p1][p2].intrari[i1][i2]=1;
                graf[p2][p1].intrari[i2][i1]=1;
                break;
            }
            case 5: //stergere legatura
                break;
            case 6://sterge tot
                nrPiese=-1;
                for (auto e:legaturi_modificate[p]){
                    graf[e[0]][e[1]].intrari[e[2]][e[3]]=0;
                    graf[e[1]][e[0]].intrari[e[3]][e[2]]=0;
                }
                break;
        }
    }
}
void increase()
{
    while (1)
    {
        if (ismouseclick(WM_LBUTTONDOWN))
        {

            return ;
        }
    }
}
void Tool_Cases(int index)
{
    switch (index)
    {
        case 0:
            desenare_intrari(WHITE);
            trasare_legatura();
            //stergere_intrari();
            //redraw();
            break;
        case 1:
            rotire();
            break;
        case 2:
            mutare_piesa();
            break;
        case 3:
            //maririe
            break;
        case 4:
            //micsorare
            break;
        case 5:
            AsteptareSelectie();
            stergere_piesa();
            break;
        case 6:
            erase_all();
            break;
        case 7:
            undo();
            break;
        case 8:
            redo();
            break;
        default:
            return;
    }
}
//trb terminata functia pt desenare meniu caracteristici, trb terminata functia de desenare la caracteristicile piesei
int main()
{
    citire_figuri();
    initwindow(LATIME_ECRAN, INALTIME_ECRAN, "Electron");
    ///drawing the Toolbar and ItemMenu
    DeseneazaBaraDeIteme();
    DeseneazaBaraDeTools();


    /// Initializing the main() variables
    int Tool_Selectat = -1;
    int Item_Selectat = -1; // no item has yet been selected
    bool running = true;

    while (running)
    {
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
                Tool_Selectat=-1;
            }
            else if (y <= INALTIMEA_BAREI_DE_ITEME)
            {

                /// Click in intem bar
                Item_Selectat = getItemIndex(x, y);
                plasare_piesa_noua(Item_Selectat);

                cout << "Numarul itemului selectat " << Item_Selectat << endl;

                Item_Selectat=-1;
            }

        }
        if (ismouseclick(WM_RBUTTONDOWN))
        {
            int x=mousex();
            int y=mousey();
            clearmouseclick(WM_RBUTTONDOWN);
            int index=index_figura_apasata(x,y);
            if (index!=-1)
                citire_modal(piese[index]);
        }
        delay(REFRESH_RATE);
        golire_ecran();
        redraw();
    }

    closegraph();
    return 0;
}
