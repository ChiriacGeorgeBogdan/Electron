#include <graphics.h>
#include <winbgim.h>
#include <iostream>
#include <cstdio>
#include <fstream>

#define FUNDAL 0

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

const char Tool_Labels[NR_TOOLS][20]= {"Make Connection", "Rotate","Move","Erase Shape", "Clear Mouse", "Erase All", "Erase Connection"};
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
    intrare intrari[5];
    int orientare; /// 0 <=> 0 grade; 1 <=> 90 grade; 2 <=> 180 grade; 3 <=> 270 grade
} piese[MAX_PIESE];

struct grafuri{
    int intrari[5][5];
}graf[MAX_PIESE][MAX_PIESE];

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
void calcul_intratri(piesa &piesaNoua)
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
    calcul_intratri(piesaNoua);
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
    calcul_intratri(piesaVeche);
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
                for(int k=0; k<5; ++k)
                {
                    for(int u=0; u<5; ++u)
                        if(graf[i][j].intrari[k][u]==1)
                            drawLine(piese[i].intrari[k].x, piese[i].intrari[k].y, piese[j].intrari[u].x, piese[j].intrari[u].y , RED);
                }
        }
    }
}

void redraw()
{
    setbkcolor(BLACK);
    DeseneazaBaraDeIteme();
    DeseneazaBaraDeTools();
    desenare_legaturi();
    for (int i=0; i<=nrPiese; ++i)
        {int CULOARE=COLOR(255,255,51); desenare_piesa(piese[i], CULOARE);}
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
        int previous_x=-1, previous_y=-1;
        if (ismouseclick(WM_LBUTTONDOWN))
        {

            int x = mousex();
            int y = mousey();
            clearmouseclick(WM_LBUTTONDOWN);

            cout<<"Coordonatele pentru al doilea click: ("<<x<<", "<<y<<")"<<endl;

            identificare_nod(running_desenare_legaturi, Index_02, Index_intrare_02, x, y);

            if (Index_02 != -1 && Index_intrare_02 != -1)
            {
                cout<<"Nodul al doilea a fost identificat la: Index "<<Index_02<< ", Intrare "<<Index_intrare_02<<endl;

                drawLine(piese[Index_01].intrari[Index_intrare_01].x, piese[Index_01].intrari[Index_intrare_01].y,piese[Index_02].intrari[Index_intrare_02].x,piese[Index_02].intrari[Index_intrare_02].y,RED);
                graf[Index_01][Index_02].intrari[Index_intrare_01][Index_intrare_02]=1;
                break;
            }
            else
            {
                cout<<"Niciun nod valid nu a fost gasit pentru al doilea click"<<endl;
            }
        }
        /*else
        {
            int x=mousex();
            int y=mousey();
            drawLine(piese[Index_01].intrari[Index_intrare_01].x, piese[Index_01].intrari[Index_intrare_01].y, x, y, RED);
            if(previous_x!=-1 && previous_y!=-1)
            {
                drawLine(piese[Index_01].intrari[Index_intrare_01].x, piese[Index_01].intrari[Index_intrare_01].y, previous_x, previous_y, BLACK);
            }
            previous_x=x; previous_y=y;
        }*/
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
        for (int j=0; j<=nrPiese; ++j)
            for (int e=0; e<3; ++e)
                for (int f=0; f<3; ++f)
                {
                    graf[i][j].intrari[e][f]=graf[nrPiese][j].intrari[e][f];
                    graf[nrPiese][j].intrari[e][f]=0;
                    graf[j][i].intrari[e][f]=graf[j][nrPiese].intrari[e][f];
                    graf[j][nrPiese].intrari[e][f]=0;

                }
        piese[i]=piese[nrPiese--];
        cout<<"Piesa stearsa este"<<i<<"\n";
    }
    cleardevice();
    redraw();
}
void AsteptareSelectie ()
{
    int state=0;
    while (!ismouseclick(WM_LBUTTONDOWN))
    {
        AnimareChenar(state);
        //desenare_intrari();
        delay(500);
        state=1-state;
        cleardevice();
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
    piese[i].orientare=(piese[i].orientare+1)%4;
    int CULOARE=COLOR(255,255,51);
    incadrare_PiesaModificata(piese[i]);
    cleardevice();
    redraw();

}
void mutare_piesa ()
{

    AsteptareSelectie();
    int i=cauta_piesa();
    clearmouseclick(WM_LBUTTONDOWN);
    while (true && i!=-1)
    {
        if (ismouseclick(WM_LBUTTONDOWN))
        {
            int x=mousex();
            int y=mousey();
            clearmouseclick(WM_LBUTTONDOWN);
            piesa copiePiesa=piese[i];
            incadrare(copiePiesa,x,y,copiePiesa.index);
            if (sePoateDesena(copiePiesa,x,y,copiePiesa.index))
            {
                piese[i]=copiePiesa;
            }
            break;
        }
    }
    cleardevice();
    redraw();
}
void Tool_Cases(int index)
{
    switch (index)
    {
        case 0:
            desenare_intrari(WHITE);
            trasare_legatura();
            stergere_intrari();
            //redraw();
            break;
        case 1:
            rotire();
            break;
        case 2:
            mutare_piesa();
            break;
        case 3:
            AsteptareSelectie();
            stergere_piesa();
            break;
        default:
            return;
    }
}

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
                cout << "Numarul itemului selectat " << Item_Selectat << endl;
            }
            else if (Item_Selectat != -1)
            {
                /// Click inafara item barului
                piesa piesaNoua;
                incadrare(piesaNoua,x,y,Item_Selectat);
                if (sePoateDesena(piesaNoua,x,y,Item_Selectat))
                {
                    int CULOARE=COLOR(255,255,51);
                    desenare_piesa(piesaNoua, CULOARE);
                    piese[++nrPiese]=piesaNoua;
                }
                Item_Selectat=-1;
            }
        }

    }

    closegraph();
    return 0;
}
