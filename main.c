#include "main.h"

int main(int argc, char* argv[])
{
    srand(time(NULL));

    ga_population_t parent_group = mk_init_ga_population(POP,LENGTH,CODE_MAX);
    ga_population_t child_group = mk_init_ga_population(POP,LENGTH,CODE_MAX);
    int i ;
    if(PRINT_FITNESS==1) printf("#generation,max_fitness, avg_fitness, min_fitness, best A, best B, best C, best D, best_individual_gtype\n");

    for(i=0;i <= GENERATION;i++){
        // 集団の適合度を計算し、線形リストを作る
        calc_fitness(parent_group,MIN,MAX);
        // 最大値・最小値、
        normalize_population(parent_group);

        // 現在の世代の表示
        if(PRINT_GROUP ==1){
            print_population(parent_group);
        }
        if(PRINT_FITNESS == 1){
            printf("%3d, ",i);
            print_fitness(parent_group);
        }

        // 現在の世代parent_groupから次世代child_groupを作る。
        generate_population(child_group,parent_group,GAP,ELITE_RATE,P_MUTATE,P_CROSS);

        // 世代を入れ替える。
        parent_group = child_group;
    }
    return EXIT_SUCCESS;
}
