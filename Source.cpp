﻿/*
Spodaj napisan program je namenjen za projektno nalogo pri predmetu Programiranje mikrokrmilnikov na Fakulteti za elektrotehniko na Univerzi Ljubljana.
S projektom sem naredil igro Space Invaders, ki je originalno izšla na Atari konzoli. Moj namen je bil narediti podobno igro z ASCII znaki, vmes sem si
pa dovolil nekatere stvari spremeniti, kot je generacija ovir, namreč sem hotel v nadaljevanju naresti še posamezne levele, vendar zaradi urnika na faksu,
kolokvijev in drugih obštudijskih projektov, ni uspelo. Zaradi tega je program napisan tako, da se mu na lahek način spreminja število ovir in sovražnikov.
V primeru, da mi bo čez poletje dolgčas, se bom s programom še kaj igral.

Princip delovanja igre temelji na enem while loopu, v katerem kličem funkcije, dokler igralec ne zmaga, oziroma je ubit. Te funkcije kontrolirajo premik
sovražnikov, igralca, prihod podatkovin število objektov na igralni površini, poleg tega, ostale funkcijo priskrbijo za pravilno inicializacijo igralca,
ovir in sovražnikov. Vse logične spremenljivke sem zapisal v obliki structov medtem, ko sem pa zapis konstant zapisal kot posamezne globalne spremenljivke.

Ko se bom spet podal v podoben projekt, bom izkoristil raje class namesto structa, saj bi mi dovolil uporabo metod, ki bi olajšale delo. Poleg tega me
je začel zanimati tudi multi-threading, ker trenutno uporabljam metodo busy wait, ki temelji na linearnem izvajanju in čakanju na naslednjo izvedbo loopa.
*/

#include <iostream>
#include <conio.h>
#include <windows.h>
using std::cout;
using std::cin;
using std::endl;

//Globalne spremenljivke, večinoma konstantne vrednosti. Če se bo program nadgrajeval v prihodnjosti in dodalo različne levele, bodo spremenljivke, ki določajo število sovražnikov in ovir, premaknjene v main(), kjer se bo njihovo število inicializiranih spreminjalo.
const unsigned int WIDTH = 40;
const unsigned int HEIGHT = 20;
const unsigned int PLAYER_START_X = WIDTH / 2;
const unsigned int PLAYER_START_Y = HEIGHT - 2;
const unsigned int ENEMY_START_X = 1;
const unsigned int ENEMY_START_Y = 1;
const unsigned int ENEMY_SPACING = 4;
const unsigned int OBSTACLE_START_X = 1;
const unsigned int OBSTACLE_START_Y = HEIGHT - 5;
const unsigned int OBSTACLE_SPACING = 1;
const unsigned int numOfEnemies = 4;
const unsigned int enemyRows = 2;
const unsigned int totalNumOfEn = enemyRows * numOfEnemies;
const unsigned int numOfObstacles = WIDTH - 2;
const unsigned int obstacleRows = 2;
const unsigned int totalNumOfOb = obstacleRows * numOfObstacles;
bool gameOver = false;
int unsigned score = 0;

//Definirana struktura igralca 
struct player {
    unsigned int playerX = WIDTH / 2; //X os igralca
    unsigned int playerY = HEIGHT - 1; //Y os igralca
    unsigned int bulletX = playerX; // X os metka
    unsigned int bulletY = playerY - 1; // Y os metka
    bool isFiring = false; //Stanje streljanja
};

//Definirana struktura sovražnikov (glej strukturo igralca)
struct enemy {
    int unsigned enemyX = 0;
    int unsigned enemyY = 0;
    int unsigned enemyBulletX = 0;
    int unsigned enemyBulletY = 0;
    bool enemyIsFiring = false;
    bool isAlive = true; //Stanje sovražnika - mrtev ali živ
    bool enemyBelow = false; //Stanje sovražnika pod trenutnim - mrtev ali ne / ali sploh je
};

//Definirana struktura ovire (glej strukturo igralca)
struct obstacle {
    unsigned int obstacleX = 0;
    unsigned int obstacleY = 0;
    bool isThere = false;
};

//Funkcija za izrisevanje igralne podlage, ki se jo generira preko cout funkcije (C++ ekvivalent printf(), ki deluje preko underflowanja <<)
void drawBoard(player p1, enemy enemies[], obstacle obstacles[]) {
    //čiščenje cmdja
    COORD coord;
    coord.X = 0;
    coord.Y = 0;
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
    //Izriše zgornji del
    for (int i = 0; i < WIDTH + 2; i++) {
        cout << "-";
    }
    cout << endl;
    //Itiracija skozi stolpce
    for (int i = 0; i < HEIGHT; i++) {
        cout << "|";
        //Iteracija skozi vrstice
        for (int j = 0; j < WIDTH; j++) {
            bool isObstacle = false; //Ali je ovira tam ali ne. Če je, izriše #.

            for (int m = 0; m < totalNumOfOb; m++) {
                if (i == obstacles[m].obstacleY && j == obstacles[m].obstacleX && obstacles[m].isThere) {
                    cout << "#";
                    isObstacle = true;
                    break;
                }
            }
            //Če ni ovire, pogleda če je tam igralec ali pa sovražnik.
            if (!isObstacle) {
                if (i == p1.playerY && j == p1.playerX) {
                    cout << "A";
                }
                else if (i == p1.bulletY && j == p1.bulletX) {
                    cout << "*";
                }
                else {
                    bool enemyAlive = false; //Ali je sovražnik živ. Če je, izpiše M, drugače pa presledek.
                    for (int k = 0; k < totalNumOfEn; k++) {
                        if (i == enemies[k].enemyY && j == enemies[k].enemyX) {
                            cout << "M";
                            enemyAlive = true;
                            break;
                        }
                        else if (i == enemies[k].enemyBulletY && j == enemies[k].enemyBulletX) {
                            cout << "*";
                            enemyAlive = true;
                            break;
                        }
                    }
                    if (!enemyAlive) {
                        cout << " ";
                    }
                }
            }
        }
        //Okvir podlage
        cout << "|" << endl;
    }
    //Spodnji del okvira
    for (int i = 0; i < WIDTH + 2; i++) {
        cout << "-";
    }
    cout << endl;
    //Izpiše rezultat
    cout << "Score: " << score << endl;
}

//Funckija inicalizacije igralca
void playerInit(player& p1) {
    p1.playerX = WIDTH / 2;
    p1.playerY = HEIGHT - 1;
    p1.isFiring = false;
}

//Funckija inicalizacije sovnjihovo število sovražnikov. Pozicija je določena glede na njihovo število po vrsticah 
void enemyInit(enemy enemies[]) {
    for (int j = 0; j < enemyRows; j++) {
        for (int i = 0; i < numOfEnemies; i++) {
            enemies[j * numOfEnemies + i].enemyX = ENEMY_START_X + i * ENEMY_SPACING;
            enemies[j * numOfEnemies + i].enemyY = ENEMY_START_Y + j;
            enemies[j * numOfEnemies + i].enemyIsFiring = false;
            enemies[j * numOfEnemies + i].isAlive = true;
        }
    }
}

//Funckija inicalizacije ovir, ki poteka podobno kot pri sovražnik le, da imajo določeno možnost generiranja.
void obstacleInit(obstacle obstacles[]) {
    srand(time(NULL));
    unsigned int obstacleChance; //Možnost generiranja
    for (int j = 0; j < obstacleRows; j++) {
        for (int i = 0; i < numOfObstacles; i++) {
            
            obstacleChance = rand() % 2;
            if (obstacleChance == 0) {
                obstacles[j * numOfObstacles + i].obstacleX = OBSTACLE_START_X + i * OBSTACLE_SPACING;
                obstacles[j * numOfObstacles + i].obstacleY = OBSTACLE_START_Y + j;
                obstacles[j * numOfObstacles + i].isThere = true;
            }
            else {
                obstacles[j * numOfObstacles + i].obstacleX = OBSTACLE_START_X + i * OBSTACLE_SPACING;
                obstacles[j * numOfObstacles + i].obstacleY = OBSTACLE_START_Y + j;
                obstacles[j * numOfObstacles + i].isThere = false;
            }
        }
    }

}

//Funkcija premika igralca, ki spreminja X koordinato igralca, in njegovo stanje streljanja. Dodana je tudi funkcija exit(0) za prekinitev igre 
void movePlayer(char input, player& p1) {
    switch (input) {
    case 'a':
        if (p1.playerX > 0) {
            p1.playerX--;
        }
        break;
    case 'd':
        if (p1.playerX < WIDTH - 1) {
            p1.playerX++;
        }
        break;
    case ' ':
        if (!p1.isFiring) {
            p1.bulletX = p1.playerX;
            p1.bulletY = p1.playerY + 1;
            p1.isFiring = true;
        }
        break;
    case 'q':
        exit(0);
        break;
    }
}

//funkcija za premik in stanje streljanja sovražnikov
void moveEnemies(enemy enemies[], player& p1) {
    static int enemyDirection = 1; //Smer premikanja
    for (int i = 0; i < totalNumOfEn; i++) {
        if (enemies[i].isAlive) { //Ali je konkreten sovražnik živ, drugače se ga ignorira
            if (enemies[i].enemyX <= 0) { // Če zadane rob, spremeni smer in iterira eno vrstico dol
                enemyDirection = 1;
                for (int j = 0; j < enemyRows; j++) {
                    for (int i = 0; i < numOfEnemies; i++) {
                        enemies[j * numOfEnemies + i].enemyY++;
                    }
                }
                break;
            }
            else if (enemies[i].enemyX == WIDTH - 1) { // Če zadane rob, spremeni smer in iterira eno vrstico dol
                enemyDirection = -1;
                for (int j = 0; j < enemyRows; j++) {
                    for (int i = 0; i < numOfEnemies; i++) {
                        enemies[j * numOfEnemies + i].enemyY++;
                    }
                }
                break;
            }
            //Pripis vrednosti, če je sovražnik pod sovražnikom
            for (int j = i + numOfEnemies; j < totalNumOfEn; j += numOfEnemies) {
                if (enemies[j].isAlive) {
                    enemies[j].enemyBelow = true;

                }
            }
            //Če ni sovražnika pod sovražnikom in če sovražnik ne strela, sovražnik lahko strelja
            if (enemies[i].enemyIsFiring == false && !enemies[i].enemyBelow) {
                srand(time(NULL));
                int chance = rand() % 5;
                if (chance == 0) { //sovražnik strelja, če je vrednost chance deljiva s 5
                    enemies[i].enemyIsFiring = true;
                    enemies[i].enemyBulletX = enemies[i].enemyX;
                    enemies[i].enemyBulletY = enemies[i].enemyY + 1;
                }
            }
            //Izračunane interakcije z metkom in igralcem, tukaj je tudi pripisana možnost za izgub igre
            if (enemies[i].enemyIsFiring == true) {
                enemies[i].enemyBulletY++;
                if (enemies[i].enemyBulletY == p1.playerY && enemies[i].enemyBulletX == p1.playerX) {
                    gameOver = true;
                }
                //Če je na sovražnikov metek doseže spodnji del podlage, sovražnik ne strelja več
                else if (enemies[i].enemyBulletY == HEIGHT - 1) {
                    enemies[i].enemyIsFiring = false;
                }
            }
            //Če sovražnik pride do iste vrstice kot je igralec je igre konec
            if (enemies[i].enemyY == p1.playerY) {
                gameOver = true;
            }
        }
    }
    //Pripis smeri premikanja h koordinatam igralca
    for (int i = 0; i < totalNumOfEn; i++) {
        enemies[i].enemyX += enemyDirection;
    }
}

//Funkcija premikanja metkov
void moveBullets(player& p1, enemy enemies[], obstacle obstacles[]) {
    if (p1.isFiring) {
        p1.bulletY--; //Premik metka navzgor
        if (p1.bulletY <= 0) {
            p1.isFiring = false; //Igralec ne strelja več, ker so metki prišli do zgornje limite
            p1.bulletX = ' '; //X koordinato nastavi na ' ', kar ga efektivno zbriše
            p1.bulletY = ' '; //Y koordinato nastavi na ' ', kar ga efektivno zbriše
        }
        //Igralec zadane sovražnika
        for (int i = 0; i < totalNumOfEn; i++) {
            if (p1.bulletY == enemies[i].enemyY && p1.bulletX == enemies[i].enemyX) {
                p1.isFiring = false;
                enemies[i].enemyX = ' ';
                enemies[i].enemyY = ' ';
                p1.bulletX = ' ';
                p1.bulletY = ' ';
                score += 10; //Poveča točke
                enemies[i].isAlive = false; //Sovražnik je umrl
            }
        }
    }
    //Premik sovražnikovega metka
    for (int i = 0; i <= totalNumOfEn; i++) {
        if (enemies[i].enemyIsFiring) {
            enemies[i].enemyBulletY++; //Premik sovražnikovega metka navzdol
            if (enemies[i].enemyBulletY > HEIGHT) {
                enemies[i].enemyIsFiring = false; //Sovražnik ne strelja več, ker je so metki prišli do spodnje limite
            }
            if (enemies[i].enemyBulletY == p1.playerY && enemies[i].enemyBulletX == p1.playerX) { //Ob premiru, da sovražnik zadane igralca, igralec zgubi
                gameOver = true;
                break;
            }
        }
    }
    for (int i = 0; i < totalNumOfOb; i++) {
        //Igralec zadane oviro
        if (p1.bulletY == obstacles[i].obstacleY && p1.bulletX == obstacles[i].obstacleX && obstacles[i].isThere) {
            p1.isFiring = false;
            obstacles[i].obstacleX = ' ';
            obstacles[i].obstacleY = ' ';
            p1.bulletX = ' ';
            p1.bulletY = ' ';
            score += 1;
            obstacles[i].isThere = false;
        }
        //Sovražnik zadane oviro
        for (int j = 0; j < totalNumOfEn; j++) {
            if (enemies[j].enemyBulletY == obstacles[i].obstacleY && enemies[j].enemyBulletX == obstacles[i].obstacleX && obstacles[i].isThere) {
                obstacles[i].obstacleX = ' ';
                obstacles[i].obstacleY = ' ';
                enemies[j].enemyBulletX = ' ';
                enemies[j].enemyBulletY = ' ';
                enemies[j].enemyIsFiring = false;
                obstacles[i].isThere = false;
            }
        }
    }
}

//Funkcija, ki vrne število živih sovražnikov
int getCurrentEnemies(enemy enemies[]) {
    int alive = 0;
    for (int i = 0; i < totalNumOfEn; i++) {
        if (enemies[i].isAlive) {
            alive++;
        }

    }
    return alive;
}

int main() {
    srand(time(NULL)); //Funkcija ki prične timer za slučajno generacijo
    struct player player1; //Inicializacija igralca
    struct obstacle obstacles[totalNumOfOb]; //Inicializacija ovir
    struct enemy enemies[totalNumOfEn]{}; //Inicializacija sovražnikov
    playerInit(player1); //klic funkcije za določitev začetnih vrednosti
    enemyInit(enemies); //klic funkcije za določitev začetnih vrednosti
    obstacleInit(obstacles); //klic funkcije za določitev začetnih vrednosti
    char input; //Inilizacija podatkov za premik
    int timer = 0; //Inicializacija timerja, pomemben za časovni premik
    int numOfAlive; //Inicializacija vrednosti živih sovražnikov
    while (!gameOver) { //Dokler igre ni konec - konec je podan znotraj funkcij
        numOfAlive = getCurrentEnemies(enemies); 
        //Nastavitev hitrosti premika sovražnikov
        if (numOfAlive < 5) {
            if (timer % 2 == 0) {
                moveEnemies(enemies, player1);
            }
        }
        //Nastavitev hitrosti premika sovražnikov
        else {
            if (timer % 5 == 0) {
                moveEnemies(enemies, player1);
            }
        }
        moveBullets(player1, enemies, obstacles);
        drawBoard(player1, enemies, obstacles);
        if (_kbhit()) { //Proži se, če je tipkovnica zazna spremembno stanja
            input = _getch(); //Pripiše vrednost udarjane tipke v input
            movePlayer(input, player1);
        }
        timer++; //Povečanje vrednosti timerja
        Sleep(10); //Uravnavanje hitrosti igre
        //Tukaj je napisan konec pogoj za zmago
        if (numOfAlive == 0) { //Zmaga se, pod pogojem, da je število sovražnikov enako nič
            cout << "YOU HAVE WON";
            exit(0); //Konča program
        }
    }
    cout << "GAME OVER";
    return 0;
}

/*
void enemyInit(enemy enemies[]) {
    for (int i = 0; i < numOfEnemies / 2; i++) {
        if (i == 0) {
            enemies[i].enemyX = ENEMY_START_X;
            enemies[i].enemyY = ENEMY_START_Y;
        }
        else {
            enemies[i].enemyX = enemies[i - 1].enemyX + ENEMY_SPACING;
            enemies[i].enemyY = enemies[i - 1].enemyY;
        }
        enemies[i].enemyIsFiring = false;
    }
    for (int j = 0; j < numOfEnemies / 2; j++) {
        if (j == 0) {
            enemies[j].enemyX = ENEMY_START_X;
            enemies[j].enemyY = ENEMY_START_Y + 1;
        }
        else {
            enemies[j].enemyX = enemies[j - 1].enemyX + ENEMY_SPACING;
            enemies[j].enemyY = enemies[j - 1].enemyY;
        }
        enemies[j].enemyIsFiring = false;
    }
}


/*
#include <iostream>
#include <conio.h>
#include <windows.h>
using namespace std;

const unsigned int WIDTH = 40;
const unsigned int HEIGHT = 20;
const unsigned int PLAYER_START_X = WIDTH / 2;
const unsigned int PLAYER_START_Y = HEIGHT - 2;
const unsigned int ENEMY_START_X = 1;
const unsigned int ENEMY_START_Y = 1;
const unsigned int ENEMY_SPACING = 4;
const unsigned int numOfEnemies = 7;
const unsigned int enemyRows = 4;
bool gameOver = false;
int unsigned score = 0;
const unsigned int totalNumOfEn = enemyRows * numOfEnemies;

struct player {
    int unsigned playerX = WIDTH / 2;
    int unsigned playerY = HEIGHT - 1;
    int unsigned bulletX = playerX;
    int unsigned bulletY = playerY - 1;
    bool isFiring = false;
};

struct enemy {
    int unsigned enemyX = 0;
    int unsigned enemyY = 0;
    bool enemyIsFiring = false;
    int unsigned enemyBulletX = 0;
    int unsigned enemyBulletY = 0;
    bool isAlive = true;
    bool enemyBelow = false;
};




void drawBoard(player p1, enemy enemies[]) {
    system("cls");
    for (int i = 0; i < WIDTH + 2; i++) {
        cout << "-";
    }
    cout << endl;
    for (int i = 0; i < HEIGHT; i++) {
        for (int j = 0; j < WIDTH; j++) {
            if (j == 0) {
                cout << "|";
            }
            if (i == p1.playerY && j == p1.playerX) {
                cout << "A";
            }
            else if (i == p1.bulletY && j == p1.bulletX) {
                cout << "*";
            }
            else {
                bool enemyAlive = false;
                for (int k = 0; k < totalNumOfEn; k++) {
                    if (i == enemies[k].enemyY && j == enemies[k].enemyX) {
                        cout << "M";
                        enemyAlive = true;
                        break;
                    }
                    else if (i == enemies[k].enemyBulletY && j == enemies[k].enemyBulletX) {
                        cout << "*";
                        enemyAlive = true;
                        break;
                    }
                }
                if (!enemyAlive) {
                    cout << " ";
                }
            }
            if ((j == WIDTH - 1)) {
                cout << "|";
            }
        }
        cout << endl;
    }
    for (int i = 0; i < WIDTH + 2; i++) {
        cout << "-";
    }
    cout << endl;
    cout << "Score: " << score << endl;
}

void playerInit(player& p1) {
    p1.playerX = WIDTH / 2;
    p1.playerY = HEIGHT - 1;
    p1.isFiring = false;
}

void enemyInit(enemy enemies[]) {
    for (int j = 0; j < enemyRows; j++) {
        for (int i = 0; i < numOfEnemies; i++) {
            enemies[j * numOfEnemies + i].enemyX = ENEMY_START_X + i * ENEMY_SPACING;
            enemies[j * numOfEnemies + i].enemyY = ENEMY_START_Y + j;
            enemies[j * numOfEnemies + i].enemyIsFiring = false;
            enemies[j * numOfEnemies + i].isAlive = true;
        }
    }
}

void movePlayer(char input, player& p1) {
    switch (input) {
    case 'a':
        if (p1.playerX > 0) {
            p1.playerX--;
        }
        break;
    case 'd':
        if (p1.playerX < WIDTH - 1) {
            p1.playerX++;
        }
        break;
    case ' ':
        if (!p1.isFiring) {
            p1.bulletX = p1.playerX;
            p1.bulletY = p1.playerY + 1;
            p1.isFiring = true;
        }
        break;
    case 'q':
        exit(0);
        break;
    }
}


void moveEnemies(enemy enemies[], player &p1) {
    static int enemyDirection = 1;
    for (int i = 0; i < totalNumOfEn; i++) {
        if (enemies[i].isAlive) {
            if (enemies[i].enemyX <= 0) {
                enemyDirection = 1;
                for (int j = 0; j < enemyRows; j++) {
                    for (int i = 0; i < numOfEnemies; i++) {
                        enemies[j * numOfEnemies + i].enemyY++;
                    }
                }
                break;

            }
            else if (enemies[i].enemyX == WIDTH - 1) {
                enemyDirection = -1;
                for (int j = 0; j < enemyRows; j++) {
                    for (int i = 0; i < numOfEnemies; i++) {
                        enemies[j * numOfEnemies + i].enemyY++;
                    }
                }
                break;
            }
            else {

            }

            for (int j = 0; j < enemyRows; j++) {
                for (int i = 0; i < numOfEnemies; i++) {
                    if (enemies[j * numOfEnemies + i].isAlive) {
                        enemies[j + i].enemyBelow = true;
                        //break;
                    }
                }
            }

            if (enemies[i].enemyIsFiring == false && !enemies[i].enemyBelow) {
                srand(time(NULL));
                int chance = rand() % 5;
                if (chance == 0) {
                    enemies[i].enemyIsFiring = true;
                    enemies[i].enemyBulletX = enemies[i].enemyX;
                    enemies[i].enemyBulletY = enemies[i].enemyY + 1;
                }
            }
            if (enemies[i].enemyIsFiring == true) {
                enemies[i].enemyBulletY++;
                if (enemies[i].enemyBulletY == p1.playerY && enemies[i].enemyBulletX == p1.playerX) {
                    gameOver = true;
                }
                else if (enemies[i].enemyBulletY == HEIGHT - 1) {
                    enemies[i].enemyIsFiring = false;
                }
            }
            if (enemies[i].enemyY == p1.playerY) {
                gameOver = true;
            }
        }
    }
    for (int i = 0; i < totalNumOfEn; i++) {
        enemies[i].enemyX += enemyDirection;
    }
}




void moveBullets(player& p1, enemy enemies[]) {
    if (p1.isFiring) {
        p1.bulletY--;
        if (p1.bulletY <= 0) {
            p1.isFiring = false;
            p1.bulletX = ' ';
        }
        for (int i = 0; i <= totalNumOfEn; i++) {
            if (p1.bulletY == enemies[i].enemyY && p1.bulletX == enemies[i].enemyX) {
                p1.isFiring = false;
                enemies[i].enemyX = ' ';
                enemies[i].enemyY = ' ';
                p1.bulletX = ' ';
                p1.bulletY = ' ';
                score += 10;
                enemies[i].isAlive = false;
            }
        }
    }
    for (int i = 0; i <= totalNumOfEn; i++) {
        if (enemies[i].enemyIsFiring) {
            enemies[i].enemyBulletY++;
            if (enemies[i].enemyBulletY > HEIGHT) {
                enemies[i].enemyIsFiring = false;
            }
            if (enemies[i].enemyBulletY == p1.playerY && enemies[i].enemyBulletX == p1.playerX) {
                gameOver = true;
                break;
            }
        }
    }
}


int getCurrentEnemies(enemy enemies[]) {
    int alive = 0;
    for (int i = 0; i < totalNumOfEn; i++) {
        if (enemies[i].isAlive) {
            alive++;
        }

    }
    return alive;
}

int main() {
    srand(time(NULL));
    struct player player1;
    struct enemy enemies[totalNumOfEn]{};
    playerInit(player1);
    enemyInit(enemies);
    char input;
    int timer = 0;
    int numOfAlive;
    while (!gameOver) {
        numOfAlive = getCurrentEnemies(enemies);
        if (numOfAlive < 5) {
            if (timer % 2 == 0) {
                moveEnemies(enemies, player1);
            }
        }
        else {
            if (timer % 5 == 0) {
                moveEnemies(enemies, player1);
            }
        }
        moveBullets(player1, enemies);
        drawBoard(player1, enemies);
        if (_kbhit()) {
            input = _getch();
            movePlayer(input, player1);
        }
        timer++;
        Sleep(10);
    }
    cout << "GAME OVER";
    return 0;
}
*/
/*
void enemyInit(enemy enemies[]) {
    for (int i = 0; i < numOfEnemies / 2; i++) {
        if (i == 0) {
            enemies[i].enemyX = ENEMY_START_X;
            enemies[i].enemyY = ENEMY_START_Y;
        }
        else {
            enemies[i].enemyX = enemies[i - 1].enemyX + ENEMY_SPACING;
            enemies[i].enemyY = enemies[i - 1].enemyY;
        }
        enemies[i].enemyIsFiring = false;
    }
    for (int j = 0; j < numOfEnemies / 2; j++) {
        if (j == 0) {
            enemies[j].enemyX = ENEMY_START_X;
            enemies[j].enemyY = ENEMY_START_Y + 1;
        }
        else {
            enemies[j].enemyX = enemies[j - 1].enemyX + ENEMY_SPACING;
            enemies[j].enemyY = enemies[j - 1].enemyY;
        }
        enemies[j].enemyIsFiring = false;
    }
}

*/
/*
void drawBoard(player p1, enemy enemies[]) {
    system("cls");
    for (int i = 0; i < WIDTH + 2; i++) {
        cout << "-";
    }
    cout << endl;
    for (int i = 0; i < HEIGHT; i++) {
        for (int j = 0; j < WIDTH; j++) {
            if (j == 0) {
                cout << "|";
            }
            if (i == p1.playerY && j == p1.playerX) {
                cout << "A";
            }
            else if (i == p1.bulletY && j == p1.bulletX) {
                cout << "*";
            }
            else {
                bool enemyAlive = false;
                for (int k = 0; k < numOfEnemies; k++) {
                    if (i == enemies[k].enemyY && j == enemies[k].enemyX) {
                        cout << "M";
                        enemyAlive = true;
                        break;
                    }
                    else if (i == enemies[k].enemyBulletY && j == enemies[k].enemyBulletX) {
                        cout << "*";
                        enemyAlive = true;
                        break;
                    }
                }
                if (!enemyAlive) {
                    cout << " ";
                }
            }
            if ((j == WIDTH - 1)) {
                cout << "|";
            }
        }
        cout << endl;
    }
    for (int i = 0; i < WIDTH + 2; i++) {
        cout << "-";
    }
    cout << endl;
    cout << "Score: " << score << endl;
}
*/

/*
void moveEnemies(enemy enemies[], player& p1) {
    static int enemyDirection = 1;
    static bool enemyReachedEdge = false;
    for (int i = 0; i < numOfEnemies; i++) {
        if (enemies[i].isAlive) {
            if (enemies[i].enemyX == 0 && enemies[i].isAlive == true) {
                enemyDirection = 1;
                enemyReachedEdge = true;
            }
            else if (enemies[i].enemyX == WIDTH - 1) {
                enemyDirection = -1;
                enemyReachedEdge = true;
            }
            else {
                enemyReachedEdge = false;
            }
            if (enemies[i].enemyIsFiring == false) {
                srand(time(NULL));
                int chance = rand() % 20;
                if (chance == 0) {
                    enemies[i].enemyIsFiring = true;
                    enemies[i].enemyBulletX = enemies[i].enemyX;
                    enemies[i].enemyBulletY = enemies[i].enemyY + 1;
                }
            }
            if (enemies[i].enemyIsFiring == true) {
                enemies[i].enemyBulletY++;
                if (enemies[i].enemyBulletY == p1.playerY && enemies[i].enemyBulletX == p1.playerX) {
                    gameOver = true;
                    return;
                }
            }
            enemies[i].enemyX += enemyDirection;
        }
    }
    // Move enemies down if they have reached the edge
    if (enemyReachedEdge) {
        for (int i = 0; i < numOfEnemies; i++) {
            if (enemies[i].isAlive) {
                enemies[i].enemyY++;
            }
        }
    }
}
*/

/*
void enemyInit(enemy enemies[]) {
    for (int j = 0; j < enemyRows; j++) {
        for (int i = 0; i < numOfEnemies; i++) {
            if (i == 0) {
                enemies[i].enemyX = ENEMY_START_X;
                enemies[i].enemyY = ENEMY_START_Y + j;
            }
            else {
                enemies[i].enemyX = enemies[i - 1].enemyX + ENEMY_SPACING;
                enemies[i].enemyY = enemies[i - 1].enemyY;
            }
            enemies[i].enemyIsFiring = false;
        }
    }


}
*/

/*
void moveEnemies(enemy enemies[], player& p1) {
    static int enemyDirection = 1;
    for (int i = 0; i < totalNumOfEn; i++) {
        if (enemies[i].isAlive) {
            if (enemies[i].enemyX <= 0) {
                enemyDirection = 1;
                for (int j = 0; j < enemyRows; j++) {
                    for (int i = 0; i < numOfEnemies; i++) {
                        enemies[j * numOfEnemies + i].enemyY++;
                    }
                }
                break;
            }
            else if (enemies[i].enemyX == WIDTH - 1) {
                enemyDirection = -1;
                for (int j = 0; j < enemyRows; j++) {
                    for (int i = 0; i < numOfEnemies; i++) {
                        enemies[j * numOfEnemies + i].enemyY++;
                    }
                }
                break;
            }
            else {
                // Check if there is an alive enemy below this one

                int row = (i / numOfEnemies) + 1;
                for (int k = 0; k < numOfEnemies; k++) {
                    if (enemies[row * numOfEnemies + k].isAlive) {
                        enemies[k].enemyBelow = true;
                        break;
                    }
                }
                if (!enemies[i].enemyBelow && enemies[i].enemyIsFiring == false) {
                    srand(time(NULL));
                    int chance = rand() % 5;
                    if (chance == 0) {
                        enemies[i].enemyBulletX = enemies[i].enemyX;
                        enemies[i].enemyBulletY = enemies[i].enemyY + 1;
                        enemies[i].enemyIsFiring = true;
                    }
                }
                if (enemies[i].enemyIsFiring == true) {
                    enemies[i].enemyBulletY++;
                    if (enemies[i].enemyBulletY == p1.playerY && enemies[i].enemyBulletX == p1.playerX) {
                        gameOver = true;
                    }
                    else if (enemies[i].enemyBulletY == HEIGHT - 1) {
                        enemies[i].enemyIsFiring = false;
                    }
                }
                if (enemies[i].enemyY == p1.playerY) {
                    gameOver = true;
                }
            }
        }
        for (int i = 0; i < totalNumOfEn; i++) {
            enemies[i].enemyX += enemyDirection;
        }
    }
    // ... rest of the function
}
*/

/*
void drawBoard(player p1, enemy enemies[]) {
    //system("cls");
    COORD coord;
    coord.X = 0;
    coord.Y = 0;
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
    for (int i = 0; i < WIDTH + 2; i++) {
        cout << "-";
    }
    cout << endl;
    for (int i = 0; i < HEIGHT; i++) {
        for (int j = 0; j < WIDTH; j++) {
            if (j == 0) {
                cout << "|";
            }
            if (i == p1.playerY && j == p1.playerX) {
                cout << "A";
            }
            else if (i == p1.bulletY && j == p1.bulletX) {
                cout << "*";
            }
            else {
                bool enemyAlive = false;
                for (int k = 0; k < totalNumOfEn; k++) {
                    if (i == enemies[k].enemyY && j == enemies[k].enemyX) {
                        cout << "M";
                        enemyAlive = true;
                        break;
                    }
                    else if (i == enemies[k].enemyBulletY && j == enemies[k].enemyBulletX) {
                        cout << "*";
                        enemyAlive = true;
                        break;
                    }
                }
                if (!enemyAlive) {
                    cout << " ";
                }
            }
            if ((j == WIDTH - 1)) {
                cout << "|";
            }
        }
        cout << endl;
    }
    for (int i = 0; i < WIDTH + 2; i++) {
        cout << "-";
    }
    cout << endl;
    cout << "Score: " << score << endl;
}
*/

/*
void drawBoard(player p1, enemy enemies[], obstacle obstacles[]) {
    //system("cls");
    COORD coord;
    coord.X = 0;
    coord.Y = 0;
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
    for (int i = 0; i < WIDTH + 2; i++) {
        cout << "-";
    }
    cout << endl;
    for (int i = 0; i < HEIGHT; i++) {
        for (int j = 0; j < WIDTH; j++) {

            if (j == 0) {
                cout << "|";
            }
            for (int m = 0; m < totalNumOfOb; m++) {
                if (i == obstacles[m].obstacleY && j == obstacles[m].obstacleX && obstacles[m].isThere) {
                    cout << "#";
                    break;

                }
            }
            if (i == p1.playerY && j == p1.playerX) {
                cout << "A";
            }
            else if (i == p1.bulletY && j == p1.bulletX) {
                cout << "*";
            }
            else {

                bool enemyAlive = false;
                for (int k = 0; k < totalNumOfEn; k++) {
                    if (i == enemies[k].enemyY && j == enemies[k].enemyX) {
                        cout << "M";
                        enemyAlive = true;
                        break;
                    }
                    else if (i == enemies[k].enemyBulletY && j == enemies[k].enemyBulletX) {
                        cout << "*";
                        enemyAlive = true;
                        break;
                    }
                }
                if (!enemyAlive) {
                    cout << " ";
                }
            }
            if ((j == WIDTH - 1)) {
                cout << "|";
            }
        }
        cout << endl;
    }
    for (int i = 0; i < WIDTH + 2; i++) {
        cout << "-";
    }
    cout << endl;
    cout << "Score: " << score << endl;
}
*/