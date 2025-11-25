#include <iostream>
#include <cstdlib>
#include <ctime>
#include <conio.h> 
#include <windows.h>
#include <chrono>
#include <thread>
#include <fstream>
#include <vector>
#include <algorithm>
#include <string>
#include <iomanip>

using namespace std;

struct Record {
    string nickname;
    int skor;
    string tanggal;
};

string spade = u8"\u2660";
string heart = u8"\u2665";
string club = u8"\u2663";
string diamond = u8"\u2666";

// Method global

void pindahKursor(int x, int y) {
    COORD coord;
    coord.X = x;
    coord.Y = y;
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
}

void delayPrint(const string& text, int delayMs = 100) { // print dengan delay
        for (size_t i = 0; i < text.size(); i++) {
            char c = text[i];
            cout << c << flush;
            Sleep(delayMs);
        }
}
void hilangkanKursor() {
    HANDLE out = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_CURSOR_INFO cursorInfo;
    GetConsoleCursorInfo(out, &cursorInfo);
    cursorInfo.bVisible = false;
    SetConsoleCursorInfo(out, &cursorInfo);
}

void layarBersih() {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

string getTanggal(){
    auto now = chrono::system_clock::now();
    time_t t = chrono::system_clock::to_time_t(now);
    tm* localTime = localtime(&t);
    char buffer[11]; // "dd-mm-yyyy" = 10 + null terminator
    strftime(buffer, sizeof(buffer), "%d-%m-%Y", localTime);
    return buffer;
}

// Merge Sort
void array_merge(vector<Record>& data, int left, int mid, int right) { // Merging
    int n1 = mid - left + 1;
    int n2 = right - mid;

    vector<Record> L;
    vector<Record> R;

    for (int i = 0; i < n1; i++) L.push_back(data[left + i]);
    for (int j = 0; j < n2; j++) R.push_back(data[mid + 1 + j]);

    int i = 0, j = 0, k = left;
    while (i < n1 && j < n2) {
        if (L[i].skor >= R[j].skor) { // descending
            data[k++] = L[i++];
        } else {
            data[k++] = R[j++];
        }
    }

    while (i < n1) data[k++] = L[i++];
    while (j < n2) data[k++] = R[j++];
}

void merge_sort(vector<Record>& data, int left, int right) { // Rekursif
    if (left < right) {
        int mid = left + (right - left) / 2;
        merge_sort(data, left, mid);
        merge_sort(data, mid + 1, right);
        array_merge(data, left, mid, right);
    }
}

void printVector(vector<Record>& data){
    int size = 3;
    if(data.size() < 3) size = data.size();

    for (size_t i = 0; i < size; i++){
        delayPrint("#" + to_string(i+1), 75);
        delayPrint(data[i].nickname, 25); cout << endl;
        delayPrint(to_string(data[i].skor) , 25); cout << endl;
        delayPrint(data[i].tanggal, 25); cout << endl << endl;
        Sleep(500);
    }
}

void readRecords(vector<Record>& records){
        fstream file("src/dataRecords.txt");
        if (!file.is_open()) {
            cerr << "File tidak bisa dibuka!" << endl;
            return;
        }
        Record r;
        string skorStr;
        while (getline(file, r.nickname) && getline(file, skorStr) && getline(file, r.tanggal)) {
            r.skor = stoi(skorStr);   
            records.push_back(r);
        }
        file.close();
}

void writeRecords(const Record& records) {
    fstream file("src/dataRecords.txt", ios::app); 
    if (!file.is_open()) {
        cerr << "File tidak bisa dibuka!" << endl;
        return;
    }
    file << records.nickname << endl << records.skor << endl << records.tanggal << endl << endl;
}


class Game{
    private:
        int nyawa;
        int waktu;
        int skor;
        int jumlahKartu = 3;
        int jumlahRandom = 0;
        bool faseHati, faseWajik, faseKeriting, faseSekop;
        char inSpade = '1', inHeart = '2', inClub = '3', inDiamond = '4';
        string setKartu;
        const int TINGGI_LAYAR = 20; // tinggi area animasi
        const int TARGET_LINE = 18; // garis target untuk input
        const int PERFECT_ZONE = 1; // zona perfect (±1 baris dari target)

        struct FallingCard {
            string symbol;
            char inputKey;
            int posY;
            int lane; // kolom/lane kartu (0-3)
            bool hit;
        };

    public:
        Game(int nyawa, int waktu, int skor){ // konstruktor
            this->nyawa = nyawa;
            this->waktu = waktu;
            this->skor = skor;
        }
        int getNyawa(){
            return nyawa;
        }
        int getSkor(){
            return skor;
        }
        void setNyawa(int nyawa = 3){
            this->nyawa = nyawa;
        }
        void setInput(char inSpade, char inHeart, char inClub, char inDiamond){
            this->inSpade = inSpade;
            this->inHeart = inHeart;
            this->inClub = inClub;
            this->inDiamond = inDiamond;
        }
        char getInSpade(){ return inSpade; }
        char getInHeart(){ return inHeart; }
        char getInClub(){ return inClub; }
        char getInDiamond(){ return inDiamond; }

        void statusFaseHati(int combo){
            if(combo >= 3 && !faseHati){
                pindahKursor(0, 22);
                cout << ">>> FASE HATI TERBUKA! <<<" << endl;
                faseHati = true;
                jumlahKartu = 4;
            }
        }

        void drawTargetZone(){
            // Gambar zona target
            pindahKursor(0, TARGET_LINE);
            cout << "========================================";
            pindahKursor(0, TARGET_LINE + 1);
            cout << "|  [" << inSpade << "]  |  [" << inHeart << "]  |  [" << inClub << "]  |  [" << inDiamond << "]  | <- TEKAN DISINI!";
            pindahKursor(0, TARGET_LINE + 2);
            cout << "========================================";
        }

        bool kartuRandom(){
            srand(static_cast<unsigned>(time(0) + rand()));
            vector<FallingCard> cards;
            
            // Generate 3-5 kartu yang akan jatuh
            int numCards = 3 + rand() % 3; // 3-5 kartu
            
            for(int i = 0; i < numCards; i++){
                FallingCard card;
                card.posY = -5 - (i * 4); // spacing antar kartu
                card.lane = rand() % 4; // lane 0-3
                card.hit = false;
                
                // Tentukan simbol dan key berdasarkan lane
                switch(card.lane){
                    case 0: card.symbol = spade; card.inputKey = inSpade; break;
                    case 1: card.symbol = heart; card.inputKey = inHeart; break;
                    case 2: card.symbol = club; card.inputKey = inClub; break;
                    case 3: card.symbol = diamond; card.inputKey = inDiamond; break;
                }
                
                cards.push_back(card);
            }

            int combo = 0;
            string feedback = "";
            int feedbackTimer = 0;
            
            // Game loop untuk kartu yang jatuh
            bool gameRunning = true;
            while(gameRunning){
                layarBersih();
                
                // Header info
                cout << "Nyawa: " << nyawa << " | Skor: " << skor << " | Combo: " << combo << endl;
                cout << "========================================" << endl;
                
                // Update dan gambar kartu
                bool allCardsPassed = true;
                for(auto& card : cards){
                    if(!card.hit && card.posY <= TARGET_LINE + 3){
                        allCardsPassed = false;
                        
                        if(card.posY >= 2 && card.posY < TARGET_LINE + 3){
                            int xPos = 5 + (card.lane * 10);
                            pindahKursor(xPos, card.posY);
                            cout << card.symbol;
                        }
                        
                        card.posY++;
                        
                        // Kartu melewati target zone tanpa di-hit
                        if(card.posY > TARGET_LINE + PERFECT_ZONE + 1 && !card.hit){
                            card.hit = true;
                            nyawa--;
                            combo = 0;
                            feedback = "MISS!";
                            feedbackTimer = 10;
                        }
                    }
                }
                
                // Gambar target zone
                drawTargetZone();
                
                // Tampilkan feedback
                if(feedbackTimer > 0){
                    pindahKursor(0, 23);
                    cout << feedback;
                    feedbackTimer--;
                }
                
                // Check input
                if(_kbhit()){
                    char input = _getch();
                    
                    // Cari kartu yang bisa di-hit
                    for(auto& card : cards){
                        if(!card.hit && card.inputKey == input){
                            // Cek apakah dalam zona hit
                            int distance = abs(card.posY - TARGET_LINE);
                            
                            if(distance <= PERFECT_ZONE){
                                // PERFECT HIT!
                                card.hit = true;
                                skor += 10;
                                combo++;
                                feedback = "PERFECT! +10";
                                feedbackTimer = 10;
                                statusFaseHati(combo);
                                break;
                            }
                            else if(distance <= PERFECT_ZONE + 2){
                                // GOOD HIT
                                card.hit = true;
                                skor += 5;
                                combo++;
                                feedback = "GOOD! +5";
                                feedbackTimer = 10;
                                break;
                            }
                        }
                    }
                }
                
                // Check jika semua kartu sudah lewat
                if(allCardsPassed){
                    gameRunning = false;
                }
                
                // Check nyawa
                if(nyawa <= 0){
                    return false;
                }
                
                Sleep(150); // Frame rate
            }
            
            Sleep(500);
            return true;
        }
        
        bool inputKartu(){
            return true; // Tidak digunakan lagi karena input sekarang realtime
        }

};

class Menu{
    private:
        Game rythmOfHearts = Game(3, 20, 0);
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
                case '1':layarBersih(); tampilkanPeraturan(); break;
                case '2':layarBersih(); newGame(); break;
                case '3':layarBersih(); tampilkanRecord(); break;
                case '4':layarBersih(); pengaturan(); break;
                case '5':cout << endl; delayPrint("Terimakasih sudah bermain :) ", 25); break;
                default: delayPrint("Masukkan input dengan benar!", 15); c = _getch(); layarBersih(); 
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
                    case '1':tampilkanPeraturan(); break;
                    case '2':layarBersih(); newGame(); break;
                    case '3':layarBersih(); tampilkanRecord(); break;
                    case '4':layarBersih(); pengaturan(); break;
                    case '5':cout << endl; delayPrint("Terimakasih sudah bermain :) ", 25); break;
                    default: delayPrint("Masukkan input dengan benar!", 15); c = _getch(); layarBersih(); 
                }
            }
            
        }
        void tampilkanRecord(){
            vector<Record> records;
            readRecords(records);
            merge_sort(records, 0, records.size()-1);
            printVector(records);
            cout << "Tekan tombol apa saja untuk keluar";
            char c;
            c = _getch();
            layarBersih();
        }
        void newGame(){
            Record records;
            string nickname;

            delayPrint("Masukkan nickname: ", 25);
            getline(cin, nickname);
            layarBersih();
            delayPrint(nickname + ", ", 25);
            Sleep(750);
            delayPrint("Bersiaplah! ", 25);
            Sleep(750);
            delayPrint(". . .", 250);
            Sleep(1500);
            layarBersih();

            // Game loop
            while(rythmOfHearts.getNyawa() > 0){
                bool continueGame = rythmOfHearts.kartuRandom();
                if(!continueGame) break;
            }

            records.nickname = nickname;
            records.skor = rythmOfHearts.getSkor();
            records.tanggal = getTanggal();

            layarBersih();
            cout << endl;
            delayPrint("GAME OVER!", 100);
            cout << endl << endl;
            string strSkor = to_string(rythmOfHearts.getSkor()); 
            delayPrint("Skor Akhir = ", 50);
            Sleep(500);
            delayPrint(strSkor, 50); 
            Sleep(500);
            cout << endl << endl; 

            delayPrint("Simpan Record? (Y/N)", 15);
            cout << endl;
            char c;
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
                    delayPrint("E M U A C H", 250); 
                    cout << endl << endl;
                    break;
            }
            cout << "Tekan tombol apa saja untuk keluar";
            c = _getch();
            layarBersih();
            rythmOfHearts.setNyawa();
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
                case '1':layarBersih(); aturanDasar(); layarBersih(); break;
                case '2':layarBersih(); break;
                case '3':layarBersih(); break;
            }
            while(c != '3'){
                cout << "(1) Aturan Dasar" << endl;
                cout << "(2) Fase" << endl;
                cout << "(3) Kembali" << endl;
                delayPrint("Pilih: ", 15);
                cout << endl;
                c = _getch();
                switch(c){
                    case '1':layarBersih(); aturanDasar(); layarBersih(); break;
                    case '2':layarBersih(); break;
                    case '3':layarBersih(); break;
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
            
            string teksPenjelasan = "Kontrol: ";
            teksPenjelasan += rythmOfHearts.getInSpade() + "(" + spade + ") | ";
            teksPenjelasan += rythmOfHearts.getInHeart() + "(" + heart + ") | ";
            teksPenjelasan += rythmOfHearts.getInClub() + "(" + club + ") | ";
            teksPenjelasan += rythmOfHearts.getInDiamond() + "(" + diamond + ")";
            delayPrint(teksPenjelasan, 25);
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
            delayPrint("Combo 3x berturut-turut membuka FASE HATI!", 25);
            c = _getch();
            cout << endl << endl;
            delayPrint("Kamu punya 3 nyawa. Good luck!", 25);
            c = _getch();
            cout << endl << endl;
            cout << "Tekan tombol apa saja untuk keluar";
            c = _getch();
        }
        void pengaturan(){
            delayPrint("Input saat ini: ", 15);
            Sleep(500);
            cout << endl;
            string teksInput = "";
            teksInput += spade + ": " + rythmOfHearts.getInSpade() + "   " + heart + ": " + rythmOfHearts.getInHeart() + "   " + club + ": " + rythmOfHearts.getInClub() + "   " + diamond + ": " + rythmOfHearts.getInDiamond();
            delayPrint(teksInput, 15);
            Sleep(750);
            cout << endl << endl;
            delayPrint("Ingin mengubah input? (Y/N)", 15);
            char c;
            char inSpade, inHeart, inClub, inDiamond;
            c = _getch();
            cout << endl;
            switch (c){
                case 'y':
                case 'Y':
                    cout << endl; delayPrint("Masukkan input untuk Spade: ", 15); inSpade = _getch(); cout << inSpade;
                
                    do{
                        cout << endl; 
                        delayPrint("Masukkan input untuk Heart: ", 15); inHeart = _getch(); cout << inHeart;
                        if(inHeart == inSpade) {cout << endl; delayPrint("Masukkan input yang berbeda! ", 15);}
                    }while(inHeart == inSpade);

                    do{
                        cout << endl; 
                        delayPrint("Masukkan input untuk Club: ", 15); inClub = _getch(); cout << inClub;
                        if(inClub == inSpade || inClub == inHeart) {cout << endl; delayPrint("Masukkan input yang berbeda! ", 15);}
                    }while(inClub == inSpade || inClub == inHeart);
                    
                    do{
                        cout << endl; 
                        delayPrint("Masukkan input untuk Diamond: ", 15); inDiamond = _getch(); cout << inDiamond;
                        if(inDiamond == inSpade || inDiamond == inHeart || inDiamond == inClub) {cout << endl; delayPrint("Masukkan input yang berbeda! ", 15);}
                    }while(inDiamond == inSpade || inDiamond == inHeart || inDiamond == inClub);

                    cout << endl;
                    Sleep(500);
                    layarBersih();
                    teksInput = "";
                    teksInput += spade + ": " + inSpade + "   " + heart + ": " + inHeart + "   " + club + ": " + inClub + "   " + diamond + ": " + inDiamond;
                    delayPrint(teksInput, 15);
                    Sleep(500);
                    cout << endl << endl; delayPrint("Yakin ingin menyimpan? (Y/N)", 15);
                    c = _getch();
                    switch (c){
                        case 'y':
                        case 'Y':
                            rythmOfHearts.setInput(inSpade, inHeart, inClub, inDiamond); 
                            cout << endl << endl; 
                            delayPrint("Pengaturan input berhasil tersimpan!", 15); 
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
            layarBersih();
        }

};

int main(int argc, char const *argv[])
{
    hilangkanKursor();  
    layarBersih();
    SetConsoleOutputCP(CP_UTF8);
    Menu tampilanAwal;
}