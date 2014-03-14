#include "tetris.h"

const Block blocks[8] = {
    {1, {{ 0, 0}, { 0, 0}, { 0, 0}}},   //null（blocksのindex値をtypeとし、それでboardを埋めるためにindex=0のテトリミノはあってはならないのでダミーのnullテトリミノを用意しておく。使用はしない。）
    {2, {{0, -1},{0, 1}, {0 ,2}}},  // 棒形
    {4, {{0, -1},{0, 1}, {1 ,1}}},  // J字型
    {4, {{0, -1},{0, 1}, {-1,1}}},  // L字型
    {2, {{0, -1},{1, 0}, {1 ,1}}},  // Z字型
    {2, {{0, -1},{-1,0}, {-1,1}}},  // S字型
    {1, {{0,  1},{1, 0}, {1 ,1}}},  // 正方形
    {4, {{0, -1},{1, 0}, {-1 ,0}}},  // T字型
};

int is_putable_block(int board[BOARD_W][BOARD_H], Status s)
{
    if(board[s.p.x][s.p.y] != 0){
        return 0;
    }
    int i, j;
    for(i = 0; i < 3; i++) {
        int dx = blocks[s.type].ps[i].x;
        int dy = blocks[s.type].ps[i].y;
        int r = s.rot % blocks[s.type].rot;
        for(j = 0; j < r; j++) {
            int nx = dx, ny = dy;
            dx = ny; dy = -nx;
        }
        if(board[s.p.x + dx][s.p.y + dy] != 0) {
            return 0;
        }
    }
    return 1;
}

int put_block(int board[BOARD_W][BOARD_H], Status s)
{
    if(!is_putable_block(board, s)) return 0;

    int i, j;
    board[s.p.x][s.p.y] = s.type;
    for(i = 0; i < 3; i++) {
        int dx = blocks[s.type].ps[i].x;
        int dy = blocks[s.type].ps[i].y;
        int r = s.rot % blocks[s.type].rot;
        for(j = 0; j < r; j++) {
            int nx = dx, ny = dy;
            dx = ny; dy = -nx;
        }
        board[s.p.x + dx][s.p.y + dy] = s.type;
    }
    return 1;
}

void remove_block(int board[BOARD_W][BOARD_H], Status s)
{
    int i, j;
    board[s.p.x][s.p.y] = 0;
    for(i = 0; i < 3; i++) {
        int dx = blocks[s.type].ps[i].x;
        int dy = blocks[s.type].ps[i].y;
        int r = s.rot % blocks[s.type].rot;
        for(j = 0; j < r; j++) {
            int nx = dx, ny = dy;
            dx = ny; dy = -nx;
        }
        board[s.p.x + dx][s.p.y + dy] = 0;
    }
}

int clear_line(int board[BOARD_W][BOARD_H])
{
    int i, j;
    int n=0, m=0, s=0;
    //ゲーム盤のすべてのマスについて見ていく。
    //空のマスは0でその他は非負整数であるから、ある行のすべてのマスの値の積と和から
    //クリアすべき行であるか、また空行であるかが分かる。
    for(i=1; i<BOARD_H-4;){
        m=1; s=0;
        for(j=1; j<BOARD_W-1; j++){
            m *= board[j][i];
            s += board[j][i];
        }
        if(m>0){
            //積が0では無いため、この行はクリアすべき行である。
            n++;    //クリアされた行数をインクリメント

            //クリアすべき行よりも上の行を1つずつ下にずらすことでクリアを実現する
            int k, l;
            for(k = i+1; k<BOARD_H-4; k++){
                for(l=1; l<BOARD_W-1; l++){
                    board[l][k-1] = board[l][k];
                }
            }
            if(n==4) break; //nが4より大きくなることは無いため

            //行がクリアされ新しくなったのでiはインクリメントしない
        }else{
            if(s==0) break; //空行なのでこれ以降はすべて空行。よって処理を中断する。
            i++;    //次の行を見に行く
        }
    }
    return n;
}

void fill(int board[BOARD_W][BOARD_H], int c1, int c2, int x, int y)
{
    if(x<=0 || x>=BOARD_W-1 || y<=0 || y>=BOARD_H-1) return;
    if(board[x][y] != c1) return;
    board[x][y] = c2;
    fill(board, c1, c2, x-1, y);
    fill(board, c1, c2, x+1, y);
    fill(board, c1, c2, x-1, y-1);
    fill(board, c1, c2, x+1, y-1);
    fill(board, c1, c2, x-1, y+1);
    fill(board, c1, c2, x+1, y+1);
    fill(board, c1, c2, x, y-1);
    fill(board, c1, c2, x, y+1);
}

void make_hole_visible(int board[BOARD_W][BOARD_H])
{
    fill(board, 0, -1, 1, BOARD_H-1);
}

void make_hole_invisible(int board[BOARD_W][BOARD_H])
{
    fill(board, -1, 0, 1, BOARD_H-1);
}

int* examine(int board[BOARD_W][BOARD_H], Status s)
{
    int* ret = (int *)malloc(sizeof(int)*4);
    int i, j;
    int clearable_line_num = 0;
    int max_height = 0;
    int hole_num = 0;
    int blockade_num = 0;
    int m=1, sum=0;

    put_block(board, s);    //とりあえずblockを置いてみる

    //クリア可能なラインの数について調べる
    for(i=1; i<BOARD_H-4;i++){
        m=1;
        sum=0;
        for(j=1; j<BOARD_W-1; j++){
            m *= board[i][j];
            sum += board[i][j];
        }
        if(m>0){
            //積が0では無いため、この行はクリアすべき行である。
            clearable_line_num++;    //クリアされた行数をインクリメント
            if(clearable_line_num == 4) break;  //4ライン以上をクリアすることはない
        }else{
            if(sum==0) break; //空行なのでこれ以降はすべて空行。よって処理を中断する。
        }
    }

    //高さを求める
    Position p[4];
    p[0].x = s.p.x;
    p[0].y = s.p.y;
    for(i = 0; i < 3; i++) {
        int dx = blocks[s.type].ps[i].x;
        int dy = blocks[s.type].ps[i].y;
        int r = s.rot % blocks[s.type].rot;
        for(j = 0; j < r; j++) {
            int nx = dx, ny = dy;
            dx = ny; dy = -nx;
        }
        p[i+1].x = s.p.x + dx;
        p[i+1].y = s.p.y + dy;
        if(max_height < s.p.y + dy){
            max_height = s.p.y + dy;
        }
    }

    //ホールの数を調べる
    make_hole_visible(board);   //ホール以外の0を-1で埋める
    for(i=1; i<BOARD_H-4; i++){
        for(j=1; j<BOARD_W-1; j++){
            if(board[i][j]==0){
                hole_num++;
            }
        }
    }
    //封鎖の数を調べる
    for(i=1; i<BOARD_W-1; i++){
        int blockade_flag=0;
        for(j=1; j<BOARD_H-4; j++){
            if(board[i][j]==0){
                blockade_flag=1;
            }
            if(blockade_flag && board[i][j]>0){
                blockade_num++;
            }
        }
    }
    make_hole_invisible(board); //-1だったものをもとに戻す
    remove_block(board, s); //もう不要なのでブロックを除く
    ret[0] = clearable_line_num;
    ret[1] = max_height;
    ret[2] = hole_num;
    ret[3] = blockade_num;
    return ret;
}

double calc_score(int board[BOARD_W][BOARD_H], Status s, double e[4])
{
    double score = 0;
    int* x = examine(board, s);
    int i;
    for(i=0; i<4; i++){
        score += e[i]*x[i];
    }
    free(x);
    return score;
}

void print_board(int board[BOARD_W][BOARD_H])
{
    int x, y;
    for(y=BOARD_H-1; y>=0; y--){
        for(x=0; x<BOARD_W; x++){
            printf("%d", board[x][y]);
        }
        printf("\n");
    }
}

double tetris(double e[4])
{
    double score = 0;
    int board[BOARD_W][BOARD_H];
    int i, j;
    for(i=0; i<BOARD_W; i++){
        for(j=0; j<BOARD_H; j++){
            if(i==0 || i==BOARD_W-1 || j==0){
                board[i][j] = 1;
            }else{
                board[i][j] = 0;
            }
        }
    }
    int c =0;
    while(1){
        // printf("#%d ", c);
        int type = rand()%7+1;
        double highest_score = -1000000;
        Status best_status;
        int game_over_flag = 1;
        for(i=0; i<blocks[type].rot; i++){
            int x=1, y=BOARD_H-4;
            double e_score;
            for(x=1; x<BOARD_W-1; x++){
                Status s = {{x, y}, type, i};
                while(1){
                    s.p.y--;
                    if(!is_putable_block(board, s)){
                        s.p.y++;
                        break;
                    }
                }
                if(s.p.y == y) continue;   //この列ではゲームオーバー
                e_score = calc_score(board, s, e);
                // printf("\ttype:%d rot:%d x:%02d y:%02d e_score:%lf\n", type, i, s.p.x, s.p.y, e_score);
                if(highest_score < e_score){
                    highest_score = e_score;
                    best_status = s;
                    game_over_flag = 0;
                }
            }
        }
        // printf("\n");
        if(game_over_flag || c > 10000){
            // printf("score: %lf\n", score);
            break;
        };
        put_block(board, best_status);
        // print_board(board);
        int l = clear_line(board);
        // print_board(board);
        // printf("\n\n");
        // print_board(board);
        if(l==1){
            score += 40;
        }else if(l==2){
            score += 120;
        }else if(l==3){
            score += 300;
        }else if(l==4){
            score += 1200;
        }
        c++;
    }
    return score;
}

double tetris_test(double e[4])
{
    double score_avg = (tetris(e)+tetris(e)+tetris(e))/3;
    return score_avg;
}
