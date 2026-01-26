/*
 * KOMPRESOR HUFFMANA - Projekt AiSD
 * Kompresja i dekompresja plikow algorytmem Huffmana
 */

#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <map>
#include <sstream>
#include <cstdlib>
#include <iomanip>
#include <utility>

using namespace std;

// ~~~~ STRUKTURA WEZLA ~~~~
// Wezel drzewa Huffmana: przechowuje znak, czestotliwosc i wskazniki na dzieci

struct Wezel {
    char zn;      // znak (dla lisci) lub '\0' (dla wezlow wewnetrznych)
    int cz;       // czestotliwosc wystapien znaku
    Wezel *L, *P; // lewe i prawe dziecko

    // Konstruktor liscia - wezel z jednym znakiem
    Wezel(char z, int c) : zn(z), cz(c), L(NULL), P(NULL) {}

    // Konstruktor wezla wewnetrznego - laczy dwa poddrzewa
    Wezel(int c, Wezel* l, Wezel* p) : zn('\0'), cz(c), L(l), P(p) {}
};

// ~~~~ KOPIEC (min-heap) ~~~~
// Kolejka priorytetowa oparta na kopcu binarnym
// Element o najmniejszej czestotliwosci jest zawsze na szczycie (indeks 0)

// Naprawia kopiec w GORE - uzywane po dodaniu elementu na koniec
// Porownuje element z rodzicem i zamienia jesli trzeba
void naprawGore(vector<Wezel*>& K, int i) {
    while (i > 0 && K[i]->cz < K[(i-1)/2]->cz) {
        swap(K[i], K[(i-1)/2]);  // zamien z rodzicem
        i = (i-1)/2;              // przejdz do rodzica
    }
}

// Naprawia kopiec w DOL - uzywane po usunieciu korzenia
// Porownuje element z dziecmi i zamienia z mniejszym
void naprawDol(vector<Wezel*>& K, int i) {
    int n = K.size();
    while (2*i+1 < n) {                              // dopoki ma dzieci
        int naj = i, l = 2*i+1, p = 2*i+2;           // indeksy: obecny, lewy, prawy
        if (K[l]->cz < K[naj]->cz) naj = l;          // lewe dziecko mniejsze?
        if (p < n && K[p]->cz < K[naj]->cz) naj = p; // prawe dziecko mniejsze?
        if (naj == i) break;                          // juz na miejscu
        swap(K[i], K[naj]);                           // zamien z mniejszym dzieckiem
        i = naj;                                      // przejdz do dziecka
    }
}

// Dodaje element do kopca - wstawia na koniec i naprawia w gore
void dodaj(vector<Wezel*>& K, Wezel* w) {
    K.push_back(w);
    naprawGore(K, K.size()-1);
}

// Pobiera i usuwa MINIMUM (korzen) - przenosi ostatni na gore i naprawia w dol
Wezel* pobierzMin(vector<Wezel*>& K) {
    if (K.empty()) return NULL;
    Wezel* m = K[0];       // zapamietaj minimum
    K[0] = K.back();       // przenies ostatni na poczatek
    K.pop_back();          // usun ostatni
    if (!K.empty()) naprawDol(K, 0);  // napraw kopiec
    return m;
}

// Zmienia priorytet elementu o danym znaku
// Szuka liscia, zmienia wartosc i naprawia kopiec w odpowiednim kierunku
int zmienPrio(vector<Wezel*>& K, char z, int nc) {
    for (size_t i = 0; i < K.size(); i++) {
        if (!K[i]->L && !K[i]->P && K[i]->zn == z) {  // znaleziono lisc
            int st = K[i]->cz;    // stara wartosc
            K[i]->cz = nc;        // nowa wartosc
            (nc < st) ? naprawGore(K, i) : naprawDol(K, i);  // napraw
            return 1;  // sukces
        }
    }
    return 0;  // nie znaleziono
}

// Buduje kopiec z tablicy (algorytm Floyda - O(n))
// Zaczyna od polowy i naprawia kazdy wezel w dol
void budujKopiec(vector<Wezel*>& K, vector<Wezel*>& D) {
    K = D;
    for (int i = K.size()/2 - 1; i >= 0; i--) naprawDol(K, i);
}

// Wypisuje zawartosc kopca - do debugowania
void pokazKopiec(vector<Wezel*>& K) {
    cout << "  | Kopiec: ";
    if (K.empty()) { cout << "(pusty)\n"; return; }
    for (size_t i = 0; i < K.size(); i++)
        cout << "(" << (K[i]->zn ? K[i]->zn : '#') << ":" << K[i]->cz << ") ";
    cout << endl;
}

// ~~~~ DRZEWO HUFFMANA ~~~~

// Usuwa drzewo rekurencyjnie - zwalnia pamiec
void usunDrzewo(Wezel* w) {
    if (!w) return;
    usunDrzewo(w->L);  // usun lewe poddrzewo
    usunDrzewo(w->P);  // usun prawe poddrzewo
    delete w;          // usun wezel
}

// Generuje kody Huffmana rekurencyjnie
// Idac w lewo dodaje '0', w prawo '1'
// Kod zapisuje tylko dla lisci (wezlow bez dzieci)
void generujKody(Wezel* w, string s, map<char,string>& kody) {
    if (!w) return;
    if (!w->L && !w->P) { kody[w->zn] = s; return; }  // lisc - zapisz kod
    generujKody(w->L, s+"0", kody);  // lewo = 0
    generujKody(w->P, s+"1", kody);  // prawo = 1
}

// Buduje drzewo Huffmana z mapy czestotliwosci
// 1. Tworzy lisc dla kazdego znaku
// 2. Laczy dwa najmniejsze wezly az zostanie jeden (korzen)
Wezel* budujDrzewo(map<char,int>& freq) {
    vector<Wezel*> K;

    // Dodaj wszystkie znaki jako liscie do kopca
    for (map<char,int>::iterator it = freq.begin(); it != freq.end(); ++it)
        dodaj(K, new Wezel(it->first, it->second));

    // Lacz dwa najmniejsze az zostanie jeden
    while (K.size() > 1) {
        Wezel* a = pobierzMin(K);  // najmniejszy
        Wezel* b = pobierzMin(K);  // drugi najmniejszy
        dodaj(K, new Wezel(a->cz + b->cz, a, b));  // nowy rodzic
    }

    return K.empty() ? NULL : K[0];  // zwroc korzen
}

// ~~~~ KOMPRESJA / DEKOMPRESJA ~~~~

// Formatuje znak do wyswietlenia (znaki specjalne jako etykiety)
string fmtZnak(char c) {
    if (c == '\n') return "[NL]";   // nowa linia
    if (c == ' ') return "[SP]";    // spacja
    if (c == '\t') return "[TAB]";  // tabulator
    return string(1, c);            // zwykly znak
}

// Konwertuje string bitow na znaki (8 bitow = 1 znak)
// Zwraca pare: (zakodowane znaki, liczba bitow paddingu)
pair<string, int> bityNaZnaki(const string& bity) {
    string wynik = "";
    int padding = 0;

    // Oblicz ile bitow paddingu potrzeba (dopelnienie do 8)
    if (bity.size() % 8 != 0) {
        padding = 8 - (bity.size() % 8);
    }

    // Dodaj padding na koncu (zera)
    string bityPad = bity + string(padding, '0');

    // Konwertuj co 8 bitow na znak
    for (size_t i = 0; i < bityPad.size(); i += 8) {
        unsigned char bajt = 0;
        for (int j = 0; j < 8; j++) {
            bajt = (bajt << 1) | (bityPad[i + j] - '0');
        }
        wynik += (char)bajt;
    }

    return make_pair(wynik, padding);
}

// KOMPRESJA: plik tekstowy -> plik .huff
void kompresuj(string wej, string wyj) {
    // 1. Otworz plik
    ifstream f(wej.c_str());
    if (!f) { cout << "\n  [!] Nie mozna otworzyc: " << wej << endl; return; }

    // 2. Wczytaj zawartosc i policz czestotliwosci
    string txt = "";
    map<char,int> freq;
    char c;
    while (f.get(c)) { freq[c]++; txt += c; }
    f.close();
    if (txt.empty()) { cout << "\n  [!] Pusty plik.\n"; return; }

    // 3. Zbuduj drzewo i wygeneruj kody
    Wezel* root = budujDrzewo(freq);
    map<char,string> kody;
    generujKody(root, "", kody);

    // 4. Pokaz tabele kodow
    cout << "\n  .------------------------------------------.\n";
    cout << "  |           TABLICA KODOW HUFFMANA         |\n";
    cout << "  |------------------------------------------|\n";
    for (map<char,int>::iterator it = freq.begin(); it != freq.end(); ++it)
        cout << "  |  " << setw(4) << left << fmtZnak(it->first)
             << "  |  " << setw(5) << right << it->second
             << "  |  " << setw(16) << left << kody[it->first] << "|\n";
    cout << "  '------------------------------------------'\n";

    // 5. Wygeneruj ciag bitow
    string bity = "";
    for (size_t i = 0; i < txt.size(); i++) bity += kody[txt[i]];

    // 6. Konwertuj bity na znaki (8 bitow = 1 bajt)
    pair<string, int> zakodowane = bityNaZnaki(bity);
    string znakiWyj = zakodowane.first;
    int padding = zakodowane.second;

    // 7. Zapisz do pliku binarnego
    // Format: slownik\npadding\nzakodowane_bajty
    ofstream o(wyj.c_str(), ios::binary);
    for (map<char,int>::iterator it = freq.begin(); it != freq.end(); ++it)
        o << (int)it->first << ":" << it->second << ";";  // format: ASCII:ile;
    o << "\n" << padding << "\n";  // liczba bitow paddingu
    o.write(znakiWyj.c_str(), znakiWyj.size());  // bajty binarne
    o.close();

    // 8. Pokaz statystyki
    cout << "\n  Oryginalny rozmiar:  " << txt.size() << " bajtow\n";
    cout << "  Skompresowany:       " << znakiWyj.size() << " bajtow (+ naglowek)\n";
    cout << "  Padding:             " << padding << " bitow\n";

    usunDrzewo(root);
    cout << "\n  [OK] Zapisano: " << wyj << endl;
}

// Konwertuje znaki na string bitow (1 znak = 8 bitow)
string znakiNaBity(const string& znaki, int padding) {
    string bity = "";

    // Konwertuj kazdy znak na 8 bitow
    for (size_t i = 0; i < znaki.size(); i++) {
        unsigned char bajt = (unsigned char)znaki[i];
        for (int j = 7; j >= 0; j--) {
            bity += ((bajt >> j) & 1) ? '1' : '0';
        }
    }

    // Usun bity paddingu z konca
    if (padding > 0 && bity.size() >= (size_t)padding) {
        bity = bity.substr(0, bity.size() - padding);
    }

    return bity;
}

// DEKOMPRESJA: plik .huff -> plik tekstowy
void dekompresuj(string wej, string wyj) {
    // 1. Otworz plik binarnie
    ifstream f(wej.c_str(), ios::binary);
    if (!f) { cout << "\n  [!] Nie mozna otworzyc: " << wej << endl; return; }

    // 2. Wczytaj slownik z pierwszej linii
    string lin;
    getline(f, lin);
    map<char,int> freq;
    stringstream ss(lin);
    string seg;
    while (getline(ss, seg, ';')) {
        size_t p = seg.find(':');
        if (p != string::npos)
            freq[(char)atoi(seg.substr(0,p).c_str())] = atoi(seg.substr(p+1).c_str());
    }

    // 3. Wczytaj liczbe bitow paddingu
    string padStr;
    getline(f, padStr);
    int padding = atoi(padStr.c_str());

    // 4. Wczytaj reszta pliku jako bajty binarne
    string zakodowane = "";
    char c;
    while (f.get(c)) {
        zakodowane += c;
    }
    f.close();

    // 5. Konwertuj bajty na bity
    string bity = znakiNaBity(zakodowane, padding);

    // 6. Odbuduj drzewo (identyczne jak przy kompresji)
    Wezel* root = budujDrzewo(freq);

    // 7. Dekoduj: idz po drzewie zgodnie z bitami
    ofstream o(wyj.c_str());
    Wezel* w = root;
    for (size_t i = 0; i < bity.size() && w; i++) {
        w = (bity[i] == '0') ? w->L : w->P;  // 0=lewo, 1=prawo
        if (w && !w->L && !w->P) {           // dotarlismy do liscia
            o << w->zn;                       // wypisz znak
            w = root;                         // wroc do korzenia
        }
    }
    o.close();

    usunDrzewo(root);
    cout << "\n  [OK] Odtworzono: " << wyj << endl;
}

// ~~~~ INTERFEJS UZYTKOWNIKA ~~~~

void baner() {
    cout << "\n  ##   ##  ##   ##  ######  ######  ##   ##    ##    ##   ##\n";
    cout << "  ##   ##  ##   ##  ##      ##      ### ###   ####   ###  ##\n";
    cout << "  #######  ##   ##  ####    ####    ## # ##  ##  ##  ## # ##\n";
    cout << "  ##   ##  ##   ##  ##      ##      ##   ##  ######  ##  ###\n";
    cout << "  ##   ##   #####   ##      ##      ##   ##  ##  ##  ##   ##\n";
    cout << "\n        K O M P R E S O R   P L I K O W   v1.0\n";
}

// Wczytuje liczbe od uzytkownika
int wczytaj() {
    string s;
    if (cin.peek() == '\n') cin.ignore();
    getline(cin, s);
    return atoi(s.c_str());
}

// Wczytuje pare: znak i liczba (np. "A 5")
int wczytajPare(char* z, int* p) {
    string s;
    if (cin.peek() == '\n') cin.ignore();
    getline(cin, s);
    stringstream ss(s);
    return (ss >> *z >> *p) ? 1 : 0;
}

// Menu testowania kolejki priorytetowej
void menuKolejki() {
    vector<Wezel*> K;
    int op;
    char z; int p;

    do {
        cout << "\n  ======= KOLEJKA PRIORYTETOWA =======\n";
        pokazKopiec(K);
        cout << "  [1] Wstaw  [2] Pobierz min  [3] Zmien prio  [4] Buduj  [0] Wroc\n";
        cout << "  > "; op = wczytaj();

        if (op == 1) {
            cout << "    Znak i priorytet: ";
            if (wczytajPare(&z, &p)) { dodaj(K, new Wezel(z, p)); cout << "    OK\n"; }
            else cout << "    Blad!\n";
        }
        else if (op == 2) {
            Wezel* w = pobierzMin(K);
            if (w) { cout << "    Pobrano: " << w->zn << ":" << w->cz << endl; delete w; }
            else cout << "    Pusty!\n";
        }
        else if (op == 3) {
            cout << "    Znak i nowy priorytet: ";
            if (wczytajPare(&z, &p)) cout << (zmienPrio(K, z, p) ? "    OK\n" : "    Nie znaleziono!\n");
            else cout << "    Blad!\n";
        }
        else if (op == 4) {
            cout << "    [a] Przyklad  [b] Wlasne: ";
            string c; getline(cin, c);
            vector<Wezel*> D;
            if (c == "a") {
                D.push_back(new Wezel('P', 12));
                D.push_back(new Wezel('Q', 4));
                D.push_back(new Wezel('R', 7));
                budujKopiec(K, D);
                cout << "    Zbudowano!\n";
            } else if (c == "b") {
                cout << "    Wpisuj (0 0 = koniec):\n";
                while (wczytajPare(&z, &p) && p != 0) D.push_back(new Wezel(z, p));
                if (!D.empty()) { budujKopiec(K, D); cout << "    Zbudowano!\n"; }
            }
        }
    } while (op != 0);

    for (size_t i = 0; i < K.size(); i++) delete K[i];
}

// ~~~~ MAIN ~~~~

int main() {
    string we, wy;
    int op;
    baner();

    do {
        cout << "\n  ============ MENU ============\n";
        cout << "  [1] Kompresuj  [2] Dekompresuj  [3] Test kolejki  [0] Wyjscie\n";
        cout << "  > "; op = wczytaj();

        if (op == 1) {
            cout << "    Plik wejsciowy: "; cin >> we;
            cout << "    Plik wyjsciowy: "; cin >> wy;
            kompresuj(we, wy);
        }
        else if (op == 2) {
            cout << "    Plik .huff: "; cin >> we;
            cout << "    Zapisz jako: "; cin >> wy;
            dekompresuj(we, wy);
        }
        else if (op == 3) menuKolejki();
        else if (op == 0) cout << "\n  Pa!\n";
    } while (op != 0);

    return 0;
}
