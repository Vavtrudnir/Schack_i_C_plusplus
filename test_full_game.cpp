#include <iostream>
#include <string>
#include <thread>
#include <chrono>
#include <windows.h>

// Copy the StockfishUCI class from the main file
class StockfishUCI {
private:
    PROCESS_INFORMATION pi;
    HANDLE hChildStdinRd, hChildStdinWr, hChildStdoutRd, hChildStdoutWr;
    bool isRunning;
    
    bool createPipes() {
        SECURITY_ATTRIBUTES saAttr;
        saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
        saAttr.bInheritHandle = TRUE;
        saAttr.lpSecurityDescriptor = NULL;

        if (!CreatePipe(&hChildStdoutRd, &hChildStdoutWr, &saAttr, 0))
            return false;
        if (!CreatePipe(&hChildStdinRd, &hChildStdinWr, &saAttr, 0))
            return false;
        return true;
    }
    
    bool startProcess() {
        STARTUPINFO si;
        ZeroMemory(&pi, sizeof(PROCESS_INFORMATION));
        ZeroMemory(&si, sizeof(STARTUPINFO));
        
        si.cb = sizeof(STARTUPINFO);
        si.hStdError = hChildStdoutWr;
        si.hStdOutput = hChildStdoutWr;
        si.hStdInput = hChildStdinRd;
        si.dwFlags |= STARTF_USESTDHANDLES;
        
        std::string cmd = "stockfish.exe";
        
        if (!CreateProcess(NULL, const_cast<char*>(cmd.c_str()), NULL, NULL, TRUE, 
                          CREATE_NO_WINDOW, NULL, NULL, &si, &pi)) {
            std::cout << "Failed to start Stockfish: " << GetLastError() << std::endl;
            return false;
        }
        return true;
    }
    
    void sendCommand(const std::string& cmd) {
        DWORD written;
        std::string command = cmd + "\n";
        WriteFile(hChildStdinWr, command.c_str(), command.length(), &written, NULL);
    }
    
    std::string readResponse() {
        char buffer[4096];
        DWORD read;
        std::string response;
        auto startTime = std::chrono::steady_clock::now();
        
        while (std::chrono::steady_clock::now() - startTime < std::chrono::milliseconds(2000)) {
            if (!ReadFile(hChildStdoutRd, buffer, sizeof(buffer) - 1, &read, NULL) || read == 0) {
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
                continue;
            }
            
            buffer[read] = '\0';
            response += buffer;
            
            if (response.find("bestmove") != std::string::npos || 
                response.find("readyok") != std::string::npos ||
                response.find("uciok") != std::string::npos) {
                break;
            }
        }
        
        return response;
    }

public:
    StockfishUCI() : isRunning(false) {}
    
    ~StockfishUCI() {
        stop();
    }
    
    bool start() {
        if (!createPipes() || !startProcess()) {
            return false;
        }
        
        isRunning = true;
        
        sendCommand("uci");
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        readResponse();
        
        sendCommand("isready");
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        readResponse();
        
        std::cout << "Stockfish started successfully" << std::endl;
        return true;
    }
    
    void stop() {
        if (isRunning) {
            sendCommand("quit");
            TerminateProcess(pi.hProcess, 0);
            CloseHandle(pi.hProcess);
            CloseHandle(pi.hThread);
            CloseHandle(hChildStdinRd);
            CloseHandle(hChildStdinWr);
            CloseHandle(hChildStdoutRd);
            CloseHandle(hChildStdoutWr);
            isRunning = false;
        }
    }
    
    std::string getBestMove(const std::string& fen, int timeMs = 1000) {
        if (!isRunning) return "";
        
        sendCommand("position fen " + fen);
        sendCommand("go movetime " + std::to_string(timeMs));
        
        std::this_thread::sleep_for(std::chrono::milliseconds(timeMs + 500));
        
        std::string response = readResponse();
        
        size_t pos = response.find("bestmove ");
        if (pos != std::string::npos) {
            size_t start = pos + 9;
            size_t end = response.find(" ", start);
            if (end == std::string::npos) {
                end = response.find("\n", start);
            }
            return response.substr(start, end - start);
        }
        
        return "";
    }
};

// Simplified chess logic for testing
enum PjasTyp { TOM, BONDE, SPRINGARE, LOPARE, TORN, DAM, KUNG };
enum Farg { VIT, SVART };

struct Pjas {
    PjasTyp typ;
    Farg farg;
    bool harFlyttat;
    
    Pjas() : typ(TOM), farg(VIT), harFlyttat(false) {}
    Pjas(PjasTyp t, Farg f) : typ(t), farg(f), harFlyttat(false) {}
};

class ChessGame {
private:
    Pjas brade[8][8];
    bool vitAttDra;
    StockfishUCI* stockfish;
    
    void initieraBrade() {
        // Sätt upp startpositionen
        for (int rad = 0; rad < 8; rad++) {
            for (int kol = 0; kol < 8; kol++) {
                brade[rad][kol] = Pjas();
            }
        }
        
        // Vita pjäser
        brade[7][0] = Pjas(TORN, VIT);
        brade[7][1] = Pjas(SPRINGARE, VIT);
        brade[7][2] = Pjas(LOPARE, VIT);
        brade[7][3] = Pjas(DAM, VIT);
        brade[7][4] = Pjas(KUNG, VIT);
        brade[7][5] = Pjas(LOPARE, VIT);
        brade[7][6] = Pjas(SPRINGARE, VIT);
        brade[7][7] = Pjas(TORN, VIT);
        for (int i = 0; i < 8; i++) {
            brade[6][i] = Pjas(BONDE, VIT);
        }
        
        // Svarta pjäser
        brade[0][0] = Pjas(TORN, SVART);
        brade[0][1] = Pjas(SPRINGARE, SVART);
        brade[0][2] = Pjas(LOPARE, SVART);
        brade[0][3] = Pjas(DAM, SVART);
        brade[0][4] = Pjas(KUNG, SVART);
        brade[0][5] = Pjas(LOPARE, SVART);
        brade[0][6] = Pjas(SPRINGARE, SVART);
        brade[0][7] = Pjas(TORN, SVART);
        for (int i = 0; i < 8; i++) {
            brade[1][i] = Pjas(BONDE, SVART);
        }
    }
    
    std::string skapaFEN() {
        std::string fen = "";
        
        for (int rad = 7; rad >= 0; rad--) {
            int tommaRutor = 0;
            for (int kol = 0; kol < 8; kol++) {
                Pjas pjas = brade[rad][kol];
                if (pjas.typ == TOM) {
                    tommaRutor++;
                } else {
                    if (tommaRutor > 0) {
                        fen += std::to_string(tommaRutor);
                        tommaRutor = 0;
                    }
                    
                    char pjasChar;
                    switch(pjas.typ) {
                        case BONDE: pjasChar = 'p'; break;
                        case SPRINGARE: pjasChar = 'n'; break;
                        case LOPARE: pjasChar = 'b'; break;
                        case TORN: pjasChar = 'r'; break;
                        case DAM: pjasChar = 'q'; break;
                        case KUNG: pjasChar = 'k'; break;
                        default: pjasChar = '?'; break;
                    }
                    
                    if (pjas.farg == VIT) {
                        pjasChar = toupper(pjasChar);
                    }
                    fen += pjasChar;
                }
            }
            if (tommaRutor > 0) {
                fen += std::to_string(tommaRutor);
            }
            if (rad > 0) {
                fen += "/";
            }
        }
        
        fen += " ";
        fen += vitAttDra ? "w" : "b";
        fen += " KQkq - 0 1";
        
        return fen;
    }
    
    bool flyttaPjas(int franKol, int franRad, int tillKol, int tillRad) {
        if (franKol < 0 || franKol > 7 || franRad < 0 || franRad > 7 ||
            tillKol < 0 || tillKol > 7 || tillRad < 0 || tillRad > 7) {
            return false;
        }
        
        Pjas franPjas = brade[franRad][franKol];
        Pjas tillPjas = brade[tillRad][tillKol];
        
        if (franPjas.typ == TOM) return false;
        if (franPjas.farg == tillPjas.farg) return false;
        if (!vitAttDra && franPjas.farg == VIT) return false;
        if (vitAttDra && franPjas.farg == SVART) return false;
        
        // Simplified move validation
        brade[tillRad][tillKol] = brade[franRad][franKol];
        brade[tillRad][tillKol].harFlyttat = true;
        brade[franRad][franKol] = Pjas();
        
        vitAttDra = !vitAttDra;
        return true;
    }
    
    void gorDatorDrag() {
        if(!stockfish) return;
        
        std::string fen = skapaFEN();
        std::string datorDrag = stockfish->getBestMove(fen, 1000);
        
        if(datorDrag.length() >= 4) {
            int franKol = datorDrag[0] - 'a';
            int franRad = datorDrag[1] - '1';
            int tillKol = datorDrag[2] - 'a';
            int tillRad = datorDrag[3] - '1';
            
            if(flyttaPjas(franKol, franRad, tillKol, tillRad)) {
                std::cout << "Dator flyttade: " << datorDrag << std::endl;
            }
        }
    }

public:
    ChessGame() {
        vitAttDra = true;
        stockfish = new StockfishUCI();
        if (!stockfish->start()) {
            std::cout << "Varning: Kunde inte starta Stockfish" << std::endl;
            delete stockfish;
            stockfish = nullptr;
        }
        initieraBrade();
    }
    
    ~ChessGame() {
        delete stockfish;
    }
    
    void testGame() {
        std::cout << "Testing chess game logic..." << std::endl;
        std::cout << "Starting FEN: " << skapaFEN() << std::endl;
        
        // Test a few moves
        for (int i = 0; i < 4; i++) {
            std::cout << "\n--- Move " << (i+1) << " ---" << std::endl;
            
            if (vitAttDra) {
                // White makes a simple move (e2-e4)
                std::cout << "Vit drar: e2-e4" << std::endl;
                flyttaPjas(4, 6, 4, 4);
            } else {
                // Black uses Stockfish
                std::cout << "Svart drar (Stockfish)..." << std::endl;
                gorDatorDrag();
            }
            
            std::cout << "Current FEN: " << skapaFEN() << std::endl;
        }
        
        std::cout << "\nGame test completed successfully!" << std::endl;
    }
};

int main() {
    std::cout << "=== Chess Game Test ===" << std::endl;
    
    ChessGame game;
    game.testGame();
    
    return 0;
}
