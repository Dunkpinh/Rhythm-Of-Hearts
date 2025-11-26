#include <iostream>
#include <cstdlib>
#include <ctime>
#include <conio.h> 
#include <windows.h>
#include <chrono>
#include <thread>
#include <fstream>
#include <vector>
#include <string>
#include <iomanip>

using namespace std;

// atribut dan method global

// struct Record untuk menyimpan history skor
struct Record {
    string nickname;
    int score;
    string tanggal;
};

string spade = u8"\u2660";
string heart = u8"\u2665";
string club = u8"\u2663";
string diamond = u8"\u2666";

// memindahkan kursor
void moveCursor(int x, int y) {
    COORD coord;
    coord.X = x;
    coord.Y = y;
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
}

// print dengan delay di tiap karakter
void delayPrint(const string& text, int delayMs = 100) {
    for (int i = 0; i < text.size(); i++) {
        char c = text[i];
        cout << c << flush;
        Sleep(delayMs);
    }
}

// hilangkan kursor
void hideCursor() {
    HANDLE out = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_CURSOR_INFO cursorInfo;
    GetConsoleCursorInfo(out, &cursorInfo);
    cursorInfo.bVisible = false;
    SetConsoleCursorInfo(out, &cursorInfo);
}

// bersihkan layar
void clearScreen() {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

// ambil tanggal hari ini
string getDate(){
    auto now = chrono::system_clock::now();
    time_t t = chrono::system_clock::to_time_t(now);
    tm* localTime = localtime(&t);
    char buffer[11];
    strftime(buffer, sizeof(buffer), "%d-%m-%Y", localTime);
    return buffer;
}

// merge sort
void merging(vector<Record>& data, int left, int mid, int right) {
    int n1 = mid - left + 1;
    int n2 = right - mid;

    vector<Record> L;
    vector<Record> R;

    for (int i = 0; i < n1; i++) L.push_back(data[left + i]);
    for (int j = 0; j < n2; j++) R.push_back(data[mid + 1 + j]);

    int i = 0, j = 0, k = left;
    while (i < n1 && j < n2) {
        if (L[i].score >= R[j].score) {
            data[k++] = L[i++];
        } else {
            data[k++] = R[j++];
        }
    }

    while (i < n1) data[k++] = L[i++];
    while (j < n2) data[k++] = R[j++];
}

void mergeSort(vector<Record>& data, int left, int right) {
    if (left < right) {
        int mid = left + (right - left) / 2;
        mergeSort(data, left, mid);
        mergeSort(data, mid + 1, right);
        merging(data, left, mid, right);
    }
}

// print records (3 skor tertinggi)
void printRecords(vector<Record>& records){
    int size = 3;
    if(records.size() < 3) size = records.size();

    for (int i = 0; i < size; i++){
        delayPrint("#" + to_string(i+1), 75); cout << endl;
        delayPrint(records[i].nickname, 25); cout << endl;
        delayPrint(to_string(records[i].score) , 25); cout << endl;
        delayPrint(records[i].tanggal, 25); cout << endl << endl;
        Sleep(500);
    }
}

// baca data records dari dataRecords.txt
void readRecords(vector<Record>& records){
    fstream file("src/dataRecords.txt");
    if (!file.is_open()) {
        cerr << "File tidak bisa dibuka!" << endl;
        return;
    }
    Record r;
    string scoreStr;
    while (getline(file, r.nickname) && getline(file, scoreStr) && getline(file, r.tanggal)) {
        r.score = stoi(scoreStr);   
        records.push_back(r);
    }
    file.close();
}

// tulis data bertipe Record ke dataRecords.txt
void writeRecords(const Record& records) {
    fstream file("src/dataRecords.txt", ios::app); 
    if (!file.is_open()) {
        cerr << "File tidak bisa dibuka!" << endl;
        return;
    }
    file << records.nickname << endl << records.score << endl << records.tanggal << endl << endl;
}

// class game
class Game{
    private:
        int life;
        int score;
        int baseNumberOfCards = 3;
        int combo = 0;
        int highestCombo = 0;
        int comboSpade = 0; // Counter untuk spade beruntun
        int totalCardsSpawned = 0; // Counter total kartu yang sudah spawn
        bool faseHati = false;
        int hitHeart = 0;
        
        // Atribut untuk fase Diamond Kill
        bool faseDiamondKill = false;
        int diamondKillRoundsRemaining = 0;
        int killedLane = -1; // Lane yang dibunuh (-1 = tidak ada)
        int diamondKillTriggerPoint = 0; // Kapan fase diamond kill akan trigger
        
        char inputKeySpade = '1', inputKeyHeart = '2', inputKeyClub = '3', inputKeyDiamond = '4';
        const int inputLine = 18; 
        const int perfectZone = 1; 

        struct fallingCard {
            string symbol;
            char inputKey;
            int lane; 
            int coordinateY;
            int coordinateX;
            bool isHit;
        };

    public:
        Game(int life, int score){ 
            this->life = life;
            this->score = score;
            // Set trigger point acak untuk diamond kill (60-80 kartu)
            this->diamondKillTriggerPoint = 60 + rand() % 21;
        }
        
        int getLife(){ 
            return life;
        }
        
        int getScore(){
            return score;
        }
        
        void setScore(int score){
            this->score = score;
        }
        
        void setLife(int life = 3){
            this->life = life;
            this->score = 0;
            this->combo = 0;
            this->highestCombo = 0;
            this->comboSpade = 0;
            this->totalCardsSpawned = 0;
            this->faseHati = false;
            this->baseNumberOfCards = 3;
            this->faseDiamondKill = false;
            this->diamondKillRoundsRemaining = 0;
            this->killedLane = -1;
            this->diamondKillTriggerPoint = 60 + rand() % 21;
        }
        
        void setInput(char inputKeySpade, char inputKeyHeart, char inputKeyClub, char inputKeyDiamond){
            this->inputKeySpade = inputKeySpade;
            this->inputKeyHeart = inputKeyHeart;
            this->inputKeyClub = inputKeyClub;
            this->inputKeyDiamond = inputKeyDiamond;
        }
        
        char getInputKeySpade(){ return inputKeySpade; }
        char getInputKeyHeart(){ return inputKeyHeart; }
        char getInputKeyClub(){ return inputKeyClub; }
        char getInputKeyDiamond(){ return inputKeyDiamond; }

        int getHighestCombo(){
            return highestCombo;
        }

        void drawHeader(){ 
            clearScreen();
            cout << "life: " << life << " | score: " << score << " | Combo: " << combo;
            if (faseDiamondKill) {
                cout << " | >>> DIAMOND KILL AKTIF! <<<";
            } else if (faseHati) {
                cout << " | FASE HATI AKTIF!";
            } else {
                cout << " | Spade Combo: " << comboSpade << "/3 | Cards: " << totalCardsSpawned;
            }
            cout << endl;
            cout << "=========================================" << endl;
            
            // Tampilkan info killed lane jika ada
            if (killedLane >= 0 && diamondKillRoundsRemaining > 0) {
                cout << "PERHATIAN: ";
                switch(killedLane) {
                    case 0: cout << spade << " SPADE"; break;
                    case 1: cout << heart << " HEART"; break;
                    case 2: cout << club << " CLUB"; break;
                }
                cout << " TERBUNUH! Tekan [" << inputKeyDiamond << "] untuk kartu ini!" << endl;
                cout << "Rounds tersisa: " << diamondKillRoundsRemaining << endl;
                cout << "=========================================" << endl;
            }
        }
        
        void drawInputLine(){
            moveCursor(0, inputLine);
            cout << "=========================================";
            moveCursor(0, inputLine + 1);
            cout << "    |  [" << inputKeySpade << "]  |  [" << inputKeyHeart << "]  |  [" << inputKeyClub << "]  |  [" << inputKeyDiamond << "]  |";
            moveCursor(0, inputLine + 2);
            cout << "=========================================";
        }

        bool randomCard(){
            // Cek apakah Diamond Kill harus diaktifkan
            if (totalCardsSpawned >= diamondKillTriggerPoint && !faseDiamondKill && diamondKillRoundsRemaining == 0) {
                faseDiamondKill = true;
                diamondKillRoundsRemaining = 5;
                
                // Pilih lane secara acak untuk dibunuh (exclude diamond)
                // Lane 0=spade, 1=heart, 2=club
                killedLane = rand() % 3;
            }
            
            // Kurangi counter rounds jika fase diamond kill aktif
            if (faseDiamondKill && diamondKillRoundsRemaining > 0) {
                diamondKillRoundsRemaining--;
                if (diamondKillRoundsRemaining == 0) {
                    faseDiamondKill = false;
                    killedLane = -1;
                }
            }
            
            vector<fallingCard> cards;

            int numCards = baseNumberOfCards + rand() % baseNumberOfCards;
            
            // Jika sudah 35 kartu dan fase hati belum terbuka, paksa spawn 3 spade beruntun
            bool forceSpade = (totalCardsSpawned >= 35 && !faseHati);
            
            for(int i = 0; i < numCards; i++){
                fallingCard card;
                
                // Force spawn spade jika kondisi terpenuhi
                if (forceSpade && i < 3) {
                    card.lane = 0; // Spade lane
                } 
                // Jika fase hati belum terbuka, hanya spawn 3 lane (spade, club, diamond)
                else if (!faseHati) {
                    int r = rand() % 3;
                    card.lane = (r >= 1) ? r + 1 : r; // 0, 2, 3 (skip lane 1 = heart)
                } 
                // Fase hati sudah terbuka, spawn semua lane
                else {
                    card.lane = rand() % 4; // 0-3 semua lane
                }
                
                card.coordinateY = -5 - (i * 4);
                card.coordinateX = card.lane * 8 + 8;
                card.isHit = false;
                
                // Set symbol dan inputKey berdasarkan lane
                // KECUALI jika lane tersebut sedang killed, maka inputKey = diamond
                switch(card.lane){
                    case 0: 
                        card.symbol = spade; 
                        card.inputKey = (killedLane == 0 && faseDiamondKill) ? inputKeyDiamond : inputKeySpade;
                        break;
                    case 1: 
                        card.symbol = heart; 
                        card.inputKey = (killedLane == 1 && faseDiamondKill) ? inputKeyDiamond : inputKeyHeart;
                        break;
                    case 2: 
                        card.symbol = club; 
                        card.inputKey = (killedLane == 2 && faseDiamondKill) ? inputKeyDiamond : inputKeyClub;
                        break;
                    case 3: 
                        card.symbol = diamond; 
                        card.inputKey = inputKeyDiamond;
                        break;
                }
                cards.push_back(card);
                totalCardsSpawned++; // Increment total kartu
            }
            
            string feedback = "";
            int feedbackFrame = 0;
            bool gameRunning = true;

            while(gameRunning){ 
                drawHeader();
                
                bool allCardsPassed = true;
                for (int i = 0; i < cards.size(); i++) {
                    if (!cards[i].isHit && cards[i].coordinateY <= inputLine + 3) {
                        allCardsPassed = false;

                        if (cards[i].coordinateY >= 2 && cards[i].coordinateY < inputLine + 3) {
                            moveCursor(cards[i].coordinateX, cards[i].coordinateY);
                            
                            // Tampilkan kartu dengan warna/indikator khusus jika killed
                            if (faseDiamondKill && cards[i].lane == killedLane) {
                                cout << "[" << cards[i].symbol << "]"; // Tandai dengan bracket
                            } else {
                                cout << cards[i].symbol;
                            }
                        }

                        cards[i].coordinateY++;

                        if (cards[i].coordinateY > inputLine + perfectZone + 1 && !cards[i].isHit) {
                            cards[i].isHit = true;
                            life--;
                            if(combo > highestCombo) highestCombo = combo;
                            combo = 0;
                            comboSpade = 0; // Reset spade combo saat miss
                            feedback = "MISS!";
                            feedbackFrame = 5;
                        }
                    }
                }
                
                drawInputLine();
                
                if(_kbhit()){
                    char input = _getch();
                    
                    for (int i = 0; i < cards.size(); i++) {
                        if (!cards[i].isHit && cards[i].inputKey == input) {
                            int distance = abs(cards[i].coordinateY - inputLine);

                            // PERFECT HIT: tepat di zona target
                            if (distance <= perfectZone) {
                                cards[i].isHit = true;
                                score += 10;
                                combo++;
                                
                                // Cek apakah input adalah spade (dan bukan killed)
                                if (input == inputKeySpade && !(faseDiamondKill && killedLane == 0)) {
                                    comboSpade++;
                                    if (comboSpade >= 3 && !faseHati) {
                                        faseHati = true;
                                        baseNumberOfCards = 4;
                                        feedback = "PERFECT! +10          >>> FASE HATI TERBUKA! <<<";
                                        feedbackFrame = 10;
                                    } else {
                                        feedback = "PERFECT! +10";
                                        feedbackFrame = 5;
                                    }
                                } else if (input == inputKeyHeart && !(faseDiamondKill && killedLane == 1)){
                                    hitHeart++;
                                    feedback = "PERFECT! +10";
                                    feedbackFrame = 5;
                                    if (hitHeart >= 7){
                                        feedback = "PERFECT! +10          >>> HEART IS BROKEN <<<";
                                        feedbackFrame = 5;
                                        faseHati = false;
                                    }
                                } else {
                                    if (!(input == inputKeySpade)) {
                                        comboSpade = 0; // Reset jika bukan spade
                                    }
                                    feedback = "PERFECT! +10";
                                    feedbackFrame = 5;
                                }
                                break;
                            }
                            // GOOD HIT: dekat zona target
                            else if (distance <= perfectZone + 2) {
                                cards[i].isHit = true;
                                score += 5;
                                combo++;
                                
                                // Cek apakah input adalah spade (dan bukan killed)
                                if (input == inputKeySpade && !(faseDiamondKill && killedLane == 0)) {
                                    comboSpade++;
                                    if (comboSpade >= 3 && !faseHati) {
                                        faseHati = true;
                                        baseNumberOfCards = 4;
                                        feedback = "GOOD! +5          >>> FASE HATI TERBUKA! <<<";
                                        feedbackFrame = 10;
                                    } else {
                                        feedback = "GOOD! +5";
                                        feedbackFrame = 5;
                                    }
                                } else if (input == inputKeyHeart && !(faseDiamondKill && killedLane == 1)){
                                    hitHeart++;
                                    feedback = "GOOD! +5";
                                    feedbackFrame = 5;
                                    if (hitHeart >= 7){
                                        feedback = "GOOD! +5          >>> HEART IS BROKEN <<<";
                                        feedbackFrame = 5;
                                        faseHati = false;
                                    }
                                } else {
                                    if (!(input == inputKeySpade)) {
                                        comboSpade = 0; // Reset jika bukan spade
                                    }
                                    feedback = "GOOD! +5";
                                    feedbackFrame = 5;
                                }
                                break;
                            }
                            // EARLY/LATE MISS: terlalu jauh dari zona target
                            else if (distance > perfectZone + 2) {
                                cards[i].isHit = true;
                                life--;
                                if(combo > highestCombo) highestCombo = combo;
                                combo = 0;
                                comboSpade = 0; // Reset spade combo saat miss
                                
                                // Bedakan terlalu awal vs terlalu lambat
                                if (cards[i].coordinateY < inputLine - perfectZone - 2) {
                                    feedback = "TOO EARLY!";
                                } else {
                                    feedback = "TOO LATE!";
                                }
                                feedbackFrame = 5;
                                break;
                            }
                        }
                    }
                }
                
                if(feedbackFrame > 0){
                    moveCursor(0, 23);
                    cout << feedback;
                    feedbackFrame--;
                }
                
                if(allCardsPassed){
                    gameRunning = false;
                }
                
                if(life <= 0){
                    return false;
                }
                
                Sleep(85);
            }
            
            Sleep(500);
            return true;
        }
};

class Menu{
    private:
        Game rhythmOfHearts = Game(3, 0);
    public:
        string sambutan1 = spade + " Selamat Datang Di Game Rythm Of Hearts " + spade;
        string sambutan2 = heart + " Selamat Datang Di Game Rythm Of Hearts " + heart;
        string sambutan3 = club + " Selamat Datang Di Game Rythm Of Hearts " + club;
        string sambutan4 = diamond + " Selamat Datang Di Game Rythm Of Hearts " + diamond;
        
        Menu(){
            pilihan();
        }
        
        void pilihan(){
            delayPrint("Pilih: ", 15);
            cout << endl;
            char c;
            c = _getch();
            switch(c){
                case '1':clearScreen(); tampilkanPeraturan(); break;
                case '2':clearScreen(); newGame(); break;
                case '3':clearScreen(); tampilkanRecord(); break;
                case '4':clearScreen(); pengaturan(); break;
                case '5':cout << endl; delayPrint("Terimakasih sudah bermain :) ", 25); break;
                default: delayPrint("Masukkan input dengan benar!", 15); c = _getch(); clearScreen(); 
            }
            while(c != '5'){
                cout << "(1) Peraturan" << endl;
                cout << "(2) New game" << endl;
                cout << "(3) Record" << endl;
                cout << "(4) Pengaturan" << endl;
                cout << "(5) Exit" << endl;
                delayPrint("Pilih: ", 15);
                cout << endl;
                c = _getch();
                switch(c){
                    case '1':clearScreen(); tampilkanPeraturan(); break;
                    case '2':clearScreen(); newGame(); break;
                    case '3':clearScreen(); tampilkanRecord(); break;
                    case '4':clearScreen(); pengaturan(); break;
                    case '5':cout << endl; delayPrint("Terimakasih sudah bermain :) ", 25); break;
                    default: delayPrint("Masukkan input dengan benar!", 15); c = _getch(); clearScreen(); 
                }
            }
        }
        
        void tampilkanRecord(){
            vector<Record> records;
            readRecords(records);
            mergeSort(records, 0, records.size()-1);
            printRecords(records);
            cout << "Tekan tombol apa saja untuk keluar";
            char c;
            c = _getch();
            clearScreen();
        }
        
        void newGame(){
            Record records;
            string nickname;
            char c;

            // game loop
            while(rhythmOfHearts.getLife() > 0){
                bool continueGame = rhythmOfHearts.randomCard();
                if(!continueGame) break;
            }
            
            int score = rhythmOfHearts.getScore();
            int highestCombo = rhythmOfHearts.getHighestCombo();
            string strScore = to_string(score); 
            string strCombo = to_string(highestCombo); 
            clearScreen();
            delayPrint("GAME OVER!", 100);
            c = _getch();
            cout << endl << endl;
            delayPrint("Score = ", 50);
            Sleep(500);
            delayPrint(strScore, 50); 
            Sleep(500);
            cout << endl;
            delayPrint("Highest combo = ", 50); 
            Sleep(500);
            delayPrint(strCombo, 50); 
            Sleep(500);
            cout << endl;
            delayPrint("Additional score = 0", 50); 
            Sleep(500);
            int addScore = 0;
            
            for (int i = highestCombo; i >= 0; i--){
                clearScreen();
                cout << "GAME OVER!" << endl << endl;
                cout << "Score = " << score << endl;
                cout << "Highest combo = " << i << endl;
                cout << "Additional score " << addScore << endl;
                addScore += 5;
                Sleep(25);
            }
            Sleep(500);
            for (int i = addScore; i >= 0; i-=5){
                clearScreen();
                cout << "GAME OVER!" << endl << endl;
                cout << "Score = " << score << endl;
                score += 5;
                cout << endl;
                cout << "Additional score " << i << endl;
                Sleep(25);
            }
            clearScreen();
            cout << "GAME OVER!" << endl << endl;
            cout << "Score = " << score << endl << endl;

            records.nickname = nickname;
            records.score = score;
            records.tanggal = getDate();

            delayPrint("Simpan Record? (Y/N)", 15);
            cout << endl;
            c = _getch();
            switch (c){
                case 'y':
                case 'Y':
                    writeRecords(records); 
                    delayPrint("Record tersimpan! ", 25); 
                    cout << endl;
                    break;
                case 'n':
                case 'N':
                    cout << endl << endl;
                    break;
            }
            cout << "Tekan tombol apa saja untuk keluar";
            c = _getch();
            clearScreen();
            rhythmOfHearts.setLife();
        }
        
        void tampilkanPeraturan(){
            char c;
            delayPrint("(1) Aturan Dasar", 25);
            cout << endl;
            Sleep(100);
            delayPrint("(2) Fase", 25);
            cout << endl;
            Sleep(100);
            delayPrint("(3) Kembali", 25);
            cout << endl;
            delayPrint("Pilih:", 15);
            cout << endl;
            c = _getch();
            switch(c){
                case '1':clearScreen(); aturanDasar(); clearScreen(); break;
                case '2':clearScreen(); break;
                case '3':clearScreen(); break;
            }
            while(c != '3'){
                cout << "(1) Aturan Dasar" << endl;
                cout << "(2) Fase" << endl;
                cout << "(3) Kembali" << endl;
                delayPrint("Pilih: ", 15);
                cout << endl;
                c = _getch();
                switch(c){
                    case '1':clearScreen(); aturanDasar(); clearScreen(); break;
                    case '2':clearScreen(); break;
                    case '3':clearScreen(); break;
                }
            }
        }
        
        void aturanDasar(){
            char c;
            delayPrint("=== ATURAN DASAR RHYTHM OF HEARTS ===", 25);
            c = _getch();
            cout << endl << endl;
            delayPrint("Ini adalah game rhythm!", 25);
            c = _getch();
            cout << endl;
            delayPrint("Kartu akan jatuh dari atas dalam 4 lane berbeda", 25);
            c = _getch();
            cout << endl << endl;
            delayPrint("4 Simbol Kartu: " + spade + " " + heart + " " + club + " " + diamond, 25);
            c = _getch();
            cout << endl << endl;
            
            string teksPenjelasan = "Input key: ";
            teksPenjelasan += rhythmOfHearts.getInputKeySpade();
            teksPenjelasan += "(" + spade + ") | " + rhythmOfHearts.getInputKeyHeart() + "(" + heart + ") | " + rhythmOfHearts.getInputKeyClub() + "(" + club + ") | " + rhythmOfHearts.getInputKeyDiamond() + "(" + diamond + ")";
            delayPrint(teksPenjelasan, 25);
            delayPrint("        (Dapat diubah di pengaturan)", 25);
            c = _getch();
            cout << endl << endl;
            
            delayPrint("Tekan tombol yang sesuai TEPAT saat kartu mencapai garis target!", 25);
            c = _getch();
            cout << endl << endl;
            delayPrint("PERFECT HIT (tepat di target) = +10 poin", 25);
            c = _getch();
            cout << endl;
            delayPrint("GOOD HIT (dekat target) = +5 poin", 25);
            c = _getch();
            cout << endl;
            delayPrint("MISS (terlambat/tidak menekan) = -1 nyawa", 25);
            c = _getch();
            cout << endl << endl;
            delayPrint("SPECIAL: Hit 3 Spade beruntun untuk membuka FASE HATI!", 25);
            c = _getch();
            cout << endl << endl;
            delayPrint("Kamu punya 3 nyawa. Good luck!", 25);
            c = _getch();
            cout << endl << endl;
            cout << "Tekan tombol apa saja untuk keluar";
            c = _getch();
        }
        
        void pengaturan(){
            delayPrint("Input key saat ini: ", 15);
            Sleep(500);
            cout << endl;
            string teksInput = "";
            teksInput += spade + ": " + rhythmOfHearts.getInputKeySpade() + "   " + heart + ": " + rhythmOfHearts.getInputKeyHeart() + "   " + club + ": " + rhythmOfHearts.getInputKeyClub() + "   " + diamond + ": " + rhythmOfHearts.getInputKeyDiamond();
            delayPrint(teksInput, 15);
            Sleep(750);
            cout << endl << endl;
            delayPrint("Ingin mengubah input key? (Y/N)", 15);
            char c;
            char inputKeySpade, inputKeyHeart, inputKeyClub, inputKeyDiamond;
            c = _getch();
            cout << endl;
            switch (c){
                case 'y':
                case 'Y':
                    cout << endl; delayPrint("Masukkan input untuk Spade: ", 15); inputKeySpade = _getch(); cout << inputKeySpade;
                
                    do{
                        cout << endl; 
                        delayPrint("Masukkan input untuk Heart: ", 15); inputKeyHeart = _getch(); cout << inputKeyHeart;
                        if(inputKeyHeart == inputKeySpade) {cout << endl; delayPrint("Masukkan input yang berbeda! ", 15);}
                    }while(inputKeyHeart == inputKeySpade);

                    do{
                        cout << endl; 
                        delayPrint("Masukkan input untuk Club: ", 15); inputKeyClub = _getch(); cout << inputKeyClub;
                        if(inputKeyClub == inputKeySpade || inputKeyClub == inputKeyHeart) {cout << endl; delayPrint("Masukkan input yang berbeda! ", 15);}
                    }while(inputKeyClub == inputKeySpade || inputKeyClub == inputKeyHeart);
                    
                    do{
                        cout << endl; 
                        delayPrint("Masukkan input untuk Diamond: ", 15); inputKeyDiamond = _getch(); cout << inputKeyDiamond;
                        if(inputKeyDiamond == inputKeySpade || inputKeyDiamond == inputKeyHeart || inputKeyDiamond == inputKeyClub) {cout << endl; delayPrint("Masukkan input yang berbeda! ", 15);}
                    }while(inputKeyDiamond == inputKeySpade || inputKeyDiamond == inputKeyHeart || inputKeyDiamond == inputKeyClub);

                    cout << endl;
                    Sleep(500);
                    clearScreen();
                    teksInput = "";
                    teksInput += spade + ": " + inputKeySpade + "   " + heart + ": " + inputKeyHeart + "   " + club + ": " + inputKeyClub + "   " + diamond + ": " + inputKeyDiamond;
                    delayPrint(teksInput, 15);
                    Sleep(500);
                    cout << endl << endl; delayPrint("Yakin ingin menyimpan? (Y/N)", 15);
                    c = _getch();
                    switch (c){
                        case 'y':
                        case 'Y':
                            rhythmOfHearts.setInput(inputKeySpade, inputKeyHeart, inputKeyClub, inputKeyDiamond); 
                            cout << endl << endl; 
                            delayPrint("Pengaturan input key berhasil tersimpan!", 15); 
                            break;
                        case 'n':
                        case 'N':
                            break;
                    }
                    break;
                case 'n':
                case 'N':
                    break;
            }
            cout << endl << endl;
            cout << "Tekan tombol apa saja untuk keluar";
            c = _getch();
            clearScreen();
        }
};

int main(int argc, char const *argv[])
{
    srand(time(0));
    hideCursor();  
    clearScreen();
    SetConsoleOutputCP(CP_UTF8);
    Menu tampilanAwal;
}