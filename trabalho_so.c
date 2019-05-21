#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <semaphore.h>
#include <time.h>
#include <string.h>
#include <errno.h>

#define MAX 128

int _heathensCounter = 0;
int _prudesCounter = 0;

pthread_mutex_t _mutex = PTHREAD_MUTEX_INITIALIZER;

sem_t _heathenTurn;
sem_t _prudesTurn;


void *heathens(){

  _heathensCounter++;
  printf("\n-- Chegou Heathens\nPrudes: %d\t\te\t\tHeathens: %d\n", _prudesCounter, _heathensCounter);

  sem_wait(&_heathenTurn);

  pthread_mutex_lock(&_mutex);


  _heathensCounter--;
  printf("\nHEATHENS PASSOU\nFaltam %d Prudes\t\te\t\t%d Heathens\n", _prudesCounter, _heathensCounter);

  sleep(1);
  if(_prudesCounter > _heathensCounter){
    sem_post(&_prudesTurn);
    printf("[%d] > [%d]\n", _prudesCounter, _heathensCounter);
    printf("\n[Mais Prudes. Mudando a vez para Prudes]\n\n");
  }
  else {
    if(_heathensCounter){
      sem_post(&_heathenTurn);
    }
    else{
      printf("\n[Acabou os Heathens. Mudando a vez para Prudes]\n\n");
      sem_post(&_prudesTurn);
    }
  }

  pthread_mutex_unlock(&_mutex);
  pthread_exit(0);

}

void *prudes(){

  _prudesCounter++;
  printf("\n-- Chegou Prudes\nPrudes: %d\t\te\t\tHeathens: %d\n", _prudesCounter, _heathensCounter);

  sem_wait(&_prudesTurn);

  pthread_mutex_lock(&_mutex);

  _prudesCounter--;
  printf("\nPRUDES PASSOU\nFaltam %d Prudes\t\te\t\t%d Heathens\n", _prudesCounter, _heathensCounter);

  sleep(1);
  if(_heathensCounter > _prudesCounter){
    sem_post(&_heathenTurn);
    printf("[%d] > [%d] \n", _heathensCounter,_prudesCounter);
    printf("\n[Mais Heathens. Mudando a vez para Heathens]\n\n");
  }
  else{
    if(_prudesCounter){
      sem_post(&_prudesTurn);
    }
    else{
      printf("\n[Acabou os Prudes. Mudando a vez para Heathens]\n\n");
      sem_post(&_heathenTurn);
    }
  }

  pthread_mutex_unlock(&_mutex);
  pthread_exit(0);
}

int main()
{
  char err_msg[MAX];
  int numeroDeHeathens, numeroDePrudes;

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

  //Heathens têm a preferência para passar
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

  printf("\n--------------------- Inicio ----------------------\n");

  //Verificações para dar suporte a diferentes números de threads

  if(numeroDeHeathens > numeroDePrudes){

    for (int i = 0; i < numeroDeHeathens; i++){
      pthread_create(&_tidHeathens[i], NULL, &heathens, NULL);
      sleep(1);
      if(i<numeroDePrudes) pthread_create(&_tidPrudes[i], NULL, &prudes, NULL);
    }

    for (int j = 0; j < numeroDeHeathens; j++){
      pthread_join(_tidHeathens[j], NULL);
      if(j<numeroDePrudes) pthread_join(_tidPrudes[j], NULL);
    }
  }
  else{
    if(numeroDePrudes > numeroDeHeathens){

      for (int i = 0; i < numeroDePrudes; i++){
        pthread_create(&_tidPrudes[i], NULL, &prudes, NULL);
        sleep(1);
        if(i<numeroDeHeathens) pthread_create(&_tidHeathens[i], NULL, &heathens, NULL);
      }

      for (int j = 0; j < numeroDePrudes; j++){
        pthread_join(_tidPrudes[j], NULL);
        if(j<numeroDeHeathens) pthread_join(_tidHeathens[j], NULL);
      }
    }
    else{

      for (int i = 0; i < numeroDeHeathens; i++){
        pthread_create(&_tidPrudes[i], NULL, &prudes, NULL);
        sleep(1);
        pthread_create(&_tidHeathens[i], NULL, &heathens, NULL);
      }

      for (int j = 0; j < numeroDeHeathens; j++){
        pthread_join(_tidHeathens[j], NULL);
        pthread_join(_tidPrudes[j], NULL);
      }
    }
  }

  printf("\n---------------------- Fim -----------------------\n");

  return 0;
}
