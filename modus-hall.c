#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <semaphore.h>
#include <time.h>
#include <string.h>
#include <errno.h>

#define MAX 128
#define NEUTRAL 0

int _heathensCounter = 0;
int _prudesCounter = 0;
int _status = NEUTRAL;

pthread_mutex_t _mutex = PTHREAD_MUTEX_INITIALIZER;

sem_t _heathenTurn;
sem_t _prudesTurn;


void *heathens(){

  _heathensCounter++; //sinaliza que heathen chegou (incremento independe de quem é a vez)

  sem_wait(&_heathenTurn); //continua se é vez dos heathens, bloqueia caso contrário
  
  pthread_mutex_lock(&_mutex);

  _heathensCounter--;
  printf("\nHEATHENS PASSOU\nFaltam %d Prudes\t\te\t%d Heathens\n", _prudesCounter, _heathensCounter);

  //Caso em que o nº de prudes ultrapassa o nº de heathens
  if(_prudesCounter > _heathensCounter){
    sem_post(&_prudesTurn); //passa a vez para os prudes
    printf("[%d] > [%d] ", _prudesCounter, _heathensCounter);
    printf("\n[Mais Prudes. Mudando a vez para Prudes]\n\n");
  } 
  else{
    if(_heathensCounter) sem_post(&_heathenTurn);           
    else{
     printf("\n[Acabou os Heathens.Mudando a vez para Prudes]\n\n");
      sem_post(&_prudesTurn); //passa a vez para os prudes
    }
  }

  pthread_mutex_unlock(&_mutex);
  pthread_exit(0);

}

void *prudes(){
  
  sleep(1); //Usado para testar o que ocorre quando prudes vão chegando depois dos heathens

  _prudesCounter++; //sinaliza que prude chegou (incremento independe de quem é a vez)

  sem_wait(&_prudesTurn); //continua se é vez dos prudes, bloqueia caso contrário

  pthread_mutex_lock(&_mutex);

  _prudesCounter--;
  printf("\nPRUDES PASSOU\nFaltam %d Prudes\t\te\t%d Heathens\n", _prudesCounter, _heathensCounter);
  
  //Caso em que o nº de heathens ultrapassa o nº de prudes
  if(_heathensCounter > _prudesCounter){        
      sem_post(&_heathenTurn); //passa a vez para os heathens
      printf("\n[%d] > [%d] \n", _heathensCounter,_prudesCounter);
      printf("[Mais Heathens. Mudando a vez para Heathens]\n\n");
  } 
  else{
    if(_prudesCounter) sem_post(&_prudesTurn);
    else{
      printf("\n[Acabou os Prudes. Mudando a vez para Heathens]\n\n");
      sem_post(&_heathenTurn); //passa a vez para os heathens
    } 
  }

  pthread_mutex_unlock(&_mutex);
  pthread_exit(0);   

}

int main()
{
  char err_msg[MAX];
  int numeroDeHeathens;
  int numeroDePrudes;
  
  pthread_t *_tidPrudes;
  pthread_t *_tidHeathens;
  
  printf("Qual a quantidade Heathens? (no mínimo 1)\n");
  scanf("%d", &numeroDeHeathens);
  if(numeroDeHeathens < 1) numeroDeHeathens = 20;

  printf("Qual a quantidade Prudes? (no mínimo 1)\n");
  scanf("%d", &numeroDePrudes);
  if(numeroDePrudes < 1) numeroDePrudes = 20;

  _tidHeathens = (pthread_t *) malloc(numeroDeHeathens * sizeof(pthread_t));
  _tidPrudes = (pthread_t *) malloc(numeroDePrudes * sizeof(pthread_t)); 

  //Heathens têm a preferência para passar, caso o ambiente seja neutro
  if (sem_init(&_heathenTurn, 0, 1) < 0)
  {
    strerror_r(errno, err_msg, MAX);
    printf("Erro em sem_init: %s\n", err_msg);
    exit(1);
  }

  if (sem_init(&_prudesTurn, 0, 0) < 0)
  {
    strerror_r(errno, err_msg, MAX);
    printf("Erro em sem_init: %s\n", err_msg);
    exit(1);
  }
  
  printf("nº de heathens: %d \t nº de prudes: %d\n",numeroDeHeathens,numeroDePrudes);
  printf("\n------------- Inicio --------------\n");

  //Criação das threads ("pessoas")
  
  while(1) {
    int i = numeroDeHeathens;
    int j = numeroDePrudes;
    if(i-- > 0) pthread_create(&_tidHeathens[i-1], NULL, &heathens, NULL);
    if(j-- > 0) pthread_create(&_tidPrudes[j-1], NULL, &prudes, NULL);
    
    if(i==0 && j==0) break;
  }

  //Espera que todas as threads terminem antes de finalizar o programa 
  while(1) {
    int i = numeroDeHeathens;
    int j = numeroDePrudes;
    if(i-- > 0) pthread_join(_tidHeathens[i-1], NULL);
    if(j-- > 0) pthread_join(_tidPrudes[j-1], NULL);
    
    if(i==0 && j==0) break;
  }
  //Programa é finalizado
  printf("\n-------------- Fim ---------------\n");
   
  return 0;
}
