#include <SFML/Graphics.hpp>
#include <iostream>
#include <vector>
#include <string>
#include <map>

class GrafisktSchack {
private:
    sf::RenderWindow window;
    sf::Font font;
    char brade[8][8];
    bool vitAttDra;
    sf::Vector2i valtRuta;
    bool harValtPjas;
    
    // Texturer för pjäser
    std::map<char, sf::Texture> pjasTexturer;
    
    // Färger
    sf::Color ljusRuta{240, 217, 181};
    sf::Color morkRuta{181, 136, 99};
    sf::Color markeradRuta{255, 255, 0, 128};
    sf::Color textFarg{50, 50, 50};
    
public:
    GrafisktSchack() : window(sf::VideoMode(640, 720), "Schackprogram med PNG") {
        initieraBrade();
        vitAttDra = true;
        harValtPjas = false;
        valtRuta = {-1, -1};
        
        // Ladda font
        if (!font.loadFromFile("C:/Windows/Fonts/arial.ttf")) {
            std::cout << "Varning: Kunde inte ladda font\n";
        }
        
        // Ladda pjäsbilder
        laddaPjasTexturer();
    }
    
    void laddaPjasTexturer() {
        struct PjasInfo {
            char symbol;
            std::string filnamn;
        };
        
        std::vector<PjasInfo> pjaser = {
            {'K', "images/white_king.png"},
            {'Q', "images/white_queen.png"},
            {'R', "images/white_rook.png"},
            {'B', "images/white_bishop.png"},
            {'N', "images/white_knight.png"},
            {'P', "images/white_pawn.png"},
            {'k', "images/black_king.png"},
            {'q', "images/black_queen.png"},
            {'r', "images/black_rook.png"},
            {'b', "images/black_bishop.png"},
            {'n', "images/black_knight.png"},
            {'p', "images/black_pawn.png"}
        };
        
        for(const auto& pjas : pjaser) {
            if(!pjasTexturer[pjas.symbol].loadFromFile(pjas.filnamn)) {
                std::cout << "Varning: Kunde inte ladda " << pjas.filnamn << std::endl;
                std::cout << "Kontrollera att filen finns i images/ mappen\n";
            } else {
                std::cout << "Laddade " << pjas.filnamn << " ✓\n";
            }
        }
    }
    
    void initieraBrade() {
        // Töm brädet
        for(int rad = 0; rad < 8; rad++) {
            for(int kol = 0; kol < 8; kol++) {
                brade[rad][kol] = ' ';
            }
        }
        
        // Sätt upp vita pjäser (rad 0 och 1)
        brade[0][0] = 'R'; brade[0][7] = 'R'; // Torn
        brade[0][1] = 'N'; brade[0][6] = 'N'; // Häst
        brade[0][2] = 'B'; brade[0][5] = 'B'; // Löpare
        brade[0][3] = 'Q'; // Dam
        brade[0][4] = 'K'; // Kung
        
        // Vita bönder
        for(int kol = 0; kol < 8; kol++) {
            brade[1][kol] = 'P';
        }
        
        // Sätt upp svarta pjäser (rad 6 och 7)
        brade[7][0] = 'r'; brade[7][7] = 'r';
        brade[7][1] = 'n'; brade[7][6] = 'n';
        brade[7][2] = 'b'; brade[7][5] = 'b';
        brade[7][3] = 'q';
        brade[7][4] = 'k';
        
        // Svarta bönder
        for(int kol = 0; kol < 8; kol++) {
            brade[6][kol] = 'p';
        }
    }
    
    void ritaBrade() {
        const float rutStorlek = 80.0f;
        const float startX = 0;
        const float startY = 80; // Plats för status text
        
        // Rita rutor
        for(int rad = 0; rad < 8; rad++) {
            for(int kol = 0; kol < 8; kol++) {
                sf::RectangleShape ruta(sf::Vector2f(rutStorlek, rutStorlek));
                ruta.setPosition(startX + kol * rutStorlek, startY + (7-rad) * rutStorlek);
                
                // Alternera färger
                if((rad + kol) % 2 == 0) {
                    ruta.setFillColor(ljusRuta);
                } else {
                    ruta.setFillColor(morkRuta);
                }
                
                // Markera vald ruta
                if(harValtPjas && valtRuta.x == kol && valtRuta.y == rad) {
                    ruta.setFillColor(markeradRuta);
                }
                
                window.draw(ruta);
                
                // Rita pjäs med bild
                if(brade[rad][kol] != ' ') {
                    char pjas = brade[rad][kol];
                    
                    if(pjasTexturer.find(pjas) != pjasTexturer.end()) {
                        // Använd PNG-bild
                        sf::Sprite pjasSprite;
                        pjasSprite.setTexture(pjasTexturer[pjas]);
                        
                        // Skala bilden till att passa i rutan (med lite marginal)
                        sf::Vector2u textureSize = pjasTexturer[pjas].getSize();
                        float scale = (rutStorlek * 0.8f) / std::max(textureSize.x, textureSize.y);
                        pjasSprite.setScale(scale, scale);
                        
                        // Centrera bilden i rutan
                        sf::FloatRect spriteBounds = pjasSprite.getLocalBounds();
                        pjasSprite.setPosition(
                            startX + kol * rutStorlek + (rutStorlek - spriteBounds.width * scale) / 2,
                            startY + (7-rad) * rutStorlek + (rutStorlek - spriteBounds.height * scale) / 2
                        );
                        
                        window.draw(pjasSprite);
                    } else {
                        // Fallback till text om bilden inte finns
                        sf::Text pjasText;
                        pjasText.setFont(font);
                        pjasText.setString(std::string(1, pjas));
                        pjasText.setCharacterSize(48);
                        pjasText.setFillColor(textFarg);
                        pjasText.setStyle(sf::Text::Bold);
                        
                        sf::FloatRect textBounds = pjasText.getLocalBounds();
                        pjasText.setPosition(
                            startX + kol * rutStorlek + (rutStorlek - textBounds.width) / 2,
                            startY + (7-rad) * rutStorlek + (rutStorlek - textBounds.height) / 2
                        );
                        
                        window.draw(pjasText);
                    }
                }
            }
        }
        
        // Rita koordinater
        for(int i = 0; i < 8; i++) {
            // Kolumn-etiketter (a-h)
            sf::Text kolText;
            kolText.setFont(font);
            kolText.setString(std::string(1, 'a' + i));
            kolText.setCharacterSize(20);
            kolText.setFillColor(textFarg);
            kolText.setPosition(startX + i * rutStorlek + 35, startY + 8 * rutStorlek + 5);
            window.draw(kolText);
            
            // Rad-etiketter (1-8)
            sf::Text radText;
            radText.setFont(font);
            radText.setString(std::to_string(i + 1));
            radText.setCharacterSize(20);
            radText.setFillColor(textFarg);
            radText.setPosition(startX - 25, startY + (7-i) * rutStorlek + 35);
            window.draw(radText);
        }
    }
    
    void ritaStatus() {
        sf::Text statusText;
        statusText.setFont(font);
        statusText.setString(vitAttDra ? "Vits tur" : "Svarts tur");
        statusText.setCharacterSize(24);
        statusText.setFillColor(textFarg);
        statusText.setPosition(10, 10);
        window.draw(statusText);
        
        if(harValtPjas) {
            sf::Text valtText;
            valtText.setFont(font);
            valtText.setString("Pjas vald - klicka destination");
            valtText.setCharacterSize(18);
            valtText.setFillColor(sf::Color::Blue);
            valtText.setPosition(10, 40);
            window.draw(valtText);
        }
    }
    
    sf::Vector2i musKlickTillRuta(sf::Vector2i musPos) {
        const float rutStorlek = 80.0f;
        const float startY = 80;
        
        int kol = musPos.x / (int)rutStorlek;
        int rad = 7 - ((musPos.y - (int)startY) / (int)rutStorlek);
        
        if(kol < 0 || kol > 7 || rad < 0 || rad > 7) {
            return {-1, -1}; // Utanför brädet
        }
        
        return {kol, rad};
    }
    
    bool flyttaPjas(int franKol, int franRad, int tillKol, int tillRad) {
        // Kontrollera att det finns en pjäs att flytta
        if(brade[franRad][franKol] == ' ') {
            return false;
        }
        
        // Kontrollera färg
        bool pjasArVit = (brade[franRad][franKol] >= 'A' && brade[franRad][franKol] <= 'Z');
        if(pjasArVit != vitAttDra) {
            return false;
        }
        
        // Gör draget
        brade[tillRad][tillKol] = brade[franRad][franKol];
        brade[franRad][franKol] = ' ';
        
        // Växla tur
        vitAttDra = !vitAttDra;
        return true;
    }
    
    void hanteraMusKlick(sf::Vector2i musPos) {
        sf::Vector2i ruta = musKlickTillRuta(musPos);
        
        if(ruta.x == -1) return; // Utanför brädet
        
        if(!harValtPjas) {
            // Välj pjäs om det finns en på denna ruta
            if(brade[ruta.y][ruta.x] != ' ') {
                // Kontrollera att det är rätt färgs tur
                bool pjasArVit = (brade[ruta.y][ruta.x] >= 'A' && brade[ruta.y][ruta.x] <= 'Z');
                if(pjasArVit == vitAttDra) {
                    valtRuta = ruta;
                    harValtPjas = true;
                }
            }
        } else {
            // Försök flytta till denna ruta
            if(flyttaPjas(valtRuta.x, valtRuta.y, ruta.x, ruta.y)) {
                std::cout << "Flyttade från " << (char)('a' + valtRuta.x) << (valtRuta.y + 1) 
                          << " till " << (char)('a' + ruta.x) << (ruta.y + 1) << std::endl;
                harValtPjas = false;
            } else {
                // Ogiltigt drag - välj ny pjäs istället om möjligt
                if(brade[ruta.y][ruta.x] != ' ') {
                    bool pjasArVit = (brade[ruta.y][ruta.x] >= 'A' && brade[ruta.y][ruta.x] <= 'Z');
                    if(pjasArVit == vitAttDra) {
                        valtRuta = ruta;
                    } else {
                        harValtPjas = false;
                    }
                } else {
                    harValtPjas = false;
                }
            }
        }
    }
    
    void kora() {
        while(window.isOpen()) {
            sf::Event event;
            while(window.pollEvent(event)) {
                if(event.type == sf::Event::Closed) {
                    window.close();
                }
                
                if(event.type == sf::Event::MouseButtonPressed) {
                    if(event.mouseButton.button == sf::Mouse::Left) {
                        hanteraMusKlick({event.mouseButton.x, event.mouseButton.y});
                    }
                }
            }
            
            window.clear(sf::Color::White);
            ritaBrade();
            ritaStatus();
            window.display();
        }
    }
};

int main() {
    std::cout << "Startar grafiskt schackprogram med PNG-bilder...\n";
    GrafisktSchack spel;
    spel.kora();
    return 0;
}