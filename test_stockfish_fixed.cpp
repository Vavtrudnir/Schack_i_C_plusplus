#include <iostream>
#include <string>
#include <thread>
#include <chrono>
#include <windows.h>

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
            
            // Look for bestmove or readyok
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
        if (!createPipes()) {
            std::cout << "Failed to create pipes" << std::endl;
            return false;
        }
        
        if (!startProcess()) {
            std::cout << "Failed to start process" << std::endl;
            return false;
        }
        
        isRunning = true;
        
        sendCommand("uci");
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        std::string uciResponse = readResponse();
        std::cout << "UCI response: " << uciResponse << std::endl;
        
        sendCommand("isready");
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        std::string readyResponse = readResponse();
        std::cout << "Ready response: " << readyResponse << std::endl;
        
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
        std::cout << "Move response: " << response << std::endl;
        
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

int main() {
    std::cout << "Testing Stockfish integration..." << std::endl;
    
    StockfishUCI stockfish;
    if (!stockfish.start()) {
        std::cout << "Failed to start Stockfish" << std::endl;
        return 1;
    }
    
    std::string startingFEN = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
    std::cout << "Getting best move for starting position..." << std::endl;
    
    std::string bestMove = stockfish.getBestMove(startingFEN, 1000);
    if (!bestMove.empty()) {
        std::cout << "Best move: " << bestMove << std::endl;
        std::cout << "Stockfish integration test PASSED!" << std::endl;
    } else {
        std::cout << "Failed to get best move" << std::endl;
        std::cout << "Stockfish integration test FAILED!" << std::endl;
    }
    
    return 0;
}
