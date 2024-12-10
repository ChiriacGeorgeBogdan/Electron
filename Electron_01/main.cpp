#include <graphics.h>
#include <winbgim.h>
#include <iostream>
#include <cstdio>
#include <fstream>

using namespace std;

/// Constants
const int LATIME_ECRAN = 1600;
const int INALTIME_ECRAN = 800;

    ///Item barul este aflat langa marginea de sus a ecranului y=0
const int INALTIMEA_BAREI_DE_ITEME = 50;   /// inaltimea Item BAR
    ///Barul de tooluri este situat langa marginea stanga a ecranului (x=0) si sub barul de iteme
const int LATIME_TOOLBAR = 150;
const int NR_TOOLS=7;
const int NR_ITEME=13;

const int MAX_PIESE = 100;  /// Nr maxim de piese pe care le putem desena

const char Tool_Labels[NR_TOOLS][20]={"Clear Mouse", "Rotate" ,"Move","Erase Shape", "Make Connection", "Erase All", "Erase Connection"};
const char Item_Labels[NR_ITEME][15]={"Shape 1"};


//fisierele si structura aferenta figurilor
const char fisiere[20][15]={"AMPLOP.PS","BATERIE.PS","CONDENS.PS","DIODA.PS","NOD.PS","POLARIZ.PS","REZIST.PS","SERVOMOT.PS","SINU.PS","STOP.PS","TRANZNPN.PS","TRANZPNP.PS","ZENNER.PS"};
struct figura{
    char nume[50];
    int nr_intrari;
    double intrari[5][2];
    char descriere[100];
    int nr_bucati;
    double bucati[20][4];
    char tip_bucata[20];
    int marire;
}figuri[50];

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
struct piesa{
    int x1,y1,x2,y2;//colt stanga sus si colt dreapta jos
    int x,y,index;
} piese[MAX_PIESE];
int nrPiese = -1; /// Nr de piese aflate pe ecran

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
    }

    /// Numirea piselor/itemelor
    setbkcolor(LIGHTGRAY);
    settextstyle(DEFAULT_FONT, HORIZ_DIR, 1);
    setcolor(BLACK);
    for (int i = 0; i < NR_ITEME; ++i) {
        outtextxy(i * (LATIME_ECRAN / NR_ITEME) + 10, 15, figuri[i].nume);
    }
}
/// Deseneaza Bara de Tooluri
void DeseneazaBaraDeTools()
{
    int TOOLS_Inaltime = (INALTIME_ECRAN-INALTIMEA_BAREI_DE_ITEME)/NR_TOOLS;
    for ( int i=0; i< NR_TOOLS; ++i)
    {
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
        /*int textWidth = textwidth(label);
        int textHeight = textheight(label);
        int textX = (LATIME_TOOLBAR - textWidth) / 2; // Center horizontally
        int textY = INALTIMEA_BAREI_DE_ITEME + i * TOOLS_Inaltime + (TOOLS_Inaltime - textHeight) / 2;*/
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
void desenare_piesa (int x, int y, int index)
{
    setbkcolor(BLACK);
    setcolor(COLOR(255,255,51));
    for (int i=0; i<figuri[index].nr_bucati; ++i)
    {
        char type=figuri[index].tip_bucata[i];
        int a=figuri[index].marire*figuri[index].bucati[i][0];
        int b=figuri[index].marire*figuri[index].bucati[i][1];
        int c=figuri[index].marire*figuri[index].bucati[i][2];
        int d=figuri[index].marire*figuri[index].bucati[i][3];
        if (type=='L') {
            line(x+a,y+b,x+c,y+d);
        }
        else if (type=='O') {
            ellipse(x+a,y+b,0,360,c,d);
        }
        else if (type=='R') {
            rectangle(x+a,y+b,x+c,y+d);
        }
    }
}
void incadrare (piesa& piesaNoua, int x, int y, int index)
{
    piesaNoua.x1=piesaNoua.y1=9999;
    piesaNoua.x2=piesaNoua.y2=0;
    for (int i=0; i<figuri[index].nr_bucati; ++i)
    {
        char type=figuri[index].tip_bucata[i];
        int a=figuri[index].marire*figuri[index].bucati[i][0];
        int b=figuri[index].marire*figuri[index].bucati[i][1];
        int c=figuri[index].marire*figuri[index].bucati[i][2];
        int d=figuri[index].marire*figuri[index].bucati[i][3];
        if (type=='L') {
            piesaNoua.x1=min(piesaNoua.x1,x+a);
            piesaNoua.y1=min(piesaNoua.y1,y+b);
            piesaNoua.x2=max(piesaNoua.x2,x+c);
            piesaNoua.y2=max(piesaNoua.y2,y+d);
        }
        else if (type=='O') {
            piesaNoua.x1=min(piesaNoua.x1,x+a-c);
            piesaNoua.y1=min(piesaNoua.y1,y+b-d);
            piesaNoua.x2=max(piesaNoua.x2,x+a+c);
            piesaNoua.y2=max(piesaNoua.y2,y+b+d);
        }
        else if (type=='R') {
            piesaNoua.x1=min(piesaNoua.x1,x+a);
            piesaNoua.y1=min(piesaNoua.y1,y+b);
            piesaNoua.x2=max(piesaNoua.x2,x+c);
            piesaNoua.y2=max(piesaNoua.y2,y+d);
        }
    }
    piesaNoua.x=x;
    piesaNoua.y=y;
    piesaNoua.index=index;
}
/// Functie care detecta ce Item a fost apasat
int getItemIndex(int x, int y) {
    if (y > INALTIMEA_BAREI_DE_ITEME) return -1;
    return x / (LATIME_ECRAN / NR_ITEME);
}


void redraw()
{
    setbkcolor(BLACK);
    DeseneazaBaraDeIteme();
    DeseneazaBaraDeTools();
    for (int i=0; i<=nrPiese; ++i)
        desenare_piesa(piese[i].x,piese[i].y,piese[i].index);
}
void animareChenar (int state)
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

void stergePiesa (int index)
{
    piese[index]=piese[nrPiese--];
}
int main() {
    citire_figuri();
    initwindow(LATIME_ECRAN, INALTIME_ECRAN, "Electron");
    ///drawing the Toolbar and ItemMenu
    DeseneazaBaraDeIteme();
    DeseneazaBaraDeTools();

    /// Initializing the main() variables
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
                int toolSelectat=(y-INALTIMEA_BAREI_DE_ITEME)/((INALTIME_ECRAN-INALTIMEA_BAREI_DE_ITEME)/NR_TOOLS);
                int state=0;
                while (!ismouseclick(WM_LBUTTONDOWN))
                {
                    animareChenar(state);
                    delay(500);
                    state=1-state;
                    cleardevice();
                    redraw();
                }
                if (toolSelectat==3) //stergere
                {
                    x=mousex();
                    y=mousey();
                    piesa pseudopiesa;
                    pseudopiesa.x1=pseudopiesa.x2=x;
                    pseudopiesa.y1=pseudopiesa.y2=y;
                    for (int i=0; i<=nrPiese;++i)
                        if (seIntersecteaza(piese[i],pseudopiesa))
                            stergePiesa(i);
                    cleardevice();
                    redraw();
                }
            }
            if (y <= INALTIMEA_BAREI_DE_ITEME)
            {
                /// Click in intem bar
                Item_Selectat = getItemIndex(x, y);
                cout << "Numarul itemului selectat " << Item_Selectat << endl;
            }
            else if (Item_Selectat != -1)
            {
                /// Click inafara item barului
                piesa piesaNoua;
                incadrare(piesaNoua,x,y,Item_Selectat);
                if (sePoateDesena(piesaNoua,x,y,Item_Selectat))
                {
                    desenare_piesa(x, y, Item_Selectat);
                    piese[++nrPiese]=piesaNoua;
                }
                Item_Selectat=-1;
            }
        }


    }

    closegraph();
    return 0;
}
