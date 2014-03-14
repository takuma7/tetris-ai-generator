#include "ga.h"

void* my_malloc(int size)
{
    void* ptr = malloc(size);
    if(!ptr){
        fprintf(stderr,"failed to maloc.\n");
        exit(1);
    }
    return ptr;
}

gtype_t mk_gtype(int code_length)
{
    gtype_t gtype = (gtype_t)my_malloc(sizeof(int)*code_length);
    return gtype;
}

void free_gtype(gtype_t gtype)
{
    free(gtype);
    return;
}

ptype_t mk_ptype()
{
    return (ptype_t)my_malloc(sizeof(struct ga_ptype));
}

void free_ptype(ptype_t ptype)
{
    free(ptype);
    return;
}

gtype_t mk_random_gtype(int code_length, int code_max)
{
    gtype_t ptr = mk_gtype(code_length*P_NUM);
    int i;
    for(i=0;i<P_NUM*code_length;i++){
        ptr[i] = rand()%(code_max+1);
    }
    return ptr;
}

void encode_gtype(ptype_t value, gtype_t gtype, int code_length, double min, double max)
{
    int i, j;
    for(i=0; i<P_NUM; i++){
        double gap = max -min;
        double remain_value = value->e[i] - min; /* 値のうち、遺伝子によって表現されている部分 */
        double value_of_code;         /* その桁の遺伝子が表現する値 */
        int position = 1;
        int pre_code = 0;
        int tmp;  /* グレイコード変換用,一時保管変数 */
        j = i*code_length;
        while ( j < (i+1)*code_length ){
            value_of_code = gap/ pow(2,position);
            if(remain_value >= value_of_code){
                gtype[j] = 1;
                remain_value -= value_of_code;
            }else{
                gtype[j] = 0;
            }
            /* グレイコードへの変換
             * バイナリコードと、元のバイナリを右に１シフトしたもののXORをとる
             */
            if(GRAY == 1){
                tmp = gtype[j];
                gtype[j] = (pre_code) ^ (gtype[j]);
                pre_code = tmp;
            }
            position++;
            j++;
        }
    }
    return;
}

ptype_t decode_gtype(gtype_t gtype, int code_length, double min, double max)
{
    double gap = max - min;
    ptype_t decoded_p = mk_ptype();
    int i, j;
    for(i=0; i<P_NUM; i++){
        double decoded_value = min;
        int position = 1;
        int pre_code = 0;                     /* １つ上位の桁のコード(バイナリ),バイナリとグレイの変換に必要 */

        /* グレイコードの解釈 */
        /* 変換されたバイナリの１つ上位の桁のコードとの排他的論理和を取る */
        j=i*code_length;
        if(GRAY==1){
            while (j<(i+1)*code_length){
                pre_code = pre_code ^ gtype[j];
                if(pre_code){
                    decoded_value += gap / pow(2,position); /* 最上位から順に、最大値と最小値の差の1/2,1/4,1/8,1/16,,,となる */
                }
                position++;
                j++;
            }
        }
        /* バイナリコードの時 */
        else{
            while (j<(i+1)*code_length){
                if(gtype[j]){
                    decoded_value += gap / pow(2,position); /* 最上位から順に、最大値と最小値の差の1/2,1/4,1/8,1/16,,,となる */
                }
                position++;
                j++;
            }
        }
        decoded_p->e[i] = decoded_value;
    }
    return decoded_p;
}

/* gtype のコピー*/
void copy_gtype(gtype_t new_gtype, gtype_t old_gtype, int length)
{
    int i = 0;
    for(i=0;i<P_NUM*length;i++){
        new_gtype[i]=old_gtype[i];
    }
    return;
}

int cross_gtype(gtype_t gtype1, gtype_t gtype2, int length)
{
    int cross_point = rand()%(P_NUM*length-1);
    int i = cross_point+1;
    int tmp;
    while(i<P_NUM*length){
        tmp = gtype1[i];
        gtype1[i]=gtype2[i];
        gtype2[i]=tmp;
        i++;
    }
    return cross_point;
}

int  mutate_gtype(gtype_t gtype, int length, int code_max, double pm)
{
    //エラー処理
    if(pm>1 || pm < 0.0){
        printf("%f is used for mutation probability, but this must be from 0.0 to 1.0 \n",pm);
        exit(-1);
    }
    int mutate_point = 0;
    int i = 0;
    double rm;
    for(i=0; i < length ; i++){
        rm = (double)rand()/RAND_MAX;
        if(rm<pm){
            gtype[i] = rand()% (code_max+1);
            mutate_point++;
        }
    }
    return mutate_point;
}

/* gtypeを表示する */
void print_gtype(gtype_t gtype,int length)
{
    int i=0;
    printf("[");
    while(i < P_NUM*length){
        if(gtype[i]<10){
            printf("%d",gtype[i]);
        }
        else{
            printf("(%d)",gtype[i]);
        }
        i++;
    }
    printf("]");
}

/* 線形リスト用の隣接した要素の入れ替え、引数は先頭のindividual_tのアドレス */
void switch_gene(individual_t *individual)
{
    individual_t tmp_ptr1 = (*individual) -> next -> next;
    individual_t tmp_ptr2 = (*individual) -> next;
    (*individual)-> next -> next = *individual;
    (*individual)->next = tmp_ptr1;
    (*individual) = tmp_ptr2;
    return;
}

/* 個体を作る。メモリ領域確保、初期化 */
individual_t mk_gene(int code_length, int code_max)
{
    individual_t ptr = my_malloc(sizeof(struct ga_individual));
    ptr->gtype = mk_random_gtype(code_length, code_max);
    ptr->ptype = mk_ptype();
    ptr->fitness = 0;
    ptr->next = NULL;
    ptr->parent1 = 0;
    ptr->parent2 = 0;
    ptr->cross_point = 0;
    return ptr;
}

/* 個体をコピーする */
void copy_gene(individual_t new_gene, individual_t old_gene, int code_length)
{
    copy_gtype(new_gene->gtype,old_gene->gtype,code_length);
    new_gene->ptype = old_gene->ptype;
    new_gene->fitness = old_gene->fitness;
    new_gene->parent1 = old_gene->rank;
    new_gene->parent2 = old_gene->rank;
    new_gene->cross_point = P_NUM * code_length-1;
    return;
}

/* 交叉、突然変異で子供を作る 突然変異回数を返す */
int mk_children_genes(individual_t child1, individual_t child2, individual_t parent1, individual_t parent2, int code_length,int code_max,double pm)
{
    int cross_point,mutateCount;
    copy_gene(child1,parent1,code_length);
    copy_gene(child2,parent2,code_length);
    child1->parent1 = parent1->rank;
    child1->parent2 = parent2->rank;
    cross_point = cross_gtype(child1->gtype,child2->gtype,code_length);
    child1->parent1 = parent1->rank;
    child1->parent2 = parent2->rank;
    child1->cross_point = cross_point;
    child2->parent1 = parent2->rank;
    child2->parent2 = parent1->rank;
    child2->cross_point = cross_point;
    mutateCount = mutate_gtype(child1->gtype,code_length,code_max,pm);
    mutateCount += mutate_gtype(child2->gtype,code_length,code_max,pm);
    return mutateCount;
}

/*GA集団の作成、初期化を行う*/
ga_population_t mk_init_ga_population(int population_size, int code_length, int code_max)
{
    ga_population_t population = my_malloc(sizeof(struct ga_population));
    population->pselect = (double*)my_malloc(sizeof(double)*population_size);
    population->mutate_count = 0;
    population->population_size = population_size;
    population->code_length = code_length;
    population->code_max = code_max;
    individual_t list_tale;
    population->genes = mk_gene(code_length, code_max);
    list_tale = population->genes;
    int i=1;
    for(i=1; i<population_size; i++){
        list_tale->next = mk_gene(code_length,code_max);
        list_tale = list_tale->next;
    }
    return population;
}

/* 指定した文字chを指定した長さlengthだけ繰り返す関数 */
/* print_population(・)の中で使われる。 */
void print_sequence(char ch, int length)
{
    int i = 0;
    for( i = 0; i < length; i++ ){
        printf("%c", ch);
    }
}

/* 集団を表示する。 */
/* 左から,世代数,親のインデックス,交叉点,gtype,ptype,fitnessを表示する。 */
/* また、最後に突然変異の回数を表示する。 */
void print_population(ga_population_t population)
{
    individual_t member = population->genes;
    int i = 0;
    printf("-------------------");
    print_sequence('-', LENGTH+2);
    printf("---------------\n");
    printf("#   parents  xsite  gtype");
    print_sequence(' ', LENGTH-3);
    printf("A   B   C   D  fitness\n");

    while(member != NULL){
        printf("%-3d (%3d,%3d) %3d  ", i, member->parent1, member->parent2, member->cross_point);
        print_gtype(member->gtype, population->code_length);
        printf(" %+3.3f  %+3.3f  %+3.3f  %+3.3f  %+3.3f\n",member->ptype->e[0],member->ptype->e[1],member->ptype->e[2],member->ptype->e[3],member->fitness);
        member = member->next;
        i++;
    }
    printf("total mutate %d\n", population->mutate_count);
    return;
}

/* 適合度を出力
   最大,平均,最小
   CSV形式にする
   */
void print_fitness(ga_population_t population)
{
    printf("%f, %f, %f, ",population->max_fitness,population->avg_fitness,population->min_fitness);
    int i;
    for(i=0; i<P_NUM; i++){
        printf("%f, ", population->genes->ptype->e[i]);
    }
    print_gtype(population->genes->gtype, population->code_length);
    printf("\n");
    return;
}

/* GA集団の個体線形リストgenesの一人一人のfitnessを見て
   配列pselectを作る
   1. pselect[i] = pselect[i-1]+fitness[i]
   2. pselect[i] = pselect[i]/pselect[POPULATION-1]
   */
void calc_pselect(ga_population_t population)
{
    int i;
    population->pselect[0] = population->genes->fitness;
    individual_t gene_ptr = population->genes->next;

    for (i=1; i < population->population_size; i++){
        population->pselect[i] = population->pselect[i-1] + gene_ptr->fitness;
        gene_ptr = gene_ptr->next;
    }
    for (i=0;i< population->population_size;i++){
        population->pselect[i] /= population->pselect[population->population_size-1];
    }
    return;
}

int less_than(individual_t individualA, individual_t individualB)
{
    return (individualA->fitness < individualB->fitness);
}

void calc_fitness(ga_population_t population, double value_min, double value_max)
{
    individual_t ptr = population->genes;
    individual_t next;
    individual_t individual_ptr = NULL;
    individual_t search_ptr = ptr;

    while(ptr != NULL){
        ptr->ptype = decode_gtype(ptr->gtype,population->code_length,value_min,value_max);
        ptr->fitness = tetris_test(ptr->ptype->e);
        next = ptr->next;
        ptr->next = NULL;

        // 線形リストに適合度順に挿入
        search_ptr = individual_ptr;
        if( search_ptr == NULL || less_than(individual_ptr, ptr)){
            ptr->next = individual_ptr;
            individual_ptr = ptr;
        } else {
            while(search_ptr->next != NULL){
                if( less_than(search_ptr->next, ptr) ){
                    break;
                }
                search_ptr = search_ptr->next;
            }
            ptr->next = search_ptr->next;
            search_ptr->next = ptr;
        }

        ptr = next;
    }
    population->genes = individual_ptr;
    return;
}

individual_t select_parent_roulette(ga_population_t population)
{
    int j=0;
    double r;
    individual_t parent;
    r = (double)rand()/RAND_MAX;
    parent = population->genes;

    /* pselect[j-1]<r<pselect[j]のとき、j番目が親になる */
    while (r > population->pselect[j]){
        parent = parent->next;
        j++;
    }
    return parent;
}

/* トーナメント方式による親選択 */
individual_t select_parent_tournament(ga_population_t population, int tournament_size)
{
    int pop = population->population_size;
    int i,j,r,min = pop;
    individual_t min_selected = NULL;
    individual_t ptr;


    for(i=0;i<tournament_size;i++){
        r = rand() % pop;
        if(min > r)
        {min = r;}
    }
    ptr = population->genes;
    for(j=0;j<min;j++){
        ptr = ptr->next;
    }
    min_selected = ptr;
    return min_selected;
}

/* 親個体の選択、param.hのSELECTION_METHODによって
   ルーレット選択かトーナメント選択を行う */
individual_t select_parent(ga_population_t population)
{
    individual_t parent;
    switch(SELECTION_METHOD){
        case 1:
            parent = select_parent_roulette(population);
            break;
        case 2:
            parent = select_parent_tournament(population,TOURNAMENT_SIZE);
            break;
        default:
            fprintf(stderr,"invalid number on SELECTION_METHOD\n");
            exit(1);
    }
    return parent;
}

/*
   適合度順に並んだ線形リストから
   最大値、最小値、平均値を記録、順番付け
   */
void normalize_population(ga_population_t population)
{
    int i;
    individual_t tmp;

    tmp = population->genes;
    population->max_fitness = population->genes->fitness; /* 先頭の適合度が最大適合度 */
    double avg = 0.0;
    /* 順番付け*/
    for(i=0;i < population->population_size;i++){
        avg += tmp->fitness;
        tmp->rank = i;
        population->min_fitness = tmp->fitness;
        tmp = tmp-> next;
    }
    avg = avg/population->population_size;
    population->avg_fitness = avg;

    return;
}

/* 新しい世代の生成
   new_populationのメモリ領域はすでに確保してあるとする
   必ずソート済みのpopulationを渡すこと
   */
void generate_population(ga_population_t new_population, ga_population_t old_population, double gap, double elite_rate, double mutate_prob, double crossover_prob)
{
    int num_of_remain = (int)(old_population->population_size*(1-gap)); /*親世代からコピーする数*/
    int num_of_elite = (int)(num_of_remain*elite_rate);  /*コピー枠のうちエリートの数*/
    int generated;
    double rand_double;
    individual_t old_gene = old_population->genes;
    individual_t new_gene = new_population->genes;

    /* 親選択テーブルを準備 */
    calc_pselect(old_population);

    /* エリート戦略 親世代での上位一定数はそのまま子供になる */
    for(generated = 0 ;generated < num_of_elite ; generated++){
        copy_gene(new_gene,old_gene,old_population->code_length);
        old_gene = old_gene->next;
        new_gene = new_gene->next;
    }
    /* エリート以外のそのまま子供になる枠 */
    for( ; generated < num_of_remain ; generated++){
        copy_gene(new_gene,select_parent(old_population),old_population->code_length);
        new_gene = new_gene->next;
    }

    new_population->mutate_count = 0;
    /* 交叉・突然変異を適応する枠 */
    /* 残り個体数が奇数の時は、一つだけ突然変異で子供を作る */
    if( (old_population->population_size - generated)%2 == 1){
        copy_gene(new_gene,select_parent(old_population),old_population->code_length);
        new_population->mutate_count += mutate_gtype(new_gene->gtype,old_population->code_length,old_population->code_max,mutate_prob);
        new_gene = new_gene->next;
        generated++;
    }

    /* 交叉・突然変異をする */
    for(; generated < old_population->population_size; generated += 2){
        rand_double = (double)rand()/RAND_MAX;
        /* 交叉するとき */
        if(rand_double < crossover_prob){
            new_population->mutate_count += mk_children_genes(new_gene,new_gene->next,select_parent(old_population),select_parent(old_population),old_population->code_length,old_population->code_max,mutate_prob);
            new_gene = new_gene->next->next;
        }
        /* 交叉しないとき */
        else{
            copy_gene(new_gene,select_parent(old_population),old_population->code_length);
            new_population->mutate_count += mutate_gtype(new_gene->gtype,old_population->code_length,old_population->code_max,mutate_prob);
            new_gene = new_gene->next;
            copy_gene(new_gene,select_parent(old_population),old_population->code_length);
            new_population->mutate_count += mutate_gtype(new_gene->gtype,old_population->code_length,old_population->code_max,mutate_prob);
            new_gene = new_gene->next;
        }
    }
    return;
}
