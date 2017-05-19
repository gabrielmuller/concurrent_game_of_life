/*
* The Game of Life
*
* a cell is born, if it has exactly three neighbours
* a cell dies of loneliness, if it has less than two neighbours
* a cell dies of overcrowding, if it has more than three neighbours
* a cell survives to the next generation, if it does not die of loneliness
* or overcrowding
*
* In this version, a 2D array of ints is used.  A 1 cell is on, a 0 cell is off.
* The game plays a number of  (given by the input), printing to the screen each time.  'x' printed
* means on, space means off.
*
*/
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

//variáveis globais
typedef unsigned char cell_t;
unsigned int lines_per_thread, lines_last_thread, num_threads, steps, size;

cell_t ** prev, ** next;
pthread_barrier_t gen;

cell_t ** allocate_board (int size) {
  cell_t ** board = (cell_t **) malloc(sizeof(cell_t*)*size);
  int	i;
  for (i=0; i<size; i++)
  board[i] = (cell_t *) malloc(sizeof(cell_t)*size);
  return board;
}

void free_board (cell_t ** board, int size) {
  int     i;
  for (i=0; i<size; i++)
  free(board[i]);
  free(board);
}


/* return the number of on cells adjacent to the i,j cell */
int adjacent_to (cell_t ** board, int size, int i, int j) {
  int	k, l, count=0;

  int sk = (i>0) ? i-1 : i;
  int ek = (i+1 < size) ? i+1 : i;
  int sl = (j>0) ? j-1 : j;
  int el = (j+1 < size) ? j+1 : j;

  for (k=sk; k<=ek; k++)
  for (l=sl; l<=el; l++)
  count+=board[k][l];
  count-=board[i][j];

  return count;
}

/* print the life board */
void print (cell_t ** board, int size) {
  int	i, j;
  /* for each row */
  for (j=0; j<size; j++) {
    /* print each column position... */
    for (i=0; i<size; i++)
    printf ("%c", board[i][j] ? 'x' : ' ');
    /* followed by a carriage return */
    printf ("\n");
  }
}

/* line_start: em que linha começa a calcular (inclusivo)
 * line_end: em que linha termina de calcular (exclusivo)
 */
void play (int size, int line_start, int line_end) {
  int	i, j, a;
  /* for each cell, apply the rules of Life */
  for (i=line_start; i<line_end; i++)
  for (j=0; j<size; j++) {
    a = adjacent_to (prev, size, i, j);
    if (a == 2) next[i][j] = prev[i][j];
    if (a == 3) next[i][j] = 1;
    if (a < 2) next[i][j] = 0;
    if (a > 3) next[i][j] = 0;
  }
}

void* start (void* args) {
  unsigned int index = *((unsigned int*)args); //número da thread
  unsigned int my_lines = lines_per_thread; //quantas linhas essa thread calcula
  cell_t ** tmp = allocate_board (size);
  if (index == num_threads - 1) {
    //última thread, fica com o resto
    my_lines = lines_last_thread;
  }
  //define o intervalo de linhas [start, end) que a thread calcula
  int start, end;
  start = index * lines_per_thread;
  end = start + my_lines;
  
  //a cada geração
  for (int i=0; i<steps; i++) {
    play (size, start, end);

    //espera todas threads terminarem de calcular
    pthread_barrier_wait(&gen);
    
    //a primeira thread prepara o tabuleiro para próxima geração
    if (index == 0) {
      #ifdef DEBUG
      printf("%d ----------\n", i + 1);
      print (next,size);
      #endif
      tmp = next;
      next = prev;
      prev = tmp;
    }
    
    //espera primeira thread terminar o procedimento
    pthread_barrier_wait(&gen);
  }
}

/* read a file into the life board */
void read_file (FILE * f, cell_t ** board, int size) {
  int	i, j;
  char	*s = (char *) malloc(size+10);

  /* read the first new line (it will be ignored) */
  fgets (s, size+10,f);

  /* read the life board */
  for (j=0; j<size; j++) {
    /* get a string */
    fgets (s, size+10,f);
    /* copy the string to the life board */
    for (i=0; i<size; i++)
    board[i][j] = s[i] == 'x';

  }
}
int round_division (int a, int b) {
  int result = a / b;
  int remainder = a % b;
  if (remainder < (b/2)) {
    return result;
  } else {
    return (result + 1);
  }
}
int main (int argc, char ** argv) {

  
  FILE    *f;
  f = stdin;
  fscanf(f,"%d %d", &size, &steps);
  prev = allocate_board (size);
  read_file (f, prev,size);
  fclose(f);
  next = allocate_board (size);

  #ifdef DEBUG
  printf("Initial:\n");
  print(prev,size);
  #endif
  
  //define num_threads a partir da linha de comando
  switch (argc) {
    case 1:
      num_threads = 1;
      printf("Usando número padrão de threads (%d)\n", num_threads);
      break;
    case 2:
      num_threads = atoi(argv[1]);
      break;
    default:
      printf("Erro: mais de dois argumentos");
      return 1;
  }
  
  //limita número de threads.
  if (num_threads > size) {
    num_threads = size;
    printf("Aviso: foram pedidas mais threads que linhas. Usando %d threads.\n", num_threads);
  }
  
  //inicia uma barreira que sincronizará as gerações
  pthread_barrier_init(&gen, NULL, num_threads);
  
  //a maior parte das threads fica com o num de linhas igual a divisão
  //do tamanho pelo num de threads arredondado para o inteiro mais perto.
  lines_per_thread = round_division(size, num_threads);
  
  //a última thread ficará com o resto da divisão de linhas
  lines_last_thread = size - (lines_per_thread * (num_threads-1));
  
  //cria e espera pelas threads
  pthread_t threads[num_threads];
  for (int i = 0; i < num_threads; i++) {
    int* j = (int*) malloc(sizeof(int));
    *j = i;
    pthread_create(&threads[i], NULL, start, j);
  }
  for (int i = 0; i < num_threads; i++) {
    pthread_join(threads[i], NULL);
  }


#ifdef RESULT
  printf("Final:\n");
  print (prev,size);
#endif

  free_board(prev,size);
  free_board(next,size);
}
