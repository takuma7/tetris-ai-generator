#define BOARD_W (12)
#define BOARD_H (25)

typedef struct Position_t{
    int x;
    int y;
} Position;

typedef struct Block_t{
    int rot;
    Position ps[3];
} Block;

typedef struct Status_t{
    Position p;
    int type;
    int rot;
} Status;

/**
 * テトリミノが指定された位置に配置可能かどうかを調べる
 *  @param board テトリスのゲーム盤
 *  @param s 現在の状態
 *  @return 配置可能であればtrue, 不可能であればfalseを返す
 */
int is_putable_block(int board[BOARD_W][BOARD_H], Status s);

/**
 * テトリミノを現在の場所に配置する
 *  @param board テトリスのゲーム盤
 *  @param s 現在の状態
 *  @return 配置に成功すればtrue, 壁や他のテトリミノの影響で失敗すればfalseを返す
 */
int put_block(int board[BOARD_W][BOARD_H], Status s);

/**
 * @param board テトリスのゲーム盤
 * @return クリアできたラインの数を返す
 */
int clear_line(int board[BOARD_W][BOARD_H]);

void remove_block(int board[BOARD_W][BOARD_H], Status s);
void fill(int board[BOARD_W][BOARD_H], int c1, int c2, int x, int y);
void make_hole_visible(int board[BOARD_W][BOARD_H]);
void make_hole_invisible(int board[BOARD_W][BOARD_H]);
int* examine(int board[BOARD_W][BOARD_H], Status s);
double calc_score(int board[BOARD_W][BOARD_H], Status s, double e[4]);
void print_board(int board[BOARD_W][BOARD_H]);
double tetris(double e[4]);
double tetris_test(double e[4]);