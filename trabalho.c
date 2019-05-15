#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <semaphore.h>
#include <time.h>
#include <string.h> //Uso da função strerror_r
#include <errno.h> //Uso da função strerror

#define MAX 128

#define NEUTRAL 0
#define HEATHENS_RULE 1
#define PRUDES_RULE 2
#define TRANSITION_TO_HEATHENS 3
#define TRANSITION_TO_PRUDES 4

pthread_t _tidPrudes[10];
pthread_t _tidHeathens[10];
int _heathensCounter = 0;
int _prudesCounter = 0;
int _status = NEUTRAL;
pthread_mutex_t _mutex = PTHREAD_MUTEX_INITIALIZER;
sem_t _heathenTurn;
sem_t _prudesTurn;
sem_t _heathenQueue;
sem_t _prudesQueue;

void *heathens()
{ 
  sem_wait(&_heathenTurn);
  sem_post(&_heathenTurn);

  printf("\nH STATUS: %d\n", _status);
  printf("Quantidade de heathens = %d\tQuantidade de prudes = %d\n", _heathensCounter, _prudesCounter);

  pthread_mutex_lock(&_mutex);
  printf("Passando pelo mutex do heathens\n");
  _heathensCounter++;
  switch(_status){
      case NEUTRAL:
          _status = HEATHENS_RULE; 
          pthread_mutex_unlock(&_mutex);
        break;

      case PRUDES_RULE:
          if (_heathensCounter > _prudesCounter) {
              _status = TRANSITION_TO_HEATHENS;
              sem_wait(&_prudesTurn);
          }
          pthread_mutex_unlock(&_mutex);
          sem_wait(&_heathenQueue);
        break;

      case TRANSITION_TO_HEATHENS:
          printf("Em Heathens, TRANSITION_TO_HEATHENS -> HEATHENS_RULE\n");
          pthread_mutex_unlock(&_mutex);
          // _status = HEATHENS_RULE;
          sem_wait(&_heathenQueue);
        break;
      default:
        // pthread_mutex_unlock(&_mutex);
        printf("a");  
  }
  sleep(1);
  pthread_mutex_lock(&_mutex);
  printf("\nHeathens passando\n");
  printf("Quantidade de heathens = %d\tQuantidade de prudes = %d\n", _heathensCounter, _prudesCounter);
  _heathensCounter--;

  if (_heathensCounter == 0){
    if (_status == TRANSITION_TO_PRUDES){
      sem_post(&_prudesTurn);
    }
    if(_prudesCounter){
      sem_post(&_prudesQueue);
      _status = PRUDES_RULE;
    }
    else{
      _status = NEUTRAL;
    }
  }

  if (_status == HEATHENS_RULE){
    if(_prudesCounter > _heathensCounter){
      _status = TRANSITION_TO_PRUDES;
      sem_wait(&_heathenTurn);
    }
  }
  pthread_mutex_unlock(&_mutex);
  pthread_exit(0);
}


void *prudes()
{

  sem_wait(&_prudesTurn);
  sem_post(&_prudesTurn);

  printf("\nP STATUS: %d\n", _status);
  printf("Quantidade de heathens = %d\tQuantidade de prudes = %d\n", _heathensCounter, _prudesCounter);

  pthread_mutex_lock(&_mutex);
  _prudesCounter++;
  switch(_status){
      case NEUTRAL:
          _status = PRUDES_RULE; 
          pthread_mutex_unlock(&_mutex);
        break;

      case HEATHENS_RULE:
          if (_prudesCounter > _heathensCounter ) {
              _status = TRANSITION_TO_PRUDES;
              sem_wait(&_heathenTurn);
          }
          pthread_mutex_unlock(&_mutex);
          sem_wait(&_prudesQueue);
        break;

      case TRANSITION_TO_PRUDES:
          pthread_mutex_unlock(&_mutex);
          _status = PRUDES_RULE;
          sem_wait(&_prudesQueue);
        break;
      default:
        pthread_mutex_unlock(&_mutex);  
  }
  pthread_mutex_lock(&_mutex);
  sleep(1);
  printf("\nPrudes passando\n");
  printf("Quantidade de heathens = %d\tQuantidade de prudes = %d\n", _heathensCounter, _prudesCounter);
  _prudesCounter--;

  if (_prudesCounter == 0){
    if (_status == TRANSITION_TO_HEATHENS){
      sem_post(&_heathenTurn);
    }
    if(_heathensCounter){
      sem_post(&_heathenQueue);
      _status = HEATHENS_RULE;
    }
    else{
      _status = NEUTRAL;
    }
  }

  if (_status == PRUDES_RULE){
    if(_heathensCounter > _prudesCounter){
      _status = TRANSITION_TO_HEATHENS;
      sem_wait(&_prudesTurn);
    }
  }
  pthread_mutex_unlock(&_mutex);
  pthread_exit(0);
}

int main()
{
  char err_msg[MAX];

  if (sem_init(&_heathenTurn, 0, 1) < 0)
  {
    strerror_r(errno, err_msg, MAX);
    printf("Erro em sem_init: %s\n", err_msg);
    exit(1);
  }

  if (sem_init(&_prudesTurn, 0, 1) < 0)
  {
    strerror_r(errno, err_msg, MAX);
    printf("Erro em sem_init: %s\n", err_msg);
    exit(1);
  }

  if (sem_init(&_heathenQueue, 0, 0) < 0)
  {
    strerror_r(errno, err_msg, MAX);
    printf("Erro em sem_init: %s\n", err_msg);
    exit(1);
  }

  if (sem_init(&_prudesQueue, 0, 0) < 0)
  {
    strerror_r(errno, err_msg, MAX);
    printf("Erro em sem_init: %s\n", err_msg);
    exit(1);
  }

  for (int i = 0; i < 10; i++){
    pthread_create(&_tidHeathens[i], NULL, &heathens, NULL);
    if(i%2 == 0){
      pthread_create(&_tidPrudes[i], NULL, &prudes, NULL);
    }
  }

  for (int j = 0; j < 10; j++){
    pthread_join(_tidHeathens[j], NULL);
    if(j%2 == 0){
      pthread_join(_tidPrudes[j], NULL);
    }
  }

  printf("\nTerminou\n");
   
  return 0;
}