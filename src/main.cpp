#include <SFML/Graphics.hpp>
#include <iostream>
#include <vector>
#include <utility>
#include <algorithm>
#include <cstring>
#include <chrono>

using namespace std;
using namespace sf;

class Board {
public:
    static const int SIZE = 56;
    bool grid[SIZE][SIZE];
    pair<int, int> next_free;
    int space;

    Board() {
        memset(grid, true, sizeof(grid));  // True if unoccupied
        next_free = {0, 0};  // (x, y), the top leftmost free cell
        space = SIZE * SIZE;  // free space on this board
    }

    bool does_fit(pair<int, int> piece) {
        if (next_free.first + piece.first > SIZE || next_free.second + piece.second > SIZE) {
            return false;
        }

        for (int w = 0; w < piece.first; ++w) {
            for (int h = 0; h < piece.second; ++h) {
                if (!grid[next_free.second + h][next_free.first + w]) {
                    return false;
                }
            }
        }

        return true;
    }

    pair<int, int> insert(pair<int, int> piece) {
        pair<int, int> position = next_free;

        for (int x = 0; x < piece.first; ++x) {
            for (int y = 0; y < piece.second; ++y) {
                grid[next_free.second + y][next_free.first + x] = false;
            }
        }

        bool updated = false;
        for (int y = 0; y < SIZE; ++y) {
            for (int x = 0; x < SIZE; ++x) {
                if (!updated && grid[y][x]) {
                    next_free = {x, y};
                    updated = true;
                }
            }
        }

        space -= piece.first * piece.second;

        return position;
    }

    Board copy() const {
        Board copied_board;
        memcpy(copied_board.grid, grid, sizeof(grid));
        copied_board.next_free = next_free;
        copied_board.space = space;
        return copied_board;
    }
};

vector<Color> generate_colors(int num_colors) {
    vector<Color> colors = {
        Color::Red, Color::Green, Color::Blue, Color::Yellow,
        Color::Magenta, Color::Cyan, Color::White, Color::Black,
        Color(255, 165, 0), // Orange
        Color(128, 0, 128), // Purple
        Color(0, 128, 0), // Dark Green
        Color(128, 128, 0) // Olive
    };

    return colors;
}

vector<pair<pair<int, int>, pair<int, int>>> get_solution(Board board, vector<pair<int, int>> remaining, vector<pair<pair<int, int>, pair<int, int>>> positions, int& iteration_count, RenderWindow& window, float cell_size, const vector<Color>& colors) {
    iteration_count++;

    if (board.space == 0) {
        return positions;
    }

    for (const auto& piece : remaining) {
        for (bool isRotated : {false, true}) {
            pair<int, int> rotated_piece = isRotated ? make_pair(piece.second, piece.first) : piece;
            if (board.does_fit(rotated_piece)) {
                Board new_board = board.copy();
                vector<pair<int, int>> new_remaining = remaining;
                pair<int, int> position = new_board.insert(rotated_piece);
                new_remaining.erase(remove(new_remaining.begin(), new_remaining.end(), piece), new_remaining.end());
                positions.push_back(make_pair(rotated_piece, position));

                // Draw the board state
                window.clear(Color::White);
                RectangleShape cell(Vector2f(cell_size, cell_size));

                // Draw the current board
                for (int y = 0; y < Board::SIZE; ++y) {
                    for (int x = 0; x < Board::SIZE; ++x) {
                        cell.setFillColor(new_board.grid[y][x] ? Color::White : Color::Black);
                        cell.setPosition(Vector2f(x * cell_size, y * cell_size));
                        window.draw(cell);
                    }
                }

                // Draw the placed pieces with different colors
                for (size_t i = 0; i < positions.size(); ++i) {
                    cell.setFillColor(colors[i % colors.size()]);
                    for (int x = 0; x < positions[i].first.first; ++x) {
                        for (int y = 0; y < positions[i].first.second; ++y) {
                            cell.setPosition(Vector2f((positions[i].second.first + x) * cell_size, (positions[i].second.second + y) * cell_size));
                            window.draw(cell);
                        }
                    }
                }

                window.display();

                auto solution = get_solution(new_board, new_remaining, positions, iteration_count, window, cell_size, colors);
                if (!solution.empty()) {
                    return solution;
                }

                positions.pop_back();
            }
        }
    }

    return {};
}

void visualize_solution(Board board, vector<pair<int, int>> all_pieces) {
    RenderWindow window(VideoMode({800, 800}), "Edison Puzzle Visualization");

    float cell_size = 800.0f / Board::SIZE;
    int iteration_count = 0;
    vector<Color> colors = generate_colors(all_pieces.size());

    // Start timing
    auto start = chrono::high_resolution_clock::now();

    auto solution = get_solution(board, all_pieces, {}, iteration_count, window, cell_size, colors);

    // End timing
    auto end = chrono::high_resolution_clock::now();
    auto duration = chrono::duration_cast<chrono::milliseconds>(end - start).count();
    cout << "Time taken: " << duration << " milliseconds" << endl;

    if (solution.empty()) {
        cout << "No solution found." << endl;
    } else {
        cout << "Solution found:" << endl;
        for (const auto& pos : solution) {
            cout << "Piece (" << pos.first.first << ", " << pos.first.second << ") at (" << pos.second.first << ", " << pos.second.second << ")" << endl;
        }
    }

    cout << "Total number of iterations: " << iteration_count << endl;

    // Keep the window open to display the final solution
    while (window.isOpen()) {
        while (const optional<Event> event = window.pollEvent()) {
            if (event->is<Event::Closed>()) {
                window.close();
            }
        }
    }
}

int main() {
    vector<pair<int, int>> all_pieces = {
        {28, 14}, {21, 18}, {21, 18}, {21, 14}, {21, 14},
        {32, 11}, {32, 10}, {28, 7}, {28, 6}, {17, 14},
        {14, 4}, {10, 7}
    };

    Board board;
    visualize_solution(board, all_pieces);

    return 0;
}
