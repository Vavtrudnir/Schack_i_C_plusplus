#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <iostream>
#include <vector>
#include <memory>
#include <string>
#include <cstring>
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

        // Create stdout pipe
        if (!CreatePipe(&hChildStdoutRd, &hChildStdoutWr, &saAttr, 0))
            return false;

        // Create stdin pipe
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
        
        // Initialize UCI
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
        
        std::this_thread::sleep_for(std::chrono::milliseconds(timeMs + 100));
        
        std::string response = readResponse();
        
        // Parse bestmove from response
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
    
    bool isMoveValid(const std::string& fen, const std::string& move) {
        if (!isRunning) return false;
        
        sendCommand("position fen " + fen);
        sendCommand("go searchmoves " + move + " movetime 10");
        
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        
        std::string response = readResponse();
        return response.find("bestmove " + move) != std::string::npos;
    }
};

enum PjasTyp { TOM, BONDE, SPRINGARE, LOPARE, TORN, DAM, KUNG };
enum Farg { VIT, SVART };
enum GameMode { HUMAN_VS_HUMAN, HUMAN_VS_COMPUTER };

struct Pjas {
    PjasTyp typ;
    Farg farg;
    bool harFlyttat;
    
    Pjas() : typ(TOM), farg(VIT), harFlyttat(false) {}
    Pjas(PjasTyp t, Farg f) : typ(t), farg(f), harFlyttat(false) {}
};

class GrafisktSchack {
private:
    sf::RenderWindow window;
    sf::Font font;
    std::vector<sf::Texture> pieceTextures;
    Pjas brade[8][8];
    bool vitAttDra;
    bool harValtPjas;
    sf::Vector2i valtRuta;
    GameMode gameMode;
    bool computerThinking;
    StockfishUCI* stockfish;
    sf::Vector2i enPassantRuta;  // Ruta där en passant är möjlig (-1,-1 om ingen)
    bool spelatSlut;
    std::string slutMeddelande;
    
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
    
    void laddaPjasTexturer() {
        pieceTextures.resize(12);
        
        std::string filnamn[] = {
            "bilder/w_pawn.png", "bilder/w_knight.png", "bilder/w_bishop.png",
            "bilder/w_rook.png", "bilder/w_queen.png", "bilder/w_king.png",
            "bilder/b_pawn.png", "bilder/b_knight.png", "bilder/b_bishop.png",
            "bilder/b_rook.png", "bilder/b_queen.png", "bilder/b_king.png"
        };
        
        for (int i = 0; i < 12; i++) {
            if (!pieceTextures[i].loadFromFile(filnamn[i])) {
                std::cout << "Kunde inte ladda " << filnamn[i] << std::endl;
            }
        }
    }
    
    int getTextureIndex(PjasTyp typ, Farg farg) {
        if (farg == VIT) {
            switch(typ) {
                case BONDE: return 0;
                case SPRINGARE: return 1;
                case LOPARE: return 2;
                case TORN: return 3;
                case DAM: return 4;
                case KUNG: return 5;
                default: return -1;
            }
        } else {
            switch(typ) {
                case BONDE: return 6;
                case SPRINGARE: return 7;
                case LOPARE: return 8;
                case TORN: return 9;
                case DAM: return 10;
                case KUNG: return 11;
                default: return -1;
            }
        }
    }
    
    std::string skapaFEN() {
        std::string fen = "";
        
        // Lägg till pjäserna (rad 0 = rank 8 = svart baksida, rad 7 = rank 1 = vit baksida)
        for (int rad = 0; rad < 8; rad++) {
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
            if (rad < 7) {
                fen += "/";
            }
        }
        
        // Lägg till vem som drar
        fen += " ";
        fen += vitAttDra ? "w" : "b";
        
        // Förenklad FEN - vi inkluderar bara det nödvändigaste
        // Castling rights, en passant target square, halfmove clock, fullmove number
        fen += " KQkq - 0 1";
        
        return fen;
    }
    
    // Hjälpfunktion: Kontrollera om vägen är fri mellan två rutor
    bool arVagenFri(int franKol, int franRad, int tillKol, int tillRad) {
        int kolRiktning = (tillKol > franKol) ? 1 : (tillKol < franKol) ? -1 : 0;
        int radRiktning = (tillRad > franRad) ? 1 : (tillRad < franRad) ? -1 : 0;
        
        int kol = franKol + kolRiktning;
        int rad = franRad + radRiktning;
        
        while (kol != tillKol || rad != tillRad) {
            if (brade[rad][kol].typ != TOM) {
                return false;
            }
            kol += kolRiktning;
            rad += radRiktning;
        }
        
        return true;
    }
    
    // Hitta kungens position
    sf::Vector2i hittaKung(Farg farg) {
        for (int rad = 0; rad < 8; rad++) {
            for (int kol = 0; kol < 8; kol++) {
                if (brade[rad][kol].typ == KUNG && brade[rad][kol].farg == farg) {
                    return {kol, rad};
                }
            }
        }
        return {-1, -1};
    }
    
    // Kontrollera om en ruta är hotad av motståndaren
    bool arRutaHotad(int kol, int rad, Farg avFarg) {
        Farg motstandardFarg = (avFarg == VIT) ? SVART : VIT;
        
        for (int r = 0; r < 8; r++) {
            for (int k = 0; k < 8; k++) {
                if (brade[r][k].typ != TOM && brade[r][k].farg == motstandardFarg) {
                    // Kontrollera om denna pjäs kan attackera rutan
                    if (kanPjasAttackeraRuta(k, r, kol, rad)) {
                        return true;
                    }
                }
            }
        }
        
        return false;
    }
    
    // Kontrollera om en pjäs kan attackera en ruta (utan att kolla schack)
    bool kanPjasAttackeraRuta(int franKol, int franRad, int tillKol, int tillRad) {
        Pjas pjas = brade[franRad][franKol];
        int kolDiff = abs(tillKol - franKol);
        int radDiff = abs(tillRad - franRad);
        
        switch(pjas.typ) {
            case BONDE:
                // Bönder attackerar diagonalt
                if (pjas.farg == VIT) {
                    return (kolDiff == 1 && tillRad == franRad - 1);
                } else {
                    return (kolDiff == 1 && tillRad == franRad + 1);
                }
                
            case TORN:
                if (franKol == tillKol || franRad == tillRad) {
                    return arVagenFri(franKol, franRad, tillKol, tillRad);
                }
                return false;
                
            case LOPARE:
                if (kolDiff == radDiff && kolDiff > 0) {
                    return arVagenFri(franKol, franRad, tillKol, tillRad);
                }
                return false;
                
            case DAM:
                if ((franKol == tillKol || franRad == tillRad || kolDiff == radDiff) && 
                    (franKol != tillKol || franRad != tillRad)) {
                    return arVagenFri(franKol, franRad, tillKol, tillRad);
                }
                return false;
                
            case SPRINGARE:
                return (kolDiff == 2 && radDiff == 1) || (kolDiff == 1 && radDiff == 2);
                
            case KUNG:
                return kolDiff <= 1 && radDiff <= 1 && (kolDiff > 0 || radDiff > 0);
                
            default:
                return false;
        }
    }
    
    // Kontrollera om kungen är i schack
    bool arKungenISchack(Farg farg) {
        sf::Vector2i kungPos = hittaKung(farg);
        if (kungPos.x == -1) return false;
        return arRutaHotad(kungPos.x, kungPos.y, farg);
    }
    
    // Kontrollera om en spelare har några giltiga drag
    bool harGiltigaDrag(Farg farg) {
        for (int franRad = 0; franRad < 8; franRad++) {
            for (int franKol = 0; franKol < 8; franKol++) {
                if (brade[franRad][franKol].typ != TOM && brade[franRad][franKol].farg == farg) {
                    // Testa alla möjliga destinationer
                    for (int tillRad = 0; tillRad < 8; tillRad++) {
                        for (int tillKol = 0; tillKol < 8; tillKol++) {
                            if (giltigtDrag(franKol, franRad, tillKol, tillRad)) {
                                return true;
                            }
                        }
                    }
                }
            }
        }
        return false;
    }
    
    // Kontrollera om det är schackmatt
    bool arSchackmatt(Farg farg) {
        return arKungenISchack(farg) && !harGiltigaDrag(farg);
    }
    
    // Kontrollera om det är patt (ingen schack men inga giltiga drag)
    bool arPatt(Farg farg) {
        return !arKungenISchack(farg) && !harGiltigaDrag(farg);
    }
    
    // Simulera ett drag och kolla om det lämnar kungen i schack
    bool lamnarKungenISchack(int franKol, int franRad, int tillKol, int tillRad) {
        // Spara tillståndet
        Pjas sparadFran = brade[franRad][franKol];
        Pjas sparadTill = brade[tillRad][tillKol];
        
        // Gör draget temporärt
        brade[tillRad][tillKol] = brade[franRad][franKol];
        brade[franRad][franKol] = Pjas();
        
        // Kolla schack
        bool iSchack = arKungenISchack(sparadFran.farg);
        
        // Återställ
        brade[franRad][franKol] = sparadFran;
        brade[tillRad][tillKol] = sparadTill;
        
        return iSchack;
    }
    
    bool giltigtDrag(int franKol, int franRad, int tillKol, int tillRad) {
        if (franKol < 0 || franKol > 7 || franRad < 0 || franRad > 7 ||
            tillKol < 0 || tillKol > 7 || tillRad < 0 || tillRad > 7) {
            return false;
        }
        
        Pjas franPjas = brade[franRad][franKol];
        Pjas tillPjas = brade[tillRad][tillKol];
        
        // Grundläggande kontroller
        if (franPjas.typ == TOM) return false;
        if (tillPjas.typ != TOM && franPjas.farg == tillPjas.farg) return false;
        if (!vitAttDra && franPjas.farg == VIT) return false;
        if (vitAttDra && franPjas.farg == SVART) return false;
        if (franKol == tillKol && franRad == tillRad) return false;
        
        int kolDiff = abs(tillKol - franKol);
        int radDiff = abs(tillRad - franRad);
        
        // Kontrollera pjässpecifika regler
        bool grundDragGiltigt = false;
        
        switch(franPjas.typ) {
            case BONDE:
                if (franPjas.farg == VIT) {
                    // Framåt ett steg
                    if (franKol == tillKol && tillRad == franRad - 1 && tillPjas.typ == TOM) {
                        grundDragGiltigt = true;
                    }
                    // Framåt två steg från startposition
                    else if (franKol == tillKol && franRad == 6 && tillRad == 4 && 
                             tillPjas.typ == TOM && brade[5][franKol].typ == TOM) {
                        grundDragGiltigt = true;
                    }
                    // Slå diagonalt
                    else if (kolDiff == 1 && tillRad == franRad - 1 && tillPjas.typ != TOM) {
                        grundDragGiltigt = true;
                    }
                    // En passant
                    else if (kolDiff == 1 && tillRad == franRad - 1 && 
                             enPassantRuta.x == tillKol && enPassantRuta.y == tillRad) {
                        grundDragGiltigt = true;
                    }
                } else {
                    // Framåt ett steg
                    if (franKol == tillKol && tillRad == franRad + 1 && tillPjas.typ == TOM) {
                        grundDragGiltigt = true;
                    }
                    // Framåt två steg från startposition
                    else if (franKol == tillKol && franRad == 1 && tillRad == 3 && 
                             tillPjas.typ == TOM && brade[2][franKol].typ == TOM) {
                        grundDragGiltigt = true;
                    }
                    // Slå diagonalt
                    else if (kolDiff == 1 && tillRad == franRad + 1 && tillPjas.typ != TOM) {
                        grundDragGiltigt = true;
                    }
                    // En passant
                    else if (kolDiff == 1 && tillRad == franRad + 1 && 
                             enPassantRuta.x == tillKol && enPassantRuta.y == tillRad) {
                        grundDragGiltigt = true;
                    }
                }
                break;
                
            case TORN:
                if (franKol == tillKol || franRad == tillRad) {
                    grundDragGiltigt = arVagenFri(franKol, franRad, tillKol, tillRad);
                }
                break;
                
            case LOPARE:
                if (kolDiff == radDiff && kolDiff > 0) {
                    grundDragGiltigt = arVagenFri(franKol, franRad, tillKol, tillRad);
                }
                break;
                
            case DAM:
                if (franKol == tillKol || franRad == tillRad || kolDiff == radDiff) {
                    grundDragGiltigt = arVagenFri(franKol, franRad, tillKol, tillRad);
                }
                break;
                
            case SPRINGARE:
                grundDragGiltigt = (kolDiff == 2 && radDiff == 1) || (kolDiff == 1 && radDiff == 2);
                break;
                
            case KUNG:
                if (kolDiff <= 1 && radDiff <= 1) {
                    grundDragGiltigt = true;
                }
                // Rockad
                else if (!franPjas.harFlyttat && franRad == tillRad && kolDiff == 2) {
                    // Kort rockad (O-O)
                    if (tillKol == 6) {
                        Pjas torn = brade[franRad][7];
                        if (torn.typ == TORN && !torn.harFlyttat && 
                            brade[franRad][5].typ == TOM && brade[franRad][6].typ == TOM &&
                            !arRutaHotad(franKol, franRad, franPjas.farg) &&
                            !arRutaHotad(5, franRad, franPjas.farg) &&
                            !arRutaHotad(6, franRad, franPjas.farg)) {
                            grundDragGiltigt = true;
                        }
                    }
                    // Lång rockad (O-O-O)
                    else if (tillKol == 2) {
                        Pjas torn = brade[franRad][0];
                        if (torn.typ == TORN && !torn.harFlyttat && 
                            brade[franRad][1].typ == TOM && brade[franRad][2].typ == TOM && 
                            brade[franRad][3].typ == TOM &&
                            !arRutaHotad(franKol, franRad, franPjas.farg) &&
                            !arRutaHotad(3, franRad, franPjas.farg) &&
                            !arRutaHotad(2, franRad, franPjas.farg)) {
                            grundDragGiltigt = true;
                        }
                    }
                }
                break;
                
            default:
                return false;
        }
        
        if (!grundDragGiltigt) return false;
        
        // Kontrollera att draget inte lämnar kungen i schack
        return !lamnarKungenISchack(franKol, franRad, tillKol, tillRad);
    }
    
    bool flyttaPjas(int franKol, int franRad, int tillKol, int tillRad) {
        if (!giltigtDrag(franKol, franRad, tillKol, tillRad)) {
            return false;
        }
        
        Pjas franPjas = brade[franRad][franKol];
        
        // Hantera en passant
        if (franPjas.typ == BONDE && enPassantRuta.x == tillKol && enPassantRuta.y == tillRad) {
            // Ta bort den slagna bonden
            int slagenBondeRad = (franPjas.farg == VIT) ? tillRad + 1 : tillRad - 1;
            brade[slagenBondeRad][tillKol] = Pjas();
        }
        
        // Återställ en passant-ruta
        enPassantRuta = {-1, -1};
        
        // Sätt ny en passant-ruta om bonde flyttar två steg
        if (franPjas.typ == BONDE && abs(tillRad - franRad) == 2) {
            enPassantRuta.x = franKol;
            enPassantRuta.y = (franRad + tillRad) / 2;
        }
        
        // Hantera rockad
        if (franPjas.typ == KUNG && abs(tillKol - franKol) == 2) {
            // Kort rockad
            if (tillKol == 6) {
                brade[franRad][5] = brade[franRad][7];
                brade[franRad][5].harFlyttat = true;
                brade[franRad][7] = Pjas();
            }
            // Lång rockad
            else if (tillKol == 2) {
                brade[franRad][3] = brade[franRad][0];
                brade[franRad][3].harFlyttat = true;
                brade[franRad][0] = Pjas();
            }
        }
        
        // Gör draget
        brade[tillRad][tillKol] = brade[franRad][franKol];
        brade[tillRad][tillKol].harFlyttat = true;
        brade[franRad][franKol] = Pjas();
        
        // Bondeförvandling
        if (brade[tillRad][tillKol].typ == BONDE) {
            if ((brade[tillRad][tillKol].farg == VIT && tillRad == 0) ||
                (brade[tillRad][tillKol].farg == SVART && tillRad == 7)) {
                // Förvandla alltid till dam (kan göras mer avancerat senare)
                brade[tillRad][tillKol].typ = DAM;
            }
        }
        
        vitAttDra = !vitAttDra;
        
        // Kontrollera spelstatus efter draget
        Farg nuvarandeFarg = vitAttDra ? VIT : SVART;
        if (arSchackmatt(nuvarandeFarg)) {
            spelatSlut = true;
            slutMeddelande = vitAttDra ? "SCHACKMATT! Svart vinner!" : "SCHACKMATT! Vit vinner!";
            std::cout << slutMeddelande << std::endl;
        } else if (arPatt(nuvarandeFarg)) {
            spelatSlut = true;
            slutMeddelande = "PATT! Oavgjort!";
            std::cout << slutMeddelande << std::endl;
        }
        
        return true;
    }
    
    void hanteraMusKlick(sf::Vector2i musPos) {
        if (computerThinking || spelatSlut) return;
        
        int rutaSize = 80;
        int kol = musPos.x / rutaSize;
        int rad = musPos.y / rutaSize;
        
        if (kol < 0 || kol > 7 || rad < 0 || rad > 7) return;
        
        if (!harValtPjas) {
            if (brade[rad][kol].typ != TOM && 
                ((vitAttDra && brade[rad][kol].farg == VIT) || 
                 (!vitAttDra && brade[rad][kol].farg == SVART))) {
                harValtPjas = true;
                valtRuta = {kol, rad};
            }
        } else {
            if (flyttaPjas(valtRuta.x, valtRuta.y, kol, rad)) {
                harValtPjas = false;
                
                // Om det är datorns tur och spelet inte är slut
                if (!spelatSlut && gameMode == HUMAN_VS_COMPUTER && !vitAttDra) {
                    computerThinking = true;
                }
            } else if (brade[rad][kol].typ != TOM && 
                      ((vitAttDra && brade[rad][kol].farg == VIT) || 
                       (!vitAttDra && brade[rad][kol].farg == SVART))) {
                valtRuta = {kol, rad};
            } else {
                harValtPjas = false;
            }
        }
    }
    
    void gorDatorDrag() {
        if(!stockfish || gameMode != HUMAN_VS_COMPUTER) return;
        
        // Sätt till false direkt för att förhindra flera anrop per frame
        computerThinking = false;
        
        std::string fen = skapaFEN();
        
        // Kör Stockfish på en separat tråd så UI inte fryser
        std::thread([this, fen]() {
            std::string datorDrag = stockfish->getBestMove(fen, 1000);
            
            if(datorDrag.length() >= 4) {
                // UCI-notation: 'a'-'h' = kol 0-7, '1'-'8' = rad 0-7
                // I vår array: brade[0] = rank 8 (topp), brade[7] = rank 1 (botten)
                // Konvertering: arrayRad = 7 - (uciRank - '1')
                int franKol = datorDrag[0] - 'a';
                int franRad = 7 - (datorDrag[1] - '1');
                int tillKol = datorDrag[2] - 'a';
                int tillRad = 7 - (datorDrag[3] - '1');
                
                if(flyttaPjas(franKol, franRad, tillKol, tillRad)) {
                    std::cout << "Dator flyttade: " << datorDrag.substr(0, 4) << std::endl;
                }
            }
        }).detach();
    }
    
    void ritaBrade() {
        int rutaSize = 80;
        
        for (int rad = 0; rad < 8; rad++) {
            for (int kol = 0; kol < 8; kol++) {
                sf::RectangleShape ruta({static_cast<float>(rutaSize), static_cast<float>(rutaSize)});
                ruta.setPosition({static_cast<float>(kol * rutaSize), static_cast<float>(rad * rutaSize)});
                
                if ((rad + kol) % 2 == 0) {
                    ruta.setFillColor(sf::Color(240, 217, 181));
                } else {
                    ruta.setFillColor(sf::Color(181, 136, 99));
                }
                
                window.draw(ruta);
                
                // Markera vald ruta
                if (harValtPjas && valtRuta.x == kol && valtRuta.y == rad) {
                    ruta.setFillColor(sf::Color(255, 255, 0, 128));
                    window.draw(ruta);
                }
                
                // Rita pjäs
                if (brade[rad][kol].typ != TOM) {
                    int textureIndex = getTextureIndex(brade[rad][kol].typ, brade[rad][kol].farg);
                    if (textureIndex >= 0) {
                        sf::Sprite sprite(pieceTextures[textureIndex]);
                        
                        // Centrera pjäsen i rutan
                        sf::FloatRect bounds = sprite.getLocalBounds();
                        // Skala pjäsen så den passar bra i rutan (t.ex. 80% av rutans storlek max)
                        float scaleX = (rutaSize * 0.8f) / bounds.size.x;
                        float scaleY = (rutaSize * 0.8f) / bounds.size.y;
                        float scale = std::min(scaleX, scaleY);
                        sprite.setScale({scale, scale});
                        
                        // Sätt positionen så den är centrerad
                        float posX = kol * rutaSize + (rutaSize - bounds.size.x * scale) / 2.0f;
                        float posY = rad * rutaSize + (rutaSize - bounds.size.y * scale) / 2.0f;
                        sprite.setPosition({posX, posY});
                        
                        window.draw(sprite);
                    }
                }
            }
        }
    }
    
    void ritaStatus() {
        std::string statusStr;
        sf::Color statusFarg;
        
        // Använd cachad status om spelet är slut
        if (spelatSlut) {
            statusStr = slutMeddelande;
            statusFarg = sf::Color::Red;
        } else {
            Farg nuvarandeFarg = vitAttDra ? VIT : SVART;
            
            // Kontrollera bara schack (inte matt/patt varje frame)
            if (arKungenISchack(nuvarandeFarg)) {
                statusStr = vitAttDra ? "SCHACK! Vit att dra" : "SCHACK! Svart att dra";
                statusFarg = sf::Color::Red;
            } else {
                statusStr = vitAttDra ? "Vit att dra" : "Svart att dra";
                statusFarg = vitAttDra ? sf::Color::Black : sf::Color::White;
            }
        }
        
        sf::Text statusText(font);
        statusText.setString(statusStr);
        statusText.setCharacterSize(24);
        statusText.setFillColor(statusFarg);
        statusText.setPosition({10.f, 650.f});
        
        // Rita bakgrund för texten
        sf::RectangleShape bakgrund({500.f, 30.f});
        bakgrund.setPosition({5.f, 645.f});
        bakgrund.setFillColor(sf::Color(240, 240, 240));
        window.draw(bakgrund);
        
        window.draw(statusText);
        
        // Visa spelläge
        sf::Text modeText(font);
        const char* modeUtf8 = (gameMode == HUMAN_VS_HUMAN) ? 
            u8"Människa vs Människa" : u8"Människa vs Dator";
        sf::String modeStr = sf::String::fromUtf8(modeUtf8, modeUtf8 + std::strlen(modeUtf8));
        modeText.setString(modeStr);
        modeText.setCharacterSize(18);
        modeText.setFillColor(sf::Color::Blue);
        modeText.setPosition({10.f, 680.f});
        window.draw(modeText);
    }

public:
    GrafisktSchack(GameMode mode = HUMAN_VS_COMPUTER) : window(sf::VideoMode({640, 720}), "Schackprogram med PNG") {
        initieraBrade();
        vitAttDra = true;
        harValtPjas = false;
        valtRuta = {-1, -1};
        enPassantRuta = {-1, -1};
        gameMode = mode;
        computerThinking = false;
        spelatSlut = false;
        slutMeddelande = "";
        
        // Starta Stockfish
        stockfish = new StockfishUCI();
        if (!stockfish->start()) {
            std::cout << "Varning: Kunde inte starta Stockfish. Spel fungerar i human-vs-human mode.\n";
            gameMode = HUMAN_VS_HUMAN;
        }
        
        // Ladda font
        if (!font.openFromFile("C:/Windows/Fonts/arial.ttf")) {
            std::cout << "Varning: Kunde inte ladda font\n";
        }
        
        // Ladda pjäsbilder
        laddaPjasTexturer();
    }
    
    ~GrafisktSchack() {
        delete stockfish;
    }
    
    void kora() {
        while(window.isOpen()) {
            while (const std::optional event = window.pollEvent()) {
                if (event->is<sf::Event::Closed>()) {
                    window.close();
                } else if (const auto* mousePressed = event->getIf<sf::Event::MouseButtonPressed>()) {
                    if (mousePressed->button == sf::Mouse::Button::Left) {
                        hanteraMusKlick({mousePressed->position.x, mousePressed->position.y});
                    }
                }
            }
            
            // Hantera datorns drag
            if(computerThinking) {
                gorDatorDrag();
            }
            
            window.clear(sf::Color::White);
            ritaBrade();
            ritaStatus();
            
            if(computerThinking) {
                sf::Text thinkingText(font, "Datorn tänker...", 20);
                thinkingText.setFillColor(sf::Color::Red);
                thinkingText.setPosition({10.f, 60.f});
                window.draw(thinkingText);
            }
            
            window.display();
        }
    }
};

int main() {
    GrafisktSchack spel;
    spel.kora();
    return 0;
}
