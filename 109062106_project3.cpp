#include <iostream>
#include <fstream>
#include <array>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <climits>
#define predict_step 5
#define corner 100
#define corner_adjacent -15
#define outside_edge 5
#define inside_edge -5
using namespace std;

struct Point {
    int x, y;
    Point() : Point(0, 0) {}
    Point(int x, int y):x(x), y(y){}
    bool operator==(const Point& rhs) const {
		return x == rhs.x && y == rhs.y;
	}
	bool operator!=(const Point& rhs) const {
		return !operator==(rhs);
	}
	Point operator+(const Point& rhs) const {
		return Point(x + rhs.x, y + rhs.y);
	}
	Point operator-(const Point& rhs) const {
		return Point(x - rhs.x, y - rhs.y);
	}
};

int player;
const int SIZE = 8;
std::array<std::array<int, SIZE>, SIZE> board;
std::vector<Point> next_valid_spots;
Point ans(3, 3);

class State{
public:
    enum SPOT_STATE {
        EMPTY = 0,
        BLACK = 1,
        WHITE = 2
    };
    array<array<int, 8>, 8> Board;
    vector<Point> Next_valid_spots;
    array<int, 3> disc_count;
    int Player;
    int alpha, beta;
    int heuristic;
    int step;
    const array<Point, 8> directions{{
        Point(-1, -1), Point(-1, 0), Point(-1, 1),
        Point(0, -1), /*{0, 0}, */Point(0, 1),
        Point(1, -1), Point(1, 0), Point(1, 1)
    }};
    State(array<array<int, 8>, 8>& b, vector<Point>& n, int p):Player(p){
        step = 0;
        disc_count[EMPTY] = disc_count[BLACK] = disc_count[WHITE] = 0;
        alpha = INT_MIN, beta = INT_MAX;
        for(int i=0;i<8;i++){
            for(int j=0;j<8;j++){
                Board[i][j] = b[i][j];
                if(Board[i][j]==EMPTY) disc_count[EMPTY]++;
                else if(Board[i][j]==BLACK) disc_count[BLACK]++;
                else if(Board[i][j]==WHITE) disc_count[WHITE]++;
            }
        }
        Next_valid_spots = n;
        setHeuristic();
    }

    vector<Point> get_valid_spots() const {
        vector<Point> valid_spots;
        for (int i = 0; i < SIZE; i++) {
            for (int j = 0; j < SIZE; j++) {
                Point p = Point(i, j);
                if (Board[i][j] != EMPTY)
                    continue;
                if (is_spot_valid(p))
                    valid_spots.push_back(p);
            }
        }
        return valid_spots;
    }
    
    void update(Point p){
        set_disc(p, Player);
        disc_count[Player]++;
        disc_count[EMPTY]--;
        flip_discs(p);
        setHeuristic();
        Player = get_next_player(Player);
        Next_valid_spots = get_valid_spots();
        step++;
    }
    // TODO
    void setHeuristic(){
        heuristic = disc_count[BLACK] - disc_count[WHITE]
                    + (find_corner(BLACK) - find_corner(WHITE)) * corner
                    + (find_corner_adjacent(BLACK) - find_corner_adjacent(WHITE)) * corner_adjacent
                    + (find_outside_edge(BLACK) - find_outside_edge(WHITE)) * outside_edge
                    + (find_inside_edge(BLACK) - find_inside_edge(WHITE)) * inside_edge;
    }
private:
    int get_next_player(int P) const {
        return 3 - P;
    }
    bool is_spot_on_board(Point p) const {
        return 0 <= p.x && p.x < SIZE && 0 <= p.y && p.y < SIZE;
    }
    int get_disc(Point p) const {
        return Board[p.x][p.y];
    }
    void set_disc(Point p, int disc) {
        Board[p.x][p.y] = disc;
    }
    bool is_disc_at(Point p, int disc) const {
        if (!is_spot_on_board(p))
            return false;
        if (get_disc(p) != disc)
            return false;
        return true;
    }
    bool is_spot_valid(Point center) const {
        if (get_disc(center) != EMPTY)
            return false;
        for (Point dir: directions) {
            // Move along the direction while testing.
            Point p = center + dir;
            if (!is_disc_at(p, get_next_player(Player)))
                continue;
            p = p + dir;
            while (is_spot_on_board(p) && get_disc(p) != EMPTY) {
                if (is_disc_at(p, Player))
                    return true;
                p = p + dir;
            }
        }
        return false;
    }
    void flip_discs(Point center) {
        for (Point dir: directions) {
            // Move along the direction while testing.
            Point p = center + dir;
            if (!is_disc_at(p, get_next_player(Player)))
                continue;
            vector<Point> discs({p});
            p = p + dir;
            while (is_spot_on_board(p) && get_disc(p) != EMPTY) {
                if (is_disc_at(p, Player)) {
                    for (Point s: discs) {
                        set_disc(s, Player);
                    }
                    disc_count[Player] += discs.size();
                    disc_count[get_next_player(Player)] -= discs.size();
                    break;
                }
                discs.push_back(p);
                p = p + dir;
            }
        }
    }
    int find_corner(int disc){
        int cnt = 0;
        if(Board[0][7]==disc) cnt++;
        if(Board[0][0]==disc) cnt++;
        if(Board[7][0]==disc) cnt++;
        if(Board[7][7]==disc) cnt++;
        return cnt;
    }
    int find_corner_adjacent(int disc){
        int cnt = 0;
        if(Board[0][1]==disc) cnt++;
        if(Board[1][0]==disc) cnt++;
        if(Board[1][1]==disc) cnt++;
        if(Board[0][6]==disc) cnt++;
        if(Board[1][6]==disc) cnt++;
        if(Board[1][7]==disc) cnt++;
        if(Board[6][0]==disc) cnt++;
        if(Board[6][1]==disc) cnt++;
        if(Board[7][1]==disc) cnt++;
        if(Board[6][6]==disc) cnt++;
        if(Board[6][7]==disc) cnt++;
        if(Board[7][6]==disc) cnt++;
        return cnt;
    }
    int find_outside_edge(int disc){
        int cnt = 0;
        if(Board[0][2]==disc) cnt++;
        if(Board[0][3]==disc) cnt++;
        if(Board[0][4]==disc) cnt++;
        if(Board[0][5]==disc) cnt++;
        if(Board[2][0]==disc) cnt++;
        if(Board[3][0]==disc) cnt++;
        if(Board[4][0]==disc) cnt++;
        if(Board[5][0]==disc) cnt++;
        if(Board[7][2]==disc) cnt++;
        if(Board[7][3]==disc) cnt++;
        if(Board[7][4]==disc) cnt++;
        if(Board[7][5]==disc) cnt++;
        if(Board[2][7]==disc) cnt++;
        if(Board[3][7]==disc) cnt++;
        if(Board[4][7]==disc) cnt++;
        if(Board[5][7]==disc) cnt++;
        return cnt;
    }
    int find_inside_edge(int disc){
        int cnt = 0;
        if(Board[1][2]==disc) cnt++;
        if(Board[1][3]==disc) cnt++;
        if(Board[1][4]==disc) cnt++;
        if(Board[1][5]==disc) cnt++;
        if(Board[2][1]==disc) cnt++;
        if(Board[3][1]==disc) cnt++;
        if(Board[4][1]==disc) cnt++;
        if(Board[5][1]==disc) cnt++;
        if(Board[6][2]==disc) cnt++;
        if(Board[6][3]==disc) cnt++;
        if(Board[6][4]==disc) cnt++;
        if(Board[6][5]==disc) cnt++;
        if(Board[2][6]==disc) cnt++;
        if(Board[3][6]==disc) cnt++;
        if(Board[4][6]==disc) cnt++;
        if(Board[5][6]==disc) cnt++;
        return cnt;
    }
};

int alpha_beta_pruning(State cur){
    if(cur.step==predict_step || cur.Next_valid_spots.size()==0) return cur.heuristic;
    if(cur.Player==1){ // max node(BLACK)
        int bestval = INT_MIN;
        for(auto it:cur.Next_valid_spots){
            State nextState = cur;
            nextState.update(it);
            int childval = alpha_beta_pruning(nextState);
            if(childval>bestval){
                bestval = childval;
                if(cur.step==0) ans = it;
            }
            cur.alpha = max(cur.alpha, bestval);
            if(cur.alpha>=cur.beta) break;
        }
        return bestval;
    }
    else if(cur.Player==2){ // min node(WHITE)
        int bestval = INT_MAX;
        for(auto it:cur.Next_valid_spots){
            State nextState = cur;
            nextState.update(it);
            int childval = alpha_beta_pruning(nextState);
            if(childval<bestval){
                bestval = childval;
                if(cur.step==0) ans = it;
            }
            cur.beta = min(cur.beta, bestval);
            if(cur.alpha>=cur.beta) break;
        }
        return bestval;
    }
}

void read_board(std::ifstream& fin) {
    fin >> player;
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            fin >> board[i][j];
        }
    }
}

void read_valid_spots(std::ifstream& fin) {
    int n_valid_spots;
    fin >> n_valid_spots;
    int x, y;
    for (int i = 0; i < n_valid_spots; i++) {
        fin >> x >> y;
        next_valid_spots.push_back({x, y});
    }
}

void write_valid_spot(std::ofstream& fout) {
    // int n_valid_spots = next_valid_spots.size();
    // srand(time(NULL));
    // // Choose random spot. (Not random uniform here)
    // int index = (rand() % n_valid_spots);
    // Point p = next_valid_spots[index];
    // // Remember to flush the output to ensure the last action is written to file.
    State s(board, next_valid_spots, player);
    alpha_beta_pruning(s);
    if(ans.x!=3 || ans.y!=3) fout << ans.x << " " << ans.y << std::endl;
    fout.flush();
}

int main(int, char** argv) {
    std::ifstream fin(argv[1]);
    std::ofstream fout(argv[2]);
    read_board(fin);
    read_valid_spots(fin);
    write_valid_spot(fout);
    fin.close();
    fout.close();
    return 0;
}
