#include "raylib.h"   //includes drawing, music and extras used for visual fucntions
#include <vector>     //dynamic 1D/2D arrays (used for maze grid).
#include <queue>      //used for BFS algorithm.
#include <stack>      //used for DFS algorithm.
#include <algorithm>    //for functions like reverse().
#include <unordered_set>  //used to store visited cells quickly (O(1) lookup).
#include <string>           //string thingy
#include <memory>

using namespace std;

// Maze class
class Maze {
private:
    vector<vector<int>> grid;   //maze grid made through Implicit graph as the movement is in 4 directions
    int width, height;          //maze dimensions
    Vector2 playerPos;          //player's location made through vector that is 2-dimensional
    Vector2 exitPos;            //ending point of maze 'E'         

public:
    Maze(int w, int h) : width(w), height(h) {          //Creates maze of size w × h
        grid.resize(height, vector<int>(width, 1));      //Fills grid with 1s(walls)
        playerPos = { 1.0f, 1.0f };                      // Sets player at(1, 1)
        exitPos = { (float)width - 2.0f, (float)height - 2.0f };        // Sets exit at bottom - right corner
        Generate();                                 // Calls Generate() to build maze
    }

    void Generate() {
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) grid[y][x] = 1;        // Fill entire grid with walls.
        }
        for (int y = 1; y < height - 1; y += 2) {
            for (int x = 1; x < width - 1; x += 2) {                //Use a pattern : every 2nd cell becomes a path(0).
                grid[y][x] = 0;
                if (x + 2 < width - 1) grid[y][x + 1] = 0;          //Also clear cells to the right and below to make corridors.
                if (y + 2 < height - 1) grid[y + 1][x] = 0;
            }
        }
        grid[1][1] = 0;
        grid[height - 2][width - 2] = 0;                //Ensure start (1,1) and exit (width-2, height-2) are open.
        playerPos = { 1.0f, 1.0f };
        exitPos = { (float)width - 2.0f, (float)height - 2.0f };        //Reset player and exit positions.
    }

    void ShuffleWalls() {
        for (int y = 1; y < height - 1; y++) {
            for (int x = 1; x < width - 1; x++) {
                if ((x == 1 && y == 1) || (x == width - 2 && y == height - 2)) continue;        //Keeps start & exit safe.
                if (grid[y][x] == 1) {
                    if (GetRandomValue(0, 9) == 0) grid[y][x] = 0;
                }                                                                     //Randomly removes some walls or adds new walls to make the maze dynamic.
                else {
                    if (GetRandomValue(0, 14) == 0) grid[y][x] = 1;
                }
            }
        }
    }

    void Draw() const {
        const int cellSize = 30;
        for (int y = 0; y < height; y++) {              //row visit
            for (int x = 0; x < width; x++) {           //column visit    
                Color color = (grid[y][x] == 1) ? BLUE : BLACK;       //agar 1 araha hai tou wall hai in blue otherwise it is an empty path              
                DrawRectangle((int)(x * cellSize), (int)(y * cellSize), cellSize, cellSize, color);         //This draws the actual box of the cell
                DrawRectangleLines((int)(x * cellSize), (int)(y * cellSize), cellSize, cellSize, DARKGRAY);     //This draws the border around the cell
            }
        }
        DrawRectangle((int)(playerPos.x * cellSize), (int)(playerPos.y * cellSize), cellSize, cellSize, GREEN);     //player's structure
        DrawText("P", (int)(playerPos.x * cellSize) + 10, (int)(playerPos.y * cellSize) + 5, 20, WHITE);            //player's name P written    
        DrawRectangle((int)(exitPos.x * cellSize), (int)(exitPos.y * cellSize), cellSize, cellSize, RED);           //Exit's structure
        DrawText("E", (int)(exitPos.x * cellSize) + 10, (int)(exitPos.y * cellSize) + 5, 20, WHITE);            //Exit's name E written  
    }

    void DrawCell(int x, int y, Color color) const {                //This is helper function to draw cells used in further classes
        const int cellSize = 30;
        DrawRectangle(x * cellSize, y * cellSize, cellSize, cellSize, color);
        DrawRectangleLines(x * cellSize, y * cellSize, cellSize, cellSize, DARKGRAY);                                                                                       
    }

    bool IsValidMove(int x, int y) const {
        return (x >= 0 && x < width && y >= 0 && y < height && grid[y][x] == 0);       //Checks if player can move to a cell: inside maze bounds and not a wall.
    }

    bool MovePlayer(int dx, int dy) {       //dx is change in x and dy is change in y
        int newX = (int)playerPos.x + dx;    //yani current jahan player horizontally majood hai uss mein new horizontal pos (dx) add kardo 
        int newY = (int)playerPos.y + dy;     //yani current jahan player vertical par hai uss mein new vertical pos (dy) add kardo   
        if (IsValidMove(newX, newY)) {          //to check ke movement wall ya bahar to nhi
            playerPos.x = (float)newX;          //update column position
            playerPos.y = (float)newY;          //upadate row position
            return true;
        }
        return false;
    }
    //Getter functions
    Vector2 GetPlayerPos() const { return playerPos; }
    Vector2 GetExitPos() const { return exitPos; }
    const vector<vector<int>>& GetGrid() const { return grid; }             
    int GetWidth() const { return width; }
    int GetHeight() const { return height; }
};

// Pathfinder class
struct Node { 
    int x, y; 
    Node* parent; 
    Node(int x, int y, Node* parent = nullptr) : x(x), y(y), parent(parent) {} 
};
class Pathfinder {
private:
    vector<Vector2> currentPath;
    void FreeQueue(queue<Node*>& q) { while (!q.empty()) { delete q.front(); q.pop(); } }
    void FreeStack(stack<Node*>& s) { while (!s.empty()) { delete s.top(); s.pop(); } }
public:
    vector<Vector2> FindPathBFS(const Maze& maze, Vector2 start, Vector2 end) {         //to find the shortest route
        currentPath.clear();                    //clear any previous route if any
        vector<Vector2> path;                   //any path ke liye
        queue<Node*> q;                         //we will use queue for BFS
        unordered_set<int> visited;             //To mark as it visited kiuke is ka O(1) hai baqi vector ya array ka O(n)
                                                //It only stores UNIQUE values So it automatically prevents duplicate visits. and order matter nhi karta warna ordered set ka look 
                                                //up time O(logn) hota 

        auto getHash = [&maze](int x, int y) {          //width =10 and cell(3,2) so unique number hogaya 23 jo sirf iss cell ka hoga
            return y * maze.GetWidth() + x; 
            };
        //Maze x,y ki form mein hota hai hota hai har cell ka address hota hai but unordered set mein hum log values in pairs store nhi karsakty tou we use this function. 
        //ye function un dono ko aik unique number banadega taake woh unordered set mein store hosake.

        Node* startNode = new Node((int)start.x, (int)start.y);             //starting position ke liye aik new node
        q.push(startNode);          //push the start position
        visited.insert(getHash((int)start.x, (int)start.y));        //mark that position as visited    
        Node* found = nullptr;                                      //It will store Exit ka node
        while (!q.empty()) {                            //keep searching until the queue becomes empty
            Node* current = q.front(); 
            q.pop();
            if (current->x == (int)end.x && current->y == (int)end.y) { found = current; break; }         //Agar yeh cell exit hai toh hum BFS rok dete hain path mil chuka hai.      
            int dx[] = { 0,1,0,-1 };        //ye dono 4 directions ko represent kar rahe hn up, right, down, left
            int dy[] = { -1,0,1,0 };    
            for (int i = 0; i < 4; i++) {           //4 direction mein searching karo
                int newX = current->x + dx[i], newY = current->y + dy[i];       //neighbor cell ko calculate karo
                if (maze.IsValidMove(newX, newY) && visited.find(getHash(newX, newY)) == visited.end()) {      //check karo ke neighbor cell wall tou nhi ya visited tou nhi?     
                    Node* neighbor = new Node(newX, newY, current);     //ab neighbor ki position ko store karlo
                    q.push(neighbor);                       //push neighbor in BFS Queue
                    visited.insert(getHash(newX, newY));        //neighbor ko mark as visited kardo
                }
            }
        }
        if (found) {
            Node* node = found;         //ab jaise he path mil jayega
            while (node) { 
                path.push_back({ (float)node->x,(float)node->y });          //exit se player ki position tak path trace karo
                    node = node->parent; }                                  //by using har node ke parent pointers
            reverse(path.begin(), path.end());                      //kiuke hum log exit to start karrahy thy so ab start to exit kardo path ko
                currentPath = path;                 //save kardo current path ko
        }
        FreeQueue(q);           //free kardo queue ki memory ko
        return path;
    }
    vector<Vector2> FindPathDFS(const Maze& maze, Vector2 start, Vector2 end) {
        currentPath.clear();
        vector<Vector2> path; 
        stack<Node*> s; 
        unordered_set<int> visited;
        auto getHash = [&maze](int x, int y) { return y * maze.GetWidth() + x; };
        Node* startNode = new Node((int)start.x, (int)start.y);
        s.push(startNode); 
        visited.insert(getHash((int)start.x, (int)start.y));
        Node* found = nullptr;
        while (!s.empty()) {
            Node* current = s.top(); s.pop();
            if (current->x == (int)end.x && current->y == (int)end.y) { 
                found = current; 
                break; }
            int dx[] = { 0,1,0,-1 }; 
            int dy[] = { -1,0,1,0 };
            for (int i = 0; i < 4; i++) {
                int newX = current->x + dx[i], newY = current->y + dy[i];
                if (maze.IsValidMove(newX, newY) && visited.find(getHash(newX, newY)) == visited.end()) {
                    Node* neighbor = new Node(newX, newY, current); 
                    s.push(neighbor); 
                    visited.insert(getHash(newX, newY));
                }
            }
        }
        if (found) {
            Node* node = found;
            while (node) { 
                path.push_back({ (float)node->x,(float)node->y }); 
                node = node->parent; 
            }
            reverse(path.begin(), path.end()); 
            currentPath = path;
        }
        FreeStack(s); 
        return path;
    }
    void DrawPath(const Maze& maze) const { 
        for (const auto& point : currentPath) maze.DrawCell((int)point.x, (int)point.y,{ 255,255,0,100 });
    }
    void ClearPath() { 
        currentPath.clear(); 
    }
    const vector<Vector2>& GetCurrentPath() const { return currentPath; }
};

// Game class
class Game {
private:
    Maze maze;
    Pathfinder pathfinder;      //Ye pathfinding logic rakhta hai(BFS / DFS).
    bool useBFS;        //Agar true BFS use hoga, false DFS use hoga.
    bool showPath;      //Agar sach path possible hai tou path screen pe draw hoga.
    int shuffleTimer;    //Maze me walls kab shuffle karni hai, track karne ke liye counter.
    bool gameWon;
    bool gameOver;  //Game state ke liye
    int timeLeft;       //Kitna time bacha hai
    int frameCounter;       //Frame update ke liye counter. Screen update par badalta hai
    int bumpCount;

    int shuffleSpeed; // frames between shuffles
    Music bgMusic;    // background music object

public:
    Game() : maze(15, 15), useBFS(true), showPath(true), shuffleTimer(0),
        gameWon(false), gameOver(false), timeLeft(90), frameCounter(0),
        bumpCount(0), shuffleSpeed(180) {
    }

    void Run() {
        const int cellSize = 30;
        int winW = maze.GetWidth() * cellSize + 500;            //window jo bani aati hai uski width
        int winH = maze.GetHeight() * cellSize + 200;           //window jo bani aati hai uski height
        InitWindow(winW, winH, "Digital Prison Escape - Hacker Maze");      

        InitAudioDevice(); // initialize audio
        bgMusic = LoadMusicStream("background.mp3"); // load music
        PlayMusicStream(bgMusic); // play music

        SetTargetFPS(60);
        if (showPath) pathfinder.FindPathBFS(maze, maze.GetPlayerPos(), maze.GetExitPos());

        while (!WindowShouldClose()) {
            UpdateMusicStream(bgMusic); // update music
            Update();
            Draw();
        }

        UnloadMusicStream(bgMusic); // unload music
        CloseAudioDevice();          // close audio
        CloseWindow();
    }

    void Update() {
        HandleInput();
        if (gameWon || gameOver) return;        //game khatam

        CheckWinCondition();
        frameCounter++;
        if (frameCounter >= 60) { timeLeft--; frameCounter = 0; if (timeLeft <= 0) gameOver = true; }

        shuffleTimer++;
        if (shuffleTimer >= shuffleSpeed) {
            maze.ShuffleWalls(); shuffleTimer = 0;
            if (showPath) {
                if (useBFS) pathfinder.FindPathBFS(maze, maze.GetPlayerPos(), maze.GetExitPos());
                else pathfinder.FindPathDFS(maze, maze.GetPlayerPos(), maze.GetExitPos());
            }
        }
    }

    void Draw() {
        BeginDrawing(); 
        ClearBackground(BLACK);
        maze.Draw();
        if (showPath) pathfinder.DrawPath(maze);

        DrawText("DIGITAL PRISON ESCAPE", 10, (int)(maze.GetHeight() * 30) + 10, 20, WHITE);
        DrawText("Use Arrow Keys to move", 10, (int)(maze.GetHeight() * 30) + 35, 18, WHITE);
        DrawText("B: BFS | D: DFS | P: Toggle Path | R: Reset", 10, (int)(maze.GetHeight() * 30) + 55, 16, WHITE);
        DrawText(TextFormat("Time Left: %d", timeLeft), 500, (int)(maze.GetHeight() * 30) + 60, 18, YELLOW);
        DrawText(TextFormat("Firewall Bumps: %d", bumpCount), 500, (int)(maze.GetHeight() * 30) + 32, 18, RED);
        const char* algoText = useBFS ? "BFS (Shortest Path)" : "DFS (Exploration)";
        DrawText(TextFormat("Algorithm: %s", algoText), 300, (int)(maze.GetHeight() * 30) + 12, 18, useBFS ? GREEN : ORANGE);

        const char* diffText;
        if (shuffleSpeed == 300) diffText = "Difficulty: Easy";     //easy=300
        else if (shuffleSpeed == 180) diffText = "Difficulty: Medium";      //medium=180
        else diffText = "Difficulty: Hard";                             //hard=60
        DrawText(diffText, 10, (int)(maze.GetHeight() * 30) + 90, 18, LIGHTGRAY);

        if (gameWon) {
            DrawRectangle((GetScreenWidth() / 2) - 200, (GetScreenHeight() / 2) - 50, 400, 100, { 0,0,0,200 });
            DrawText("ESCAPE SUCCESSFUL!", (GetScreenWidth() / 2) - 130, (GetScreenHeight() / 2) - 30, 30, GREEN);
            DrawText("You hacked the digital prison!", (GetScreenWidth() / 2) - 150, (GetScreenHeight() / 2) + 0, 20, WHITE);
            int score = timeLeft * 10 - bumpCount * 2;
            DrawText(TextFormat("Score: %d", score), (GetScreenWidth() / 2) - 40, (GetScreenHeight() / 2) + 55, 18, LIGHTGRAY);
            DrawText("Press R to play again", (GetScreenWidth() / 2) - 90, (GetScreenHeight() / 2) + 30, 18, YELLOW);
        }
        if (gameOver && !gameWon) {
            DrawRectangle((GetScreenWidth() / 2) - 200, (GetScreenHeight() / 2) - 50, 400, 100, { 0,0,0,200 });
            DrawText("TIME'S UP!", (GetScreenWidth() / 2) - 80, (GetScreenHeight() / 2) - 20, 30, RED);
            DrawText("You failed to escape...", (GetScreenWidth() / 2) - 120, (GetScreenHeight() / 2) + 10, 20, WHITE);
            DrawText("Press R to try again", (GetScreenWidth() / 2) - 90, (GetScreenHeight() / 2) + 40, 18, YELLOW);
        }

        if ((gameWon || gameOver) && IsKeyPressed(KEY_ESCAPE)) {
            CloseWindow();
            exit(0);
        }

        int barX = 10, barY = (int)(maze.GetHeight() * 30) + 110, barWidth = 300, barHeight = 15;
        DrawRectangle(barX, barY, barWidth, barHeight, DARKGRAY);
        float timeRatio = (float)timeLeft / 90.0f;
        int fillWidth = (int)(barWidth * timeRatio);
        Color fillColor = (timeLeft > 30) ? GREEN : (timeLeft > 10 ? ORANGE : RED);
        DrawRectangle(barX, barY, fillWidth, barHeight, fillColor);
        DrawText("Time Remaining", barX + barWidth + 10, barY - 2, 16, WHITE);

        EndDrawing();
    }

    void HandleInput() {
        if (IsKeyPressed(KEY_UP) && !maze.MovePlayer(0, -1)) bumpCount++;
        if (IsKeyPressed(KEY_DOWN) && !maze.MovePlayer(0, 1)) bumpCount++;
        if (IsKeyPressed(KEY_LEFT) && !maze.MovePlayer(-1, 0)) bumpCount++;
        if (IsKeyPressed(KEY_RIGHT) && !maze.MovePlayer(1, 0)) bumpCount++;

        if (IsKeyPressed(KEY_B)) { useBFS = true; showPath = true; pathfinder.FindPathBFS(maze, maze.GetPlayerPos(), maze.GetExitPos()); }
        if (IsKeyPressed(KEY_D)) { useBFS = false; showPath = true; pathfinder.FindPathDFS(maze, maze.GetPlayerPos(), maze.GetExitPos()); }
        if (IsKeyPressed(KEY_P)) {
            showPath = !showPath;
            if (showPath) {
                if (useBFS) pathfinder.FindPathBFS(maze, maze.GetPlayerPos(), maze.GetExitPos());
                else pathfinder.FindPathDFS(maze, maze.GetPlayerPos(), maze.GetExitPos());
            }
            else pathfinder.ClearPath();
        }

        if (IsKeyPressed(KEY_E)) shuffleSpeed = 300;
        if (IsKeyPressed(KEY_M)) shuffleSpeed = 180;
        if (IsKeyPressed(KEY_H)) shuffleSpeed = 60;

        if (IsKeyPressed(KEY_R)) {
            maze.Generate(); pathfinder.ClearPath(); gameWon = false; gameOver = false;
            shuffleTimer = 0; timeLeft = 90; bumpCount = 0; frameCounter = 0;
            if (showPath) {
                if (useBFS) pathfinder.FindPathBFS(maze, maze.GetPlayerPos(), maze.GetExitPos());
                else pathfinder.FindPathDFS(maze, maze.GetPlayerPos(), maze.GetExitPos());
            }
        }
    }

    void CheckWinCondition() { 
        Vector2 p = maze.GetPlayerPos(), e = maze.GetExitPos(); 
        if ((int)p.x == (int)e.x && (int)p.y == (int)e.y) 
        gameWon = true; 
    }
};

int main() {
    Game game;
    game.Run();
    return 0;
}