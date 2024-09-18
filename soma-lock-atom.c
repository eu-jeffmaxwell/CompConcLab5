#include <stdlib.h> 
#include <pthread.h>
#include <stdio.h>

long int soma = 0; //variavel compartilhada entre as threads
pthread_mutex_t mutex; //variavel de lock para exclusao mutua
pthread_cond_t cond;   //variavel de condicao para sincronizacao
int contagem_multiplos = 0; //contador de múltiplos de 10 impressos
int processando_multiplos = 0; //flag para indicar que estamos processando um múltiplo de 10

//funcao executada pelas threads
void *ExecutaTarefa (void *arg) {
  long int id = (long int) arg;
  printf("Thread : %ld esta executando...\n", id);

  for (int i = 0; i < 100000; i++) {
     pthread_mutex_lock(&mutex);

     // Pausa se atingimos um múltiplo de 10 e estamos esperando a thread extra imprimir
     while (soma % 10 == 0 && soma == 0 && processando_multiplos) {
         pthread_cond_wait(&cond, &mutex);
     }

     soma++; //incrementa a variavel compartilhada 

     // Sinaliza a thread extra se for um múltiplo de 10
     if (soma % 10 == 0) {
         processando_multiplos = 1;
         pthread_cond_signal(&cond);
     }
     pthread_mutex_unlock(&mutex);
  }
  
  printf("Thread : %ld terminou!\n", id);
  pthread_exit(NULL);
}

//funcao executada pela thread de log
void *extra (void *args) {
  printf("Extra : esta executando...\n");

  while (contagem_multiplos < 20) {
     pthread_mutex_lock(&mutex);

     // Aguarda até encontrar um múltiplo de 10
     while (soma % 10 != 0 || soma == 0 || !processando_multiplos) {
         pthread_cond_wait(&cond, &mutex);
     }

     // Imprime o múltiplo de 10
     printf("soma = %ld \n", soma);
     contagem_multiplos++;

     // Permite que as outras threads continuem
     processando_multiplos = 0;
     pthread_cond_broadcast(&cond);
     pthread_mutex_unlock(&mutex);
     
  }

  printf("Extra : terminou!\n");
  pthread_exit(NULL);
}

//fluxo principal
int main(int argc, char *argv[]) {
   pthread_t *tid; //identificadores das threads no sistema
   int nthreads; //qtde de threads (passada linha de comando)

   //--le e avalia os parametros de entrada
   if (argc < 2) {
      printf("Digite: %s <numero de threads>\n", argv[0]);
      return 1;
   }
   nthreads = atoi(argv[1]);

   //--aloca as estruturas
   tid = (pthread_t*) malloc(sizeof(pthread_t)*(nthreads+1));
   if (tid == NULL) { puts("ERRO--malloc"); return 2; }

   //--inicializa o mutex e a variavel de condicao
   pthread_mutex_init(&mutex, NULL);
   pthread_cond_init(&cond, NULL);

   //--cria as threads
   for (long int t = 0; t < nthreads; t++) {
     if (pthread_create(&tid[t], NULL, ExecutaTarefa, (void *)t)) {
       printf("--ERRO: pthread_create()\n"); exit(-1);
     }
   }

   //--cria thread de log
   if (pthread_create(&tid[nthreads], NULL, extra, NULL)) {
      printf("--ERRO: pthread_create()\n"); exit(-1);
   }

   //--espera todas as threads terminarem
   for (int t = 0; t < nthreads; t++) {
    printf(" entrando %d..\n", t);
    if (pthread_join(tid[t], NULL)) {
      printf("--ERRO: pthread_join() \n"); exit(-1); 
    } 
   }
   
   //--finaliza o mutex e a condicao
   pthread_mutex_destroy(&mutex);
   pthread_cond_destroy(&cond);
   
   printf("Valor final de 'soma' = %ld\n", soma);

   return 0;
}
