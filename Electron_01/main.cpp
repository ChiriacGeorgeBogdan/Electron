#include <graphics.h>
#include <winbgim.h>
#include <iostream>
#include <cstdio>

using namespace std;

/// Constants
const int LUNGIME_ECRAN = 1600;
const int INALTIME_ECRAN = 800;

    ///Item barul este aflat langa marginea de sus a ecranului y=0
const int INALTIMEA_BAREI_DE_ITEME = 50;   /// inaltimea Item BAR
const int NR_ITEME = 12; /// Nr de piese/iteme diferite
    ///Barul de tooluri este situat langa marginea stanga a ecranului (x=0) si sub barul de iteme
const int LUNGIMEA_BAREI_DE_TOOLS = 150;
const int NR_TOOLS=7;

const int MAX_PIESE = 100;  /// Nr maxim de piese pe care le putem desena

const char Tool_Labels[NR_TOOLS][20]={"Clear Mouse", "Rotate" ,"Move","Erase Shape", "Make Connection", "Erase All", "Erase Connection"};
const char Item_Labels[NR_ITEME][15]={"Shape 1"};

/// Un struct pentru piese
struct Shape {
    int x, y, width, height, color;
};

/// Memoreaza toate piesele aflate pe ecran
Shape piese[MAX_PIESE];
int NrPiese = 0; /// Nr de piese aflate pe ecran

/// Verificarea suprapunerii
bool SeSuprapun(const Shape& p1, const Shape& p2)
{
    return !(p1.x + p1.width < p2.x || p1.x > p2.x + p2.width || p1.y + p1.height < p2.y || p1.y > p2.y + p2.height);
}

/// Verifica daca se poate desena fara suprapunere
bool SePoateDesena(int x, int y, int width, int height)
{
    /// verifica mai intai suprapunerea cu meniul de iteme
    if (y - height / 2 <= INALTIMEA_BAREI_DE_ITEME)
        return false;
    if( x-width/2 <=LUNGIMEA_BAREI_DE_TOOLS)
        return false;
    /// verifica apoi daca se suprapune cu alte figuri utilizand o alta functie
    Shape newPiesa = {x - width / 2, y - height / 2, width, height};
    for (int i = 0; i < NrPiese; ++i)
        if (SeSuprapun(newPiesa, piese[i]))
            return false;

    return true;
}

/// Deseneaza Bara de Iteme/Piese
void DeseneazaBaraDeIteme()
{
    int Lungimea_Barei_Iteme = LUNGIME_ECRAN / NR_ITEME;

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
        char label[10];
        sprintf(label, "Piesa %d", i + 1);
        outtextxy(i * (LUNGIME_ECRAN / NR_ITEME) + 10, 15, label);
    }
}
/// Deseneaza Bara de Tooluri
void DeseneazaBaraDeTools()
{
    int TOOLS_Inaltime = (INALTIME_ECRAN-INALTIMEA_BAREI_DE_ITEME)/NR_TOOLS;
    for ( int i=0; i< NR_TOOLS; ++i)
    {
        setfillstyle(SOLID_FILL, DARKGRAY);
        bar(0, 1+INALTIMEA_BAREI_DE_ITEME+i*TOOLS_Inaltime, LUNGIMEA_BAREI_DE_TOOLS,  1+INALTIMEA_BAREI_DE_ITEME+(i+1)*TOOLS_Inaltime);

        setcolor(WHITE);
        rectangle(0, 1+INALTIMEA_BAREI_DE_ITEME+i*TOOLS_Inaltime, LUNGIMEA_BAREI_DE_TOOLS,  1+INALTIMEA_BAREI_DE_ITEME+(i+1)*TOOLS_Inaltime);

        /// Numirea Toolurilor
        //Tool_Labels[NR_TOOLS];
        setbkcolor(DARKGRAY);
        settextstyle(DEFAULT_FONT, HORIZ_DIR, 1);
        setcolor(BLACK);
        char label[20];
        strcpy(label, Tool_Labels[i]);
        /*int textWidth = textwidth(label);
        int textHeight = textheight(label);
        int textX = (LUNGIMEA_BAREI_DE_TOOLS - textWidth) / 2; // Center horizontally
        int textY = INALTIMEA_BAREI_DE_ITEME + i * TOOLS_Inaltime + (TOOLS_Inaltime - textHeight) / 2;*/
        outtextxy(20, 10+INALTIMEA_BAREI_DE_ITEME+i*TOOLS_Inaltime, label);
    }


    //for(int i=0; i < )

}

/// Functie care deseneaza piesa selectata
void DeseneazaPiesa(int Item_Index, int x, int y)
{
    int width, height, color;
    switch (Item_Index)
    {
        case 0: { // Dreptunghi Rosu
            width = 60; height = 30; color = RED;
            break;
        }
        case 1: { // Dreptunghi Verde
            width = 80; height = 40; color = GREEN;
            break;
        }
        case 2: { // Dreptunghi Albastru
            width = 50; height = 50; color = BLUE;
            break;
        }

        /// mai multe cazuri?

        default:
            cout << "N-a fost definita o pisea pentru acest Item " << Item_Index << endl;
            return;
    }

    if (SePoateDesena(x, y, width, height)) {
        setcolor(color);
        setfillstyle(SOLID_FILL, color);
        bar(x - width / 2, y - height / 2, x + width / 2, y + height / 2);

        piese[NrPiese++] = {x - width / 2, y - height / 2, width, height, color};
    } else {
        cout << "Nu se poate desena caci se suprapun" << endl;
    }
}

/// Functie care detecta ce Item a fost apasat
int getItemIndex(int x, int y) {
    if (y > INALTIMEA_BAREI_DE_ITEME) return -1;
    return x / (LUNGIME_ECRAN / NR_ITEME);
}

int main() {

    initwindow(LUNGIME_ECRAN, INALTIME_ECRAN, "Electron");

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

            if (y <= INALTIMEA_BAREI_DE_ITEME)
            {
                /// Click in intem bar
                Item_Selectat = getItemIndex(x, y);
                cout << "Numarul itemului selectat " << Item_Selectat << endl;
            }
            else if (Item_Selectat != -1)
            {
                /// Click inafara item barului
                DeseneazaPiesa(Item_Selectat, x, y);
            }
        }


    }

    closegraph();
    return 0;
}
