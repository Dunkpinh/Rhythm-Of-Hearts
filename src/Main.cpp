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
void delayPrint(const string& text, int delayMs = 100) { // print dengan delay
        for (size_t i = 0; i < text.size(); i++) {
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
    char buffer[11]; // "dd-mm-yyyy" = 10 + null terminator
    strftime(buffer, sizeof(buffer), "%d-%m-%Y", localTime);
    return buffer;
}

// merge sort
// digunakan untuk mengurutkan data records secara menurun 
void merging(vector<Record>& data, int left, int mid, int right) { // merging
    int n1 = mid - left + 1;
    int n2 = right - mid;

    vector<Record> L;
    vector<Record> R;

    for (int i = 0; i < n1; i++) L.push_back(data[left + i]);
    for (int j = 0; j < n2; j++) R.push_back(data[mid + 1 + j]);

    int i = 0, j = 0, k = left;
    while (i < n1 && j < n2) {
        if (L[i].score >= R[j].score) { // descending
            data[k++] = L[i++];
        } else {
            data[k++] = R[j++];
        }
    }

    while (i < n1) data[k++] = L[i++];
    while (j < n2) data[k++] = R[j++];
}
void mergeSort(vector<Record>& data, int left, int right) { // rekursif
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

    for (size_t i = 0; i < size; i++){
        delayPrint("#" + to_string(i+1), 75); cout << endl;
        delayPrint(records[i].nickname, 25); cout << endl;
        delayPrint(to_string(records[i].score) , 25); cout << endl;
        delayPrint(records[i].tanggal, 25); cout << endl << endl;
        Sleep(500);
    }
}

// baca data records dari dataRecords.txt kemudian di-push ke vector
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
        int jumlahKartu = 3;
        int jumlahRandom = 0;
        int combo = 0;
        int highestCombo = 0;
        bool faseHati, faseWajik, faseKeriting, faseSekop;
        char inputKeySpade = '1', inputKeyHeart = '2', inputKeyClub = '3', inputKeyDiamond = '4';
        string setKartu; 
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

        getHighestCombo(){
            return highestCombo;
        }

        void statusFaseHati(int combo){
            if(combo >= 3 && !faseHati){
                moveCursor(0, 22);
                cout << ">>> FASE HATI TERBUKA! <<<" << endl;
                faseHati = true;
                jumlahKartu = 4;
            }
        }

        void drawHeader(){ 
            clearScreen();
            cout << "life: " << life << " | score: " << score << " | Combo: " << combo << endl;
            cout << "=========================================" << endl;
        }
        void drawInputLine(){
            moveCursor(0, inputLine);
            cout << "=========================================";
            moveCursor(0, inputLine + 1);
            cout << "    |  [" << inputKeySpade << "]  |  [" << inputKeyHeart << "]  |  [" << inputKeyClub << "]  |  [" << inputKeyDiamond << "]  |";
            moveCursor(0, inputLine + 2);
            cout << "=========================================";
        }

        // generate kartu random yang muncul sebanyak 3-5 di 4 lane berbeda, kemudian melakukan pengecekkan hit
        bool randomCard(){
            srand(time(0)); // seed
            vector<fallingCard> cards; // vector penampung kartu-kartu yang jatuh dalam 1 kali putaran
            
            int numCards = 3 + rand() % 3; // generate jumlah kartu yang akan jatuh (3-5)
            
            for(int i = 0; i < numCards; i++){
                fallingCard card; // penampung 1 kartu jatuh
                card.lane = rand() % 4; // generate di lane mana kartu bakal jatuh (0-3)
                card.coordinateY = -5 - (i * 4); // koordinat y kartu (mulai dari minus agar ada jeda)
                card.coordinateX = card.lane * 8 + 8; // koordinat x kartu (dengan jarak (8*lane + 8) untuk tiap kartu)
                card.isHit = false; // status kartu apakah sudah di hit
                
                // simbol dan input key berdasarkan lane (0,1,2,3) -> (♠,♥,♣,♦)
                switch(card.lane){
                    case 0: card.symbol = spade; card.inputKey = inputKeySpade; break;
                    case 1: card.symbol = heart; card.inputKey = inputKeyHeart; break;
                    case 2: card.symbol = club; card.inputKey = inputKeyClub; break;
                    case 3: card.symbol = diamond; card.inputKey = inputKeyDiamond; break;
                }
                
                cards.push_back(card); // push kartu ke vector
            }
            string feedback = ""; // penampung pesan PERFECT, HIT, MISS
            int feedbackFrame = 0; // frame kartu (simpelnya berapa lama dia muncul di layar)
            
            bool gameRunning = true; // status default, true jika game masih berjalan

            // game loop untuk kartu yang jatuh
            while(gameRunning){ 
                drawHeader();
                
                // Update dan gambar kartu
                bool allCardsPassed = true; // status apakah semua kartu dalam 1 putaran sudah selesai
                for (size_t i = 0; i < cards.size(); i++) { // dilakukan pengecekan untuk setiap kartu dalam 1 putaran
                    if (!cards[i].isHit && cards[i].coordinateY <= inputLine + 3) { // jika ada kartu yang belum di hit dan koordinat y -nya belum sampai bawah
                        allCardsPassed = false; // maka belum semua kartu selesai

                        // print berkala kartu jatuh ke bawah
                        if (cards[i].coordinateY >= 2 && cards[i].coordinateY < inputLine + 3) { // jika kartu berada di area koordinat y layar valid
                            moveCursor(cards[i].coordinateX, cards[i].coordinateY); // kursor dipindah ke koordinat pasangan (x, y)
                            cout << cards[i].symbol; // lalu di print simbolnya yang sudah ditentukan 
                        }

                        cards[i].coordinateY++; // increment koordinat y untuk semua kartu yang masih valid

                        // kartu melewati target zone tanpa di-Hit
                        if (cards[i].coordinateY > inputLine + perfectZone + 1 && !cards[i].isHit) { // jika kartu sudah melewati batas input dan tidak diinput
                            cards[i].isHit = true; // status is hit diubah agar proses pada kartu tersebut selesai
                            life--; // nyawa berkurang
                            if(combo > highestCombo) highestCombo = combo;
                            combo = 0; // combo direset
                            feedback = "MISS!";
                            feedbackFrame = 5;
                        }
                    }
                }
                
                drawInputLine();
                
                // cek input
                if(_kbhit()){ // cek apakah ada input
                    char input = _getch(); // ambil 1 karakter input tadi tanpa enter
                    
                    // cek kartu yang di-hit
                    for (size_t i = 0; i < cards.size(); i++) {
                        if (!cards[i].isHit && cards[i].inputKey == input) { // jika kartu belum di-hit dan input sama dengan inputKey dari kartu
                            // cek apakah dalam zona isHit
                            int distance = abs(cards[i].coordinateY - inputLine); // jarak absolut

                            if (distance <= perfectZone) { // perfect hit jika jarak kurang dari zona perfect 
                                cards[i].isHit = true; // status hit berubah dan proses selesai
                                score += 10; // skor bertambah
                                combo++; // combo bertambah
                                feedback = "PERFECT! +10";
                                feedbackFrame = 5;
                                statusFaseHati(combo);
                                break;
                            }
                            else if (distance <= perfectZone + 2) { // good hit jika jarak kurang dari zona perfect + 2
                                cards[i].isHit = true; // status hit berubah dan proses selesai
                                score += 5;
                                combo++;
                                feedback = "GOOD! +5";
                                feedbackFrame = 5;
                                break;
                            }
                        }
                    }
                }
                
                // tampilkan pesan
                if(feedbackFrame > 0){
                    moveCursor(0, 23);
                    cout << feedback;
                    feedbackFrame--; // jumlah frame berkurang tiap putaran
                }
                
                
                // cek apakah semua kartu sudah selesai
                if(allCardsPassed){ // kalau sudah selesai
                    gameRunning = false; // game harus berhenti berjalan (untuk 1 set, set selanjutnya dipanggil ulang di while dalam method newGame() di class Menu)
                }
                
                // cek nyawa
                if(life <= 0){
                    return false;
                }
                
                Sleep(150); // frame rate
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
            delayPrint(sambutan2, 50);
            Sleep(750);
            clearScreen();
            cout << sambutan3;
            Sleep(750);
            clearScreen();
            cout << sambutan4;
            Sleep(750);
            clearScreen();
            cout << sambutan1;
            Sleep(750);
            cout << endl;
            pilihan();
        }
        void pilihan(){
            delayPrint("(1) Peraturan", 25); 
            cout << endl;
            Sleep(250);
            delayPrint("(2) New game", 25);
            cout << endl;
            Sleep(250);
            delayPrint("(3) Record", 25);
            cout << endl;
            Sleep(250);
            delayPrint("(4) Pengaturan", 25);
            cout << endl;
            Sleep(250);
            delayPrint("(5) Exit", 25);
            cout << endl;
            Sleep(250);
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

            delayPrint("Masukkan nickname: ", 25);
            getline(cin, nickname);
            clearScreen();
            delayPrint(nickname + ", ", 25);
            Sleep(750);
            delayPrint("Bersiaplah! ", 25);
            Sleep(750);
            delayPrint(". . .", 250);
            Sleep(1500);
            clearScreen();

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
            c = _getch();
            int addScore = 0;
            
            for (int i = highestCombo; i >= 0; i--){
                clearScreen();
                cout << "GAME OVER!" << endl << endl;
                cout << "Score = " << score << endl;
                cout << "Highest combo = " << i << endl;
                cout << "Additional score " << addScore << endl;
                addScore += 5;
                Sleep(100);
            }
            c = _getch();
            for (int i = addScore; i >= 0; i-=5){
                clearScreen();
                cout << "GAME OVER!" << endl << endl;
                cout << "Score = " << score << endl;
                score += 5;
                cout << endl;
                cout << "Additional score " << i << endl;
                Sleep(100);
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
                    // delayPrint("E M U A C H", 250); 
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
    hideCursor();  
    clearScreen();
    SetConsoleOutputCP(CP_UTF8);
    Menu tampilanAwal;
}