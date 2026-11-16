#include <chrono>
#include <iomanip>
#include <iostream>
#include <limits>
#include <queue>
#include <string>
#include <utility>
#include <vector>

using namespace std;
using Clock = chrono::high_resolution_clock;

struct SearchResult {
    bool found = false;
    int visitedCells = 0;
    int pathlength = 0;
    double times = 0.0;
    vector<vector<bool>> path;
};

bool findStartAndEnd(const vector<string>& maze, pair<int, int>& start, pair<int, int>& finish) {
    int startCnt = 0;
    int endCnt = 0;
    start = {-1, -1};
    finish = {-1, -1};

    for (int r = 0; r < static_cast<int>(maze.size()); ++r) {
        for (int c = 0; c < static_cast<int>(maze[r].size()); ++c) {
            if (maze[r][c] == 'S') {
                start = {r, c};
                ++startCnt;
            } else if (maze[r][c] == 'E') {
                finish = {r, c};
                ++endCnt;
            }
        }
    }

    return startCnt == 1 && endCnt == 1 && start.first != -1 && finish.first != -1;
}

bool isOpenCell(const vector<string>& maze, int r, int c) {
    int rows = static_cast<int>(maze.size());
    int cols = rows > 0 ? static_cast<int>(maze[0].size()) : 0;

    return r >= 0 && r < rows && c >= 0 && c < cols && maze[r][c] != '#';
}

bool dfsHelper(const vector<string>& maze,
               int r,
               int c,
               int endR,
               int endC,
               vector<vector<bool>>& visited,
               vector<vector<bool>>& path,
               int& visitedCells) {
    visited[r][c] = true;
    ++visitedCells;

    if (r == endR && c == endC) {
        path[r][c] = true;
        return true;
    }

    static const int dr[4] = {-1, 1, 0, 0};
    static const int dc[4] = {0, 0, -1, 1};

    for (int k = 0; k < 4; ++k) {
        int nr = r + dr[k];
        int nc = c + dc[k];

        if (isOpenCell(maze, nr, nc) && !visited[nr][nc]) {
            if (dfsHelper(maze, nr, nc, endR, endC, visited, path, visitedCells)) {
                path[r][c] = true;
                return true;
            }
        }
    }

    return false;
}

SearchResult solveDFS(const vector<string>& maze, const pair<int, int>& start, const pair<int, int>& finish) {
    int rows = static_cast<int>(maze.size());
    int cols = rows > 0 ? static_cast<int>(maze[0].size()) : 0;

    SearchResult result;
    result.path.assign(rows, vector<bool>(cols, false));
    vector<vector<bool>> visited(rows, vector<bool>(cols, false));

    auto begin = Clock::now();
    result.found = dfsHelper(maze, start.first, start.second, finish.first, finish.second, visited, result.path, result.visitedCells);
    auto end = Clock::now();
    result.times = chrono::duration<double, milli>(end - begin).count();

    if (result.found) {
        int cellsOnPath = 0;
        for (const auto& row : result.path) {
            for (bool cell : row) {
                if (cell) {
                    ++cellsOnPath;
                }
            }
        }
        result.pathlength = cellsOnPath - 1;
    }

    return result;
}

SearchResult solveBFS(const vector<string>& maze, const pair<int, int>& start, const pair<int, int>& finish) {
    int rows = static_cast<int>(maze.size());
    int cols = rows > 0 ? static_cast<int>(maze[0].size()) : 0;

    SearchResult result;
    result.path.assign(rows, vector<bool>(cols, false));
    vector<vector<bool>> visited(rows, vector<bool>(cols, false));
    vector<vector<pair<int, int>>> parent(rows, vector<pair<int, int>>(cols, {-1, -1}));
    queue<pair<int, int>> q;

    auto begin = Clock::now();
    q.push(start);
    visited[start.first][start.second] = true;

    static const int dr[4] = {-1, 1, 0, 0};
    static const int dc[4] = {0, 0, -1, 1};

    while (!q.empty()) {
        auto current = q.front();
        q.pop();

        int r = current.first;
        int c = current.second;
        ++result.visitedCells;

        if (r == finish.first && c == finish.second) {
            result.found = true;
            break;
        }

        for (int k = 0; k < 4; ++k) {
            int nr = r + dr[k];
            int nc = c + dc[k];

            if (isOpenCell(maze, nr, nc) && !visited[nr][nc]) {
                visited[nr][nc] = true;
                parent[nr][nc] = {r, c};
                q.push({nr, nc});
            }
        }
    }

    if (result.found) {
        pair<int, int> current = finish;
        while (current != start) {
            int r = current.first;
            int c = current.second;
            result.path[r][c] = true;
            current = parent[r][c];
        }
        result.path[start.first][start.second] = true;

        int cellsOnPath = 0;
        for (const auto& row : result.path) {
            for (bool cell : row) {
                if (cell) {
                    ++cellsOnPath;
                }
            }
        }
        result.pathlength = cellsOnPath - 1;
    }

    auto end = Clock::now();
    result.times = chrono::duration<double, milli>(end - begin).count();
    return result;
}

void displayMaze(const vector<string>& maze, const vector<vector<bool>>& path) {
    for (int r = 0; r < static_cast<int>(maze.size()); ++r) {
        for (int c = 0; c < static_cast<int>(maze[r].size()); ++c) {
            char ch = maze[r][c];
            if (path[r][c] && ch != 'S' && ch != 'E') {
                ch = '*';
            }
            cout << ch << ' ';
        }
        cout << '\n';
    }
}

void displayResult(const string& name, const vector<string>& maze, const SearchResult& result) {
    cout << "\n--- " << name << " ---\n";
    if (!result.found) {
        cout << "No path exists from S to E.\n";
    } else {
        cout << "Path found!\n";
        displayMaze(maze, result.path);
        cout << "Path length (moves): " << result.pathlength << '\n';
    }
    cout << "Visited cells: " << result.visitedCells << '\n';
    cout << fixed << setprecision(6)
         << "Execution time: " << result.times << " ms\n";
}

bool readCustomMaze(vector<string>& maze) {
    int rows, cols;
    cout << "Enter number of rows and columns (1 to 50): ";
    if (!(cin >> rows >> cols) || rows < 1 || rows > 50 || cols < 1 || cols > 50) {
        cout << "Invalid dimensions.\n";
        return false;
    }

    maze.assign(rows, "");
    cout << "Enter each row using only S, E, . and #.\n";
    for (int r = 0; r < rows; ++r) {
        cin >> maze[r];
        if (static_cast<int>(maze[r].size()) != cols) {
            cout << "Each row must contain exactly " << cols << " characters.\n";
            return false;
        }

        for (char ch : maze[r]) {
            if (ch != 'S' && ch != 'E' && ch != '.' && ch != '#') {
                cout << "Invalid character in maze.\n";
                return false;
            }
        }
    }

    pair<int, int> start, finish;
    if (!findStartAndEnd(maze, start, finish)) {
        cout << "Maze must contain exactly one S and one E.\n";
        return false;
    }

    return true;
}

int main() {
    vector<string> maze = {
        "S.#..",
        "..#.#",
        "#...#",
        ".##..",
        "....E"
    };

    int choice = 0;
    do {
        cout << "\n====================================\n";
        cout << "MAZE SOLVER: DFS vs BFS\n";
        cout << "====================================\n";
        cout << "1. Load sample maze\n";
        cout << "2. Enter a custom maze\n";
        cout << "3. Display current maze\n";
        cout << "4. Solve using DFS (Backtracking)\n";
        cout << "5. Solve using BFS (Shortest Path)\n";
        cout << "6. Compare DFS and BFS\n";
        cout << "7. Exit\n";
        cout << "Enter your choice: ";

        if (!(cin >> choice)) {
            cout << "Please enter a valid menu number.\n";
            return 0;
        }

        pair<int, int> start, finish;

        switch (choice) {
            case 1:
                maze = {
                    "S.#..",
                    "..#.#",
                    "#...#",
                    ".##..",
                    "....E"
                };
                cout << "Sample maze loaded.\n";
                break;
            case 2:
                readCustomMaze(maze);
                break;
            case 3:
                cout << "\nCurrent maze:\n";
                for (const string& row : maze) {
                    cout << row << '\n';
                }
                break;
            case 4:
            case 5:
            case 6:
                if (!findStartAndEnd(maze, start, finish)) {
                    cout << "Invalid maze: it needs exactly one S and one E.\n";
                    break;
                }

                if (choice == 4) {
                    SearchResult dfsResult = solveDFS(maze, start, finish);
                    displayResult("DFS (Backtracking)", maze, dfsResult);
                } else if (choice == 5) {
                    SearchResult bfsResult = solveBFS(maze, start, finish);
                    displayResult("BFS (Shortest Path)", maze, bfsResult);
                } else {
                    SearchResult dfsResult = solveDFS(maze, start, finish);
                    SearchResult bfsResult = solveBFS(maze, start, finish);
                    displayResult("DFS (Backtracking)", maze, dfsResult);
                    displayResult("BFS (Shortest Path)", maze, bfsResult);
                    cout << "\n--- COMPARISON ---\n";
                    cout << "DFS path length: " << dfsResult.pathlength << '\n';
                    cout << "BFS path length: " << bfsResult.pathlength << '\n';
                    if (bfsResult.found && (!dfsResult.found || bfsResult.pathlength <= dfsResult.pathlength)) {
                        cout << "BFS provides a shortest path.\n";
                    }
                    cout << "Timing varies with machine and input size.\n";
                }
                break;
            case 7:
                cout << "Exiting Maze Solver. Goodbye!\n";
                break;
            default:
                cout << "Invalid choice. Try again.\n";
                break;
        }
    } while (choice != 7);

    return 0;
}
