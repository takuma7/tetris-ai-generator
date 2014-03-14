#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <time.h>

#define GRAY 1              /* グレイコードかバイナリコードの指定。グレイコードなら1, バイナリコードなら0 */
#define MAX (10)            /* 扱う実数の最大値 */
#define MIN (-10)           /* 扱う実数の最小値 */
#define LENGTH (10)         /* 遺伝子のコード長 */
#define POP 100             /* 個体数 */
#define CODE_MAX 1          /* 各遺伝子コードの最大値。これが１ならコードは0か1になる。ビットストリングの場合は１で固定 */
#define GAP 0.9             /* 一回の生殖で子供と入れ替わる割合 */
#define ELITE_RATE 1.0      /* そのまま残る数のうち、エリートの割合 */
#define P_MUTATE 0.0        /* 突然変異率。LENGTHの逆数程度がよい */
#define P_CROSS 1.0         /* 交叉確率 */
#define GENERATION 10       /* GAを計算する世代数 */
#define SELECTION_METHOD 1  /* 1はルーレット 2はトーナメント*/
#define TOURNAMENT_SIZE 5   /* トーナメントサイズ。トーナメントの時だけ意味がある  */

#define P_NUM 4

/* 出力 */
#define PRINT_GROUP 1
#define PRINT_FITNESS 1

#include "tetris.h"

typedef int* gtype_t;

typedef struct ga_ptype* ptype_t;
struct ga_ptype{
    double e[P_NUM];    //係数A, B, C, D, E, F, G
};

typedef struct ga_individual* individual_t;
struct ga_individual{
    gtype_t gtype;  /*遺伝子型 int配列の先頭のポインタ*/
    ptype_t ptype;  /* 表現型 */
    double fitness; /* 適応度*/
    individual_t next; /* 連結リストでの次の個体 */
    int rank;  /* 連結リストのなかでの順位(ソート後) */
    int parent1; /* 交叉での親1のインデックス */
    int parent2; /* 交叉での親2のインデックス */
    int cross_point; /* 交叉したポイント */
};

/* 集団を表す構造体を宣言し、*/
/*   そのポインタ型をga_population_tとする。 */
typedef struct ga_population* ga_population_t;
struct ga_population {
    individual_t genes;  /* 個体の連結リスト先頭のポインタ */
    double * pselect; /* 適合度の配列 */
    int mutate_count;  /* 突然変異回数の合計 */
    double max_fitness;  /* 適合度の最大値 */
    double min_fitness;  /* 適合度の最小値 */
    double avg_fitness;  /* 適合度の平均値 */
    int population_size; /* 集団の個体数 */
    int code_length; /* 遺伝子長 */
    int code_max; /* 各遺伝子座の最大値,ビットストリングの場合は1 */
};

void* my_malloc(int size);

gtype_t mk_gtype(int code_length);

void free_gtype(gtype_t gtype);
ptype_t mk_ptype();

void free_ptype(ptype_t ptype);

gtype_t mk_random_gtype(int code_length, int code_max);

void encode_gtype(ptype_t value, gtype_t gtype, int code_length, double min, double max);

ptype_t decode_gtype(gtype_t gtype, int code_length, double min, double max);

/* gtype のコピー*/
void copy_gtype(gtype_t new_gtype, gtype_t old_gtype, int length);

int cross_gtype(gtype_t gtype1, gtype_t gtype2, int length);

int  mutate_gtype(gtype_t gtype, int length, int code_max, double pm);

/* gtypeを表示する */
void print_gtype(gtype_t gtype,int length);

/* 線形リスト用の隣接した要素の入れ替え、引数は先頭のindividual_tのアドレス */
void switch_gene(individual_t *individual);

/* 個体を作る。メモリ領域確保、初期化 */
individual_t mk_gene(int code_length, int code_max);

/* 個体をコピーする */
void copy_gene(individual_t new_gene, individual_t old_gene, int code_length);

/* 交叉、突然変異で子供を作る 突然変異回数を返す */
int mk_children_genes(individual_t child1, individual_t child2, individual_t parent1, individual_t parent2, int code_length,int code_max,double pm);

/*GA集団の作成、初期化を行う*/
ga_population_t mk_init_ga_population(int population_size, int code_length, int code_max);

/* 指定した文字chを指定した長さlengthだけ繰り返す関数 */
/* print_population(・)の中で使われる。 */
void print_sequence(char ch, int length);

/* 集団を表示する。 */
/* 左から,世代数,親のインデックス,交叉点,gtype,ptype,fitnessを表示する。 */
/* また、最後に突然変異の回数を表示する。 */
void print_population(ga_population_t population);

/* 適合度を出力
   最大,平均,最小
   CSV形式にする
   */
void print_fitness(ga_population_t population);

/* GA集団の個体線形リストgenesの一人一人のfitnessを見て
   配列pselectを作る
   1. pselect[i] = pselect[i-1]+fitness[i]
   2. pselect[i] = pselect[i]/pselect[POPULATION-1]
   */
void calc_pselect(ga_population_t population);

int less_than(individual_t individualA, individual_t individualB);

void calc_fitness(ga_population_t population, double value_min, double value_max);
individual_t select_parent_roulette(ga_population_t population);

/* トーナメント方式による親選択 */
individual_t select_parent_tournament(ga_population_t population, int tournament_size);

/* 親個体の選択、param.hのSELECTION_METHODによって
   ルーレット選択かトーナメント選択を行う */
individual_t select_parent(ga_population_t population);

/*
   適合度順に並んだ線形リストから
   最大値、最小値、平均値を記録、順番付け
   */
void normalize_population(ga_population_t population);

/* 新しい世代の生成
   new_populationのメモリ領域はすでに確保してあるとする
   必ずソート済みのpopulationを渡すこと
   */
void generate_population(ga_population_t new_population, ga_population_t old_population, double gap, double elite_rate, double mutate_prob, double crossover_prob);
