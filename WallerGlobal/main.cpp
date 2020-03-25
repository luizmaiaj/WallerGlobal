// main.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
//INCLUSAO DE BIBLIOTECAS
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#define gens 11                    //NUMERO DE GERACOES
#define pop 500                    //TAMANHO DA POPULACAO
#define pop_70 350                 //70% DA POPULACAO
#define pop_30 150                 //30% DA POPULACAO
#define PI 3.14159265358979323846
#define height 200                 //ALTURA DA MATRIZ - ARRUMAR SET'S
#define width 200                  //LARGURA DA MATRIZ
#define runs 2                     //EXECUCOES DE CADA INDIVIDUOS
#define limit 1000                 //LIMITA COMPRIMENTO DO INDIVIDUO
#define angle 30                   //ANGULO QUE O ROBO DEVE VIRAR
#define run_steps 5000             //NUMERO DE PASSOS DA ARVORE


/*
 ************************************************************************
 POSICAO REAL, VIRAR, INDIVIDUOS EM ARQUIVO, VERIFICA ARQUIVOS EXISTENTES
 ************************************************************************

 -------------------------------------------------------------
 BASEADO EM ALOCACAO DINAMICA DE MEMORIA BUSCA NODE LEFT-RIGHT
 -------------------------------------------------------------

 OBJETIVO:
   INDIVIDUO DEVE PERCORRER A MATRIZ SEMPRE PROXIMO A PAREDE (UMA COLUNA/LINHA)
 E COMPLETAR SUA VOLTA EM NO MAXIMO 200 PASSOS (SAO CONSIDERADOS PASSOS OS TERMINAIS).

 FITNESS = (50 * FIT) - (10 * UNFIT);
           FIT   = FITNESS DE CADA EXECUCAO,
                 PELA SUA IMPORTANCIA EH MULTIPLICADO POR 40.

           UNFIT = NUMERO DE VEZES QUE O ROBO SAIU DO SEU CAMINHO,
                 COMO PENALIZACAO EH MULTIPLICADO POR 10.

  FUNCOES  :
    PROGN3 (3), EXECUTA TRES RAMOS EM SEQUENCIA;

    PROGN2 (2), EXECUTA DOIS RAMOS EM SEQUENCIA;

    IFWALL (I), EXECUTA RAMO ESQUERDO SE NAO HOUVER PAREDE A FRENTE E DIREITO SE HOUVER.

  TERMINAIS:
    WALKFRONT (F), FAZ ROBO ANDAR PRA FRENTE;

    WALKBACK  (B), FAZ ROBO ANDAR PRA TRAS;

    RIGHT     (R), FAZ ROBO VIRAR 90o A DIREITA;

    LEFT      (L), FAZ ROBO VIRAR 90o A ESQUERDA.

  ----------------------------------------------------------
  PARAMETROS IMPORTANTES:
    NUMERO DE GERACOES: G = 51.

    TAMANHO DA POPULACAO: M = 500.

    PROBABILIDADE DE REPRODUCAO = 30%

    PROBABILIDADE DE CRUZAMENTO = 70%

    PROBABILIDADE DE MUTACAO    =  0%
  ----------------------------------------------------------

  OUTROS DADOS:
    COMPLEXIDADE DOS INDIVIDUOS E LIMITADA.
*/

struct tree {           //ESTRUTURA COM TOPO E TRES RAMIFICACOES

    char info;           //GUARDA APENAS UM CARACTERE REFERENTE A APENAS UM TERMINAL OU FUNCAO,
                         //VISA ECONOMIA DE MEMORIA E FACILIDADE DE IMPLEMENTACAO.

    struct tree* top;    //EH 'NULL' EM INICIO
    struct tree* right;  //EH 'NULL' EM TERMINAIS
    struct tree* center; //EH 'NULL' EM 'PROGN2', 'IFWALL' E TERMINAIS 
    struct tree* left;   //EH 'NULL' EM TERMINAIS
};

struct ind {            //INDIVIDUO COMPLETO COM ARVORE E FITNESS, UTILIZADO APENAS NA RAIZ
    int fitness;
    struct tree* start;
};

int n;  //CONTROLA NUMERO DE SORTEIOS DO INDIVIDUOS

int dir, robolin, robocol;  //PARAMETROS DO ROBO

double rebolin, rebocol;         //POSICAO REAL DO ROBO

int enviro[height][width],  //AMBIENTE
way[height][width],       //CAMINHO IDEAL
steps[height][width];     //CAMINHO PERCORRIDO

int fit, unfit,      //GUARDA FITNESS
pos;               //GUARDA POSICAO DO RAMO DE TROCA

struct tree* ptr;    //RETORNO DE PONTO DE CRUZAMENTO

FILE* gene;     //PONTEIRO PARA ARQUIVO

char command[50];    //COMANDOS PARA CONSOLE

//***********************************************
//RECEBE POSICAO E RETORNA DISTANCIA ATE A PAREDE
//GLOBAIS: REBOLIN, REBOCOL, ENVIRO[height][width]
double distance(void)
{
    double dist = 0;
    int xpos, ypos;
    double xreal, yreal, aux;

    aux = (PI * dir) / 180;

    xpos = xreal = rebocol;
    ypos = yreal = rebolin;

    while (!enviro[ypos][xpos])
    {
        yreal -= sin(aux);
        xreal += cos(aux);

        ypos = yreal;
        xpos = xreal;

        dist++;
    }

    dist--;
    yreal += sin(aux);
    xreal -= cos(aux);

    xpos = xreal;
    ypos = yreal;

    while (!enviro[ypos][xpos])
    {
        yreal -= .1 * sin(aux);
        xreal += .1 * cos(aux);

        ypos = yreal;
        xpos = xreal;

        dist += .1;
    }

    return(dist);
}

//**************
//LIBERA MEMORIA
//FUNCOES: FREEMEM
void freemem(struct tree* pointer)
{
    if (pointer->left)
        freemem(pointer->left);

    if (pointer->center)
        freemem(pointer->center);

    if (pointer->right)
        freemem(pointer->right);

    free(pointer);
}

//************
//MEDE FITNESS
//GLOBAIS: ROBOLIN, ROBOCOL, WAY[height][width]*
int fitness(void)
{
    robolin = rebolin;
    robocol = rebocol;

    if (way[robolin][robocol] == 1)
    {
        way[robolin][robocol] = 0;
        return(1);
    }

    return(0);
}

//*********************************************
//SE TIVER PAREDE (SOMENTE) EM FRENTE RETORNA 1
//FUNCOES: DISTANCE
int ifwall(void)
{
    if (distance() <= 1)
        return(1);

    return(0);
}

//**************************
//FAZ ROBO ANDAR PARA FRENTE
//GLOBAIS: DIR, ROBOLIN*, ROBOCOL*, REBOLIN*, REBOCOL*
void walkfront(double rlin, double rcol)
{
    int ilin, icol;

    rlin -= sin((PI * dir) / 180);
    rcol += cos((PI * dir) / 180);

    ilin = rlin;
    icol = rcol;

    if (!enviro[ilin][icol])
    {
        rebolin = rlin;
        rebocol = rcol;

        robolin = rebolin;
        robocol = rebocol;
    }
}

//************************
//FAZ ROBO ANDAR PARA TRAS
//GLOBAIS: DIR, ROBOLIN*, ROBOCOL*, REBOLIN*, REBOCOL*
void walkback(double rlin, double rcol)
{
    int ilin, icol;

    rlin -= sin((PI * (dir + 180)) / 180);
    rcol += cos((PI * (dir + 180)) / 180);

    ilin = rlin;
    icol = rcol;

    if (!enviro[ilin][icol])
    {
        rebolin = rlin;
        rebocol = rcol;

        robolin = rebolin;
        robocol = rebocol;
    }
}

//****************************
//FAZ ROBO VIRAR 5o A ESQUERDA
//GLOBAIS: DIR*
void left(void)
{
    dir += angle;

    if (dir >= 360)
        dir -= 360;
}

//***************************
//FAZ ROBO VIRAR 5o A DIREITA
//GLOBAIS: DIR*
void right(void)
{
    dir -= angle;

    if (dir >= 360)
        dir -= 360;
}

//**********************************************************************
//ALOCA MEMORIA PARA UMA ESTRUTURA E RETORNA PONTEIRO PARA ESSA ALOCACAO
struct tree* alloc(void)
{
    struct tree* pointer;

    pointer = (struct tree*) malloc(sizeof(struct tree));
    if (!pointer)
    {
        printf("\n\n\tFalha na alocacao de memoria!!!\n\n");
        exit(1);
    }

    return(pointer);
}

//******************************
//RECEBE PONTEIRO E SORTEIA INFO
//GLOBAIS: N*
//FUNCOES: ALLOC, FREEMEM
struct tree* sort(struct tree* pointer)
{
    int randnum;    //GUARDA NUMERO SORTEADO
    struct tree* aux;  //AUXILIAR DE INICIALIZACAO DO PONTEIRO

    aux = alloc();

    aux->top = NULL;
    aux->left = NULL;
    aux->center = NULL;
    aux->right = NULL;

    if (n == 1)
        randnum = 1;  //NA PRIMEIRA EXECUCAO SORTEIA `PROGN3`

    else if (n < limit)
        randnum = (rand() % 7) + 1;  //RANDNUM RECEBE NUMERO DE 1 a 7

    else randnum = (rand() % 4) + 4;  //SE EXCEDER 300 SORTEIOS PASSA A SORTEAR SOMENTE TERMINAIS

    n++;  //GUARDA COMPLEXIDADE DO INDIVIDUO

    switch (randnum)
    {
    case 1:
        pointer->info = 51;  //GUARDA `3` EM INFO REFERENTE A `PROGN3`
        break;
    case 2:
        pointer->info = 50;  //GUARDA `2` EM INFO REFERENTE A `PROGN2`
        break;
    case 3:
        pointer->info = 73;  //GUARDA `I` EM INFO REFERENTE A `IFWALL`
        break;
    case 4:
        pointer->info = 70;  //GUARDA `F` EM INFO REFERENTE A `WALKFRONT`
        break;
    case 5:
        pointer->info = 66;  //GUARDA `B` EM INFO REFERENTE A `WALKBACK`
        break;
    case 6:
        pointer->info = 76;  //GUARDA `L` EM INFO REFERENTE A `LEFT`
        break;
    case 7:
        pointer->info = 82;  //GUARDA `R` EM INFO REFERENTE A `RIGHT`
        break;
    }

    if (randnum == 4 || randnum == 5 || randnum == 6 || randnum == 7)
    {
        pointer->left = NULL;
        pointer->center = NULL;
        pointer->right = NULL;
    }

    else if (randnum == 3 || randnum == 2)
    {
        pointer->left = aux;
        pointer->center = NULL;
        pointer->right = aux;
    }

    else
    {
        pointer->left = aux;
        pointer->center = aux;
        pointer->right = aux;
    }

    freemem(aux);

    return(pointer);
}

//******************************************************
//RECEBE PONTEIRO E CRIA INDIVIDUO POR CHAMADA RECURSIVA
//FUNCOES: MAKETREE, SORT, ALLOC
struct tree* maketree(struct tree* pointer)
{
    struct tree* aux;

    pointer = sort(pointer);  //RECEBE INFORMACAO SORTEADA

    if (pointer->left)  //CHECA VALIDADE DO PONTEIRO
    {
        aux = alloc();        //ALOCA aux

        pointer->left = aux;  //left APONTA aux

        aux->top = pointer;   //top APONTA pointer

        aux = maketree(aux);  //CONTINUA ARVORE
    }

    if (pointer->center)  //CHECA VALIDADE DO PONTEIRO
    {
        aux = alloc();          //ALOCA aux

        pointer->center = aux;  //center APONTA aux

        aux->top = pointer;     //top APONTA pointer

        aux = maketree(aux);    //CONTINUA ARVORE
    }

    if (pointer->right)  //CHECA VALIDADE DO PONTEIRO
    {
        aux = alloc();         //ALOCA aux

        pointer->right = aux;  //right APONTA aux

        aux->top = pointer;    //top APONTA pointer

        aux = maketree(aux);   //CONTINUA ARVORE
    }

    return(pointer);
}

//************************************************************
//RECEBE DOIS PONTEIROS E FAZ UMA COPIA DO PRIMEIRO NO SEGUNDO
//FUNCOES: COPY, ALLOC
struct tree* copy(struct tree* pointer1, struct tree* pointer2)
{
    struct tree* aux;

    pointer2->info = pointer1->info;

    if (pointer1->left)
    {
        aux = alloc();

        pointer2->left = aux;

        aux->top = pointer2;

        copy(pointer1->left, aux);
    }

    if (pointer1->center)
    {
        aux = alloc();

        pointer2->center = aux;

        aux->top = pointer2;

        copy(pointer1->center, aux);
    }

    if (pointer1->right)
    {
        aux = alloc();

        pointer2->right = aux;

        aux->top = pointer2;

        copy(pointer1->right, aux);
    }

    return(pointer2);
}

//*****************
//IMPRIME INDIVIDUO
//FUNCOES: PRINT
void print(struct tree* pointer)
{

    printf("%c ", pointer->info);

    if (pointer->left)
        print(pointer->left);

    if (pointer->center)
        print(pointer->center);

    if (pointer->right)
        print(pointer->right);
}

//*****************
//EXECUTA INDIVIDUO
//GLOBAIS: STEPS[height][width]*, FIT*, UNFIT*, N*, ROBOLIN, ROBOCOL
//FUNCOES: EXECUTE, IFWALL, WALKFRONT, WALKBACK, LEFT, RIGHT, FITNESS
void execute(struct tree* pointer)
{
    robolin = rebolin;
    robocol = robocol;

    switch (pointer->info)
    {
    case 51:                    //PROGN3
        execute(pointer->left);   //CHAMA RAMO ESQUERDO

        execute(pointer->center); //CHAMA RAMO CENTRAL

        execute(pointer->right);  //CHAMA RAMO DIREITO
        break;

    case 50:                    //PROGN2
        execute(pointer->left);

        execute(pointer->right);
        break;

    case 73:                   //IFWALL
        if (ifwall() == 0)
            execute(pointer->left);

        else
            execute(pointer->right);
        break;

    case 70:                   //WALKFRONT
        walkfront(rebolin, rebocol);

        if (fitness())
            fit++;
        else unfit++;

        steps[robolin][robocol] = n;

        n++;
        break;

    case 66:                   //WALKBACK
        walkback(rebolin, rebocol);

        if (fitness())
            fit++;
        else unfit++;

        steps[robolin][robocol] = n;

        n++;
        break;

    case 76:                   //LEFT
        left();

        steps[robolin][robocol] = n;

        n++;
        break;

    case 82:                   //RIGHT
        right();

        steps[robolin][robocol] = n;

        n++;
        break;
    }
}

//**********************
//PREENCHE CAMINHO IDEAL
//GLOBAIS: WAY[height][width]
void setway(void)
{
    int line, column;

    for (line = 0; line < height; line++)
        for (column = 0; column < width; column++)
            way[line][column] = 0;

    for (line = 0; line < height; line++)
        for (column = 0; column < width; column++)
        {
            switch (line)
            {
            case 2:
                if (column >= 2 && column <= 147)
                    way[line][column] = 1;
                break;

            case 51:
                if (column >= 147 && column <= 197)
                    way[line][column] = 1;
                break;

            case 147:
                if (column >= 2 && column <= 51)
                    way[line][column] = 1;
                break;

            case 197:
                if (column >= 51 && column <= 197)
                    way[line][column] = 1;
                break;
            }

            switch (column)
            {
            case 2:
                if (line >= 2 && line <= 147)
                    way[line][column] = 1;
                break;

            case 51:
                if (line >= 147 && line <= 197)
                    way[line][column] = 1;
                break;

            case 147:
                if (line >= 2 && line <= 51)
                    way[line][column] = 1;
                break;

            case 197:
                if (line >= 51 && line <= 197)
                    way[line][column] = 1;
                break;
            }
        }
}

//***************
//MATRIZ AMBIENTE
void setenviro(int matriz[height][width])
{
    int line, column;

    for (line = 0; line < height; line++)
        for (column = 0; column < width; column++)
        {
            if (line == 0 || line == 199 || column == 0 || column == 199)
                matriz[line][column] = 1;

            else
                matriz[line][column] = 0;

            if (line <= 49 && column >= 149)
                matriz[line][column] = 1;

            if (line >= 149 && column <= 49)
                matriz[line][column] = 1;
        }
}

//*****************
//IMAGEM DO CAMINHO
void setbest(unsigned char matriz[height][width][3])
{
    int line, column, R = 0, G = 1, B = 2;

    for (line = 0; line < height; line++)
        for (column = 0; column < width; column++)
        {
            if (line == 0 || line == 199 || column == 0 || column == 199)
                matriz[line][column][R] = matriz[line][column][G] = matriz[line][column][B] = 255;

            else
                matriz[line][column][R] = matriz[line][column][G] = matriz[line][column][B] = 0;

            if (line <= 49 && column >= 149)
                matriz[line][column][R] = matriz[line][column][G] = matriz[line][column][B] = 255;

            if (line >= 149 && column <= 49)
                matriz[line][column][R] = matriz[line][column][G] = matriz[line][column][B] = 255;
        }
}

//******************************
//MEDE COMPLEXIDADE DO INDIVIDUO
//GLOBAIS: N*
//FUNCOES: LENGTH
int length(struct tree* pointer)
{

    if (pointer->left)
    {
        length(pointer->left);
        n++;
    }

    if (pointer->center)
    {
        length(pointer->center);
        n++;
    }

    if (pointer->right)
    {
        length(pointer->right);
        n++;
    }

    return(n);

}

//*******************************************
//RETORNA PONTEIRO PARA POSICAO DE CRUZAMENTO
//GLOBAIS: N*, POS*, *PRT*
//FUNCOES: COUNT
struct tree* count(struct tree* pointer)
{
    if (pointer->left)
    {
        n--;

        if (n > 0)
            count(pointer->left);

        else if (n == 0)
        {
            pos = 0;

            ptr = pointer->left;
        }
    }

    if (pointer->center)
    {
        n--;

        if (n > 0)
            count(pointer->center);

        else if (n == 0)
        {
            pos = 1;

            ptr = pointer->center;
        }
    }

    if (pointer->right)
    {
        n--;

        if (n > 0)
            count(pointer->right);

        else if (n == 0)
        {
            pos = 2;

            ptr = pointer->right;
        }
    }
}

//******************************
//GUARDA INDIVIDUO EM UMA STRING
//GLOBAIS: *GENE
//FUNCOES: SALVAR
void salvar(struct tree* pointer)
{
    fprintf(gene, "%c", pointer->info);

    if (pointer->left)
        salvar(pointer->left);

    if (pointer->center)
        salvar(pointer->center);

    if (pointer->right)
        salvar(pointer->right);

}

//**********************************************************
//JUNTO COM A FUNCAO LOAD CARREGA UM INDIVIDUO DE UM ARQUIVO
//GLOBAIS: GENE
//FUNCOES: ALLOC, FREEMEM
struct tree* read(struct tree* pointer)
{
    struct tree* aux;
    char read_aux;

    aux = alloc();

    aux->top = NULL;
    aux->left = NULL;
    aux->center = NULL;
    aux->right = NULL;

    fscanf(gene, "%c", &read_aux);

    pointer->info = read_aux;

    if (pointer->info == 70 || pointer->info == 66 || pointer->info == 76 || pointer->info == 82)
    {
        pointer->left = NULL;
        pointer->center = NULL;
        pointer->right = NULL;
    }

    else if (pointer->info == 50 || pointer->info == 73)
    {
        pointer->left = aux;
        pointer->center = NULL;
        pointer->right = aux;
    }

    else
    {
        pointer->left = aux;
        pointer->center = aux;
        pointer->right = aux;
    }

    freemem(aux);

    return(pointer);
}

//**********************************************************
//JUNTO COM A FUNCAO READ CARREGA UM INDIVIDUO DE UM ARQUIVO
//FUNCOES: LOAD, READ, ALLOC
struct tree* load(struct tree* pointer)
{
    struct tree* aux;


    pointer = read(pointer);

    if (pointer->left)
    {
        aux = alloc();

        pointer->left = aux;

        aux->top = pointer;

        aux = load(aux);
    }

    if (pointer->center)
    {
        aux = alloc();

        pointer->center = aux;

        aux->top = pointer;

        aux = load(aux);
    }

    if (pointer->right)
    {
        aux = alloc();

        pointer->right = aux;

        aux->top = pointer;

        aux = load(aux);
    }

    return(pointer);
}

//******************
//PROGRAMA PRINCIPAL
void main(void)
{
    struct ind rob[pop],          //PONTEIRO INICIAL DO INDIVIDUO
        robtemp[pop_70];

    int stime;                   //REPASSA A HORA PARA SEMENTE DO RANDOM
    long ltime;                  //RECEBE A HORA PARA RANDOM

    int line, column;            //AUXILIARES DE IMPRESSAO DA MATRIZ

    int randnum;                 //AUXILIAR DE SORTEIO

    int i, j, k, gap, list[pop_70],  //LOOPING E AUXILIAR DE SORTEIO
        crosspoint[2],                 //PONTOS DE CRUZAMENTO
        postree[2];                    //POSICAO DO RAMO

    struct ind x, pai[2];            //AUXILIARES DE ORGANIZACAO E CRUZAMENTO

    char a[5];

    struct tree* pointer[4];

    int best = 0;

    unsigned char best_way[height][width][3];

    struct bests
    {
        unsigned char matriz[height][width][3];  //MELHOR CAMINHO
    }caminho[gens];

    int gen, nsort, num;

    float averg_gen, averg_all = 0;

    int max_gen, max_all = 0;

    //SEMENTE DO RANDOM
    ltime = time(NULL);
    stime = (unsigned)ltime / 2;
    srand(stime);

    //LIMPA TELA
    system("clear");
    printf("\n******************************************************************************\n");


    //ALOCA INICIO PARA INDIVIDUOS EM ARQUIVO
    printf("\n\t\tCarregando\n");
    for (i = 0; i < 100; i++)
    {
        char filename[20];

        rob[i].start = alloc();

        rob[i].start->top = NULL;

        rob[i].fitness = 0;

        sprintf(filename, "rb%.3dtr.txt", i);

        if ((gene = fopen(filename, "r")) == NULL)
        {
            printf("\n\tNao existem individuos sucessores para \"%s\"!!!", filename);
            printf("\n\tNovos individuos serao criados.\n\n");
            break;
        }

        rob[i].start = load(rob[i].start);

        fclose(gene);
    }

    //ALOCA INICIO PARA TODOS OS INDIVIDUOS E SORTEIA OS INDIVIDUOS
    printf("\n\t\tAlocando e Sorteando\n");
    for (; i < pop; i++)
    {
        rob[i].start = alloc();    //ALOCA

        rob[i].start->top = NULL;

        rob[i].fitness = 0;

        n = 1;                            //SORTEIA
        rob[i].start = maketree(rob[i].start);

        if (i < pop_70)
        {
            robtemp[i].start = alloc();

            robtemp[i].fitness = 0;
        }
    }

    //VERIFICA ARQUIVOS DE DADOS EXISTENTES
    {
        char filename[20];


        i = 0;

        sprintf(filename, "data%.3d.txt", i);

        while (gene = fopen(filename, "r"))
        {
            fclose(gene);

            i++;
            sprintf(filename, "data%.3d.txt", i);
        }

        if ((gene = fopen(filename, "w+")) == NULL)  //ARQUIVO COM DADOS DA SIMULACAO
        {
            printf("\n\n\tFalha ao tentar criar arquivo!!!\n\n");
            exit(1);
        }

        else
        {
            fprintf(gene, "GERACAO\tMEDIA\t\tMAIOR\n");
        }
    }

    for (gen = 0; gen < gens; gen++)
    {
        averg_gen = max_gen = 0;

        //EXECUCAO DO INDIVIDUO
        printf("\n\t\tExecutando ->      ");

        for (i = 0; i < pop; i++)
        {
            double aux, aux1;

            aux1 = i;

            aux = ((aux1 + 1) / pop) * 100;

            printf("\b\b\b\b");

            if (aux >= 10)
                printf("\b");

            printf("%3.1f%%", aux); fflush(stdout);

            rob[i].fitness = 0;

            for (j = 0; j < runs; j++)
            {
                setenviro(enviro);  //AMBIENTE

                for (line = 0; line < height; line++)   //AMBIENTE EM PASSOS
                    for (column = 0; column < width; column++)
                        steps[line][column] = 0;

                setway();             //CAMINHO IDEAL

                fit = unfit = 0;

                dir = angle * (rand() % (360 / angle));          //SORTEIO DA DIRECAO

                robocol = (rand() % 198) + 1;     //SORTEIO DA COLUNA

                if (robocol <= 49)                 //SORTEIO DA LINHA
                    robolin = (rand() % 148) + 1;

                else if (robocol >= 149)
                    robolin = (rand() % 148) + 50;

                else robolin = (rand() % 198) + 1;

                rebolin = robolin;
                rebocol = robocol;

                for (n = 0; n < run_steps;)
                {
                    execute(rob[i].start);
                    if (fit == 770) break;
                }

                rob[i].fitness += fit;

                //if(rob[i].fitness < 0) rob[i].fitness = 0;

                if (fit >= best)
                {
                    setbest(best_way);    //MATRIZ QUE VAI VIRAR IMAGEM

                    best = fit;

                    for (line = 0; line < height; line++)
                        for (column = 0; column < width; column++)
                            if (steps[line][column] > 0)
                            {
                                best_way[line][column][0] = 255;
                                best_way[line][column][1] = 0;
                                best_way[line][column][2] = 0;
                            }
                }
            }

            rob[i].fitness /= j;
        }

        //*****************************
        //ORGANIZA EM ORDEM DECRESCENTE
        printf("\n\n\t\tOrganizando\n");

        a[0] = 7;
        a[1] = 5;
        a[2] = 3;
        a[3] = 2;
        a[4] = 1;

        for (k = 0; k < 5; k++)
        {
            gap = a[k];
            for (i = gap; i < pop; ++i)
            {
                x = rob[i];
                for (j = i - gap; x.fitness > rob[j].fitness && j >= 0; j = j - gap)
                    rob[j + gap] = rob[j];
                rob[j + gap] = x;
            }
        }

        //**********            
        //CRUZAMENTO
        if (gen < (gens - 1))
        {
            printf("\n\t\tCruzando\n");

            for (i = 0; i < pop_70; i++)
                list[i] = i;

            nsort = pop_70;

            for (num = 0; num < pop_70; num += 2)
            {
                for (i = 0; i < 2; i++)  //SORTEIO DOS PAIS
                {
                    if (nsort > 0)
                        randnum = rand() % nsort;

                    else randnum = 0;

                    pai[i].start = alloc();

                    pai[i].start = copy(rob[list[randnum]].start, pai[i].start);

                    nsort--;

                    list[randnum] = list[nsort];

                    n = 1;

                    crosspoint[i] = (rand() % (length(pai[i].start) - 1)) + 2;
                }

                for (i = 0; i < 2; i++)
                {
                    n = crosspoint[i] - 1;

                    count(pai[i].start);

                    pointer[i] = ptr;

                    postree[i] = pos;
                }

                for (i = 0; i < 2; i++)
                    pointer[i + 2] = pointer[i]->top;

                switch (postree[0])
                {
                case 0:
                    pointer[2]->left = pointer[1];

                    pointer[1]->top = pointer[2];
                    break;

                case 1:
                    pointer[2]->center = pointer[1];

                    pointer[1]->top = pointer[2];
                    break;

                case 2:
                    pointer[2]->right = pointer[1];

                    pointer[1]->top = pointer[2];
                    break;
                }

                switch (postree[1])
                {
                case 0:
                    pointer[3]->left = pointer[0];

                    pointer[0]->top = pointer[3];
                    break;

                case 1:
                    pointer[3]->center = pointer[0];

                    pointer[0]->top = pointer[3];
                    break;

                case 2:
                    pointer[3]->right = pointer[0];

                    pointer[0]->top = pointer[3];
                    break;
                }

                robtemp[num].start = pai[0].start;
                robtemp[num + 1].start = pai[1].start;
            }

            /*
              -----------------------
              MANUSEIO DE MEMORIA
              -----------------------
                */
            printf("\n\t\tManuseio de memoria\n");

            for (num = 0; num < pop_70; num++)
            {
                //freemem(rob[pop_30 + num].start);     //LIBERA 'ROB'

                //rob[pop_30 + num].start = alloc();    //REALOCA 'ROB'
                //rob[pop_30 + num].start->top = NULL;

                rob[pop_30 + num].start = robtemp[num].start;

                //freemem(robtemp[num].start);          //LIBERA INICIO DE 'TEMP'

                //robtemp[num].start = alloc();         //REALOCA 'TEMP'
                //robtemp[num].fitness = 0;
            }

        }

        printf("\n\t\tGeracao: %d", gen + 1);

        for (num = 0; num < pop; num++)
        {
            averg_gen += rob[num].fitness;

            if (rob[num].fitness > max_gen)
                max_gen = rob[num].fitness;

            if (max_gen > max_all)
                max_all = max_gen;
        }

        averg_gen /= pop;

        averg_all += averg_gen;

        printf("\n\n\tMEDIA GERACAO = %3.4f, MAIOR GERACAO = %3d\n", averg_gen, max_gen);

        //IMPRIME INDIVIDUO, FITNESS E COMPLEXIDADE
        for (i = 0; i < 10; i++)
        {
            n = 1;

            printf("\n");

            printf("\n\t%.2d - FITNESS = %3d, e COMPRIMENTO = %3d\n", i, rob[i].fitness, length(rob[i].start));
        }

        printf("\n");

        fprintf(gene, "%d\t%3.4f  \t%3d\n", gen + 1, averg_gen, max_gen);

        fflush(gene);  //ATUALIZA ARQUIVO DE DADOS


        printf("\n******************************************************************************\n");

        for (line = 0; line < height; line++)
            for (column = 0; column < width; column++)
                for (i = 0; i < 3; i++)
                    caminho[gen].matriz[line][column][i] = best_way[line][column][i];
    }

    averg_all /= (gens - 1);

    printf("\n\tMEDIA GERAL = %3.4f, MAIOR GERAL = %3d\n", averg_all, max_all);

    fclose(gene);

    //SALVA INDIVIDUOS
    printf("\n\t\tSalvando individuos...\n");

    for (i = 0; i < 100; i++)
    {
        char filename[20];

        sprintf(filename, "rb%.3dtr.txt", i);

        if ((gene = fopen(filename, "w+")) != NULL)
        {
            salvar(rob[i].start);

            n = 1;
            fprintf(gene, "\n\nCOMPRIMENTO = %d\n\nFITNESS = %d\n", length(rob[i].start), rob[i].fitness);
            fclose(gene);
        }
    }

    //LIBERA MEMORIA
    printf("\n\t\tLiberando memoria...\n");
    for (i = 0; i < pop; i++)
        freemem(rob[i].start);

    //VERIFICA ARQUIVOS QUE JA EXISTEM
    {
        char filename[20];


        i = 0;

        sprintf(filename, "caminho%.3d.gif", i);

        while (gene = fopen(filename, "r"))
        {
            fclose(gene);

            i++;
            sprintf(filename, "caminho%.3d.gif", i);
        }
    }

    //SALVA CAMINHOS  
    printf("\n\t\tSalvando caminhos ->      ");
    for (gen = 0; gen < gens; i++, gen++)
    {
        char filename[20];
        double aux, aux1, aux2;

        aux = ((gen + 1) / gens) * 100;

        sprintf(filename, "caminho%.3d.ppm", i);

        //SALVA UMA MATRIZ PARA UM ARQUIVO TIPO PPM

        if ((gene = fopen(filename, "w+")) == NULL)
        {
            printf("\n\n\tArquivo nao pode ser criado.\n\n");
            exit(-1);
        }

        fprintf(gene, "P3 \n 200 200 \n 255 \n");  //GRAVA COMECO

        for (line = 0; line < height; line++)     //GRAVA NO ARQUIVO
        {
            for (column = 0; column < width; column++)
                fprintf(gene, "%d %d %d   ", caminho[gen].matriz[line][column][0], caminho[gen].matriz[line][column][1], caminho[gen].matriz[line][column][2]);

            fprintf(gene, "\n");
        }

        fclose(gene);  //FECHA ARQUIVO

        sprintf(command, "convert %s caminho%.3d.gif", filename, i);

        system(command);

        sprintf(command, "rm %s -f", filename);

        system(command);

        aux1 = gen;

        aux = ((aux1 + 1) / gens) * 100;

        printf("\b\b\b\b");

        if (aux >= 10)
            printf("\b");

        printf("%3.1f%%", aux); fflush(stdout);
    }

    printf("\n\n\t\"FIM\"\n\n\a");

    //SINAL DE SAIDA
    exit(0);
}
