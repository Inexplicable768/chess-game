#include <SFML/Graphics.hpp>
#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <string>

#define WIDTH 512
#define HEIGHT 512
#define TILE 64
#define SPRITESHEET "chess_pieces.bmp"
#define IMG_WIDTH 1052
#define IMG_HEIGHT 375
#define SPRITE_SIZE IMG_WIDTH / 6
#define FPS 120

// board[y][x] format
// Positive = white, Negative = black
// 1 = Pawn, 2 = Rook, 3 = Knight, 4 = Bishop, 5 = Queen, 6 = King
int board[8][8] = {
    {-2, -3, -4, -5, -6, -4, -3, -2},
    {-1, -1, -1, -1, -1, -1, -1, -1},
    {0,  0,  0,  0,  0,  0,  0,  0 },
    {0,  0,  0,  0,  0,  0,  0,  0 },
    {0,  0,  0,  0,  0,  0,  0,  0 },
    {0,  0,  0,  0,  0,  0,  0,  0 },
    {1,  1,  1,  1,  1,  1,  1,  1 },
    {2,  3,  4,  5,  6,  4,  3,  2 }
};

bool isWhiteTurn = true;

int getColor(int piece) {
    return (piece > 0) - (piece < 0);
}

bool isMoveLegal(int sx, int sy, int ex, int ey) { // check if any move is legal
    if (ex < 0 || ex >= 8 || ey < 0 || ey >= 8) return false;
    if (sx == ex && sy == ey) return false;

    int piece = board[sy][sx];
    if (piece == 0) return false;

    int target = board[ey][ex];
    if (target != 0 && getColor(piece) == getColor(target)) return false;

    int dx = ex - sx;
    int dy = ey - sy;

    // PAWN
    if (abs(piece) == 1) {
        int dir = piece > 0 ? -1 : 1;
        if (dx == 0 && dy == dir && target == 0) return true;
        if (dx == 0 && dy == 2 * dir && target == 0 && board[sy + dir][sx] == 0 &&
            ((piece > 0 && sy == 6) || (piece < 0 && sy == 1)))
            return true;
        if (abs(dx) == 1 && dy == dir && target != 0 && getColor(piece) != getColor(target))
            return true;
    }

    // ROOK
    if (abs(piece) == 2) {
        if (dx != 0 && dy != 0) return false;
        int stepX = (dx > 0) - (dx < 0);
        int stepY = (dy > 0) - (dy < 0);
        int x = sx + stepX, y = sy + stepY;
        while (x != ex || y != ey) {
            if (board[y][x] != 0) return false;
            x += stepX;
            y += stepY;
        }
        return true;
    }

    // KNIGHT
    if (abs(piece) == 3)
        return (abs(dx) == 1 && abs(dy) == 2) || (abs(dx) == 2 && abs(dy) == 1);

    // BISHOP
    if (abs(piece) == 4) {
        if (abs(dx) != abs(dy)) return false;
        int stepX = (dx > 0) - (dx < 0);
        int stepY = (dy > 0) - (dy < 0);
        int x = sx + stepX, y = sy + stepY;
        while (x != ex || y != ey) {
            if (board[y][x] != 0) return false;
            x += stepX;
            y += stepY;
        }
        return true;
    }

    // QUEEN
    if (abs(piece) == 5) {
        if (dx == 0 || dy == 0 || abs(dx) == abs(dy)) {
            int stepX = (dx > 0) - (dx < 0);
            int stepY = (dy > 0) - (dy < 0);
            int x = sx + stepX, y = sy + stepY;
            while (x != ex || y != ey) {
                if (board[y][x] != 0) return false;
                x += stepX;
                y += stepY;
            }
            return true;
        }
    }

    // KING
    if (abs(piece) == 6)
        return abs(dx) <= 1 && abs(dy) <= 1; // move if 1 on eacther side


    // SPECIAL CASES //

    // CASTLING
    return false;
}

sf::Vector2i getTileUnderMouse(sf::RenderWindow& window) {
    sf::Vector2i pos = sf::Mouse::getPosition(window);
    return {pos.x / TILE, pos.y / TILE};
}

void drawBoard(sf::RenderWindow& window, sf::Texture& tex) {
    for (int y = 0; y < 8; y++) {
        for (int x = 0; x < 8; x++) {
            sf::RectangleShape tile(sf::Vector2f(TILE, TILE));
            tile.setPosition(x * TILE, y * TILE);
            tile.setFillColor(((x + y) % 2 == 0) ? sf::Color(238, 238, 210) : sf::Color(118, 150, 86));
            window.draw(tile);

            int piece = board[y][x];
            if (piece != 0) {
                sf::Sprite spr(tex);
                int type = abs(piece);
                int row = (piece > 0) ? 0 : 1;
                int col = type - 1;
                spr.setTextureRect(sf::IntRect(col * SPRITE_SIZE, row * SPRITE_SIZE, SPRITE_SIZE, SPRITE_SIZE));
                spr.setPosition(x * TILE, y * TILE);
                spr.setScale((float)TILE / SPRITE_SIZE, (float)TILE / SPRITE_SIZE);
                window.draw(spr);
            }
        }
    }
}

// --- Random "AI" move generator for black ---
void engineMove() {
    std::vector<std::pair<sf::Vector2i, sf::Vector2i>> moves;

    for (int sy = 0; sy < 8; sy++) {
        for (int sx = 0; sx < 8; sx++) {
            if (board[sy][sx] < 0) {
                for (int ey = 0; ey < 8; ey++) {
                    for (int ex = 0; ex < 8; ex++) {
                        if (isMoveLegal(sx, sy, ex, ey))
                            moves.push_back({{sx, sy}, {ex, ey}});
                    }
                }
            }
        }
    }

    if (!moves.empty()) {
        auto m = moves[rand() % moves.size()];
        board[m.second.y][m.second.x] = board[m.first.y][m.first.x];
        board[m.first.y][m.first.x] = 0;
    }
}
int pieceToValue(char c) { // convert fen peices to number
    switch (std::tolower(c)) {
        case 'p': return 1;
        case 'n': return 2;
        case 'b': return 3;
        case 'r': return 4;
        case 'q': return 5;
        case 'k': return 6;
        default: return 0;
    }
}

void load_fen_game(const std::string& fen) {
    int row = 0, col = 0;
    for (char c : fen) {
         if (c == '/') { // check for new row
            row++;
            col = 0;
        } 
         else if (std::isdigit(c)) {
            col += c - '0'; // add empty squares
        } 
    
     else {
        int val = pieceToValue(c);
        if (std::isupper(c))
            board[row][col] = val;    // white piece
        else
            board[row][col] = -val;   // black piece
        col++;
    }
  }
}

std::vector<std::string> read_file_as_string(const std::string& name) { // return every line of a file as a vector
    std::ifstream inputFile(name);
    std::vector<std::string> text;
     if (!inputFile.is_open()) {std::cerr << "ERROR" << std::endl;}
     std::string line;
     while (std::getline(inputFile, line)) {
        text.push_back(line);
     }

    inputFile.close();

    return text;
}

int main() {
    srand(time(0));
    sf::RenderWindow window(sf::VideoMode(WIDTH, HEIGHT), "SFML Chess Game");
    sf::Texture tex;
    if (!tex.loadFromFile(SPRITESHEET)) {
        std::cerr << "Error loading " << SPRITESHEET << "\n";
        return 1;
    }

    sf::Vector2i selected(-1, -1);

    while (window.isOpen()) {
        sf::Event e;
        while (window.pollEvent(e)) {
            if (e.type == sf::Event::Closed) window.close();

            if (e.type == sf::Event::MouseButtonPressed && e.mouseButton.button == sf::Mouse::Left && isWhiteTurn) {
                sf::Vector2i click = getTileUnderMouse(window);
                if (selected.x == -1) {
                    if (board[click.y][click.x] > 0) selected = click;
                } else {
                    if (isMoveLegal(selected.x, selected.y, click.x, click.y)) {
                        board[click.y][click.x] = board[selected.y][selected.x];
                        board[selected.y][selected.x] = 0;
                        isWhiteTurn = false;
                    }
                    selected = {-1, -1};
                }
            }
        }

        if (!isWhiteTurn) {
            engineMove();
            isWhiteTurn = true;
        }

        window.clear();
        drawBoard(window, tex);
        window.display();
    }
    return 0;
}
