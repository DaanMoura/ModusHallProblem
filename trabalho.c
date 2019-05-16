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

pthread_t _tidPrudes[20];
pthread_t _tidHeathens[20];
int _heathensCounter = 0;
int _prudesCounter = 0;
int _status = NEUTRAL;
pthread_mutex_t _mutex = PTHREAD_MUTEX_INITIALIZER;
sem_t _heathenTurn;
sem_t _prudesTurn;
sem_t _heathenQueue;
sem_t _prudesQueue;

void sem_post_many(sem_t *semaforo, int many) {
	while (many-- > 0)
		sem_post(semaforo);
}

//Novas funções (Luid)
//Mantive as antigas abaixo, porque não sei se isso está certo.
void *heathens(){
  //lógica tomada: cada thread é uma "pessoa", ou seja, 
  //quando a thread é chamada significa que uma pessoa chegou, logo, a incrementação independe se é a vez dele ou não
    _heathensCounter++;
    printf("\nCHEGOU 1 HEATENS\n");
    
    sem_wait(&_heathenTurn);
    pthread_mutex_lock(&_mutex);

    _heathensCounter--;
    printf("\nHEATHENS PASSANDO\nFaltam %d Prudes\t\te\t%d Heathens\n", _prudesCounter, _heathensCounter);

    if(_prudesCounter > _heathensCounter){
        printf("\nMudando a vez para Prudes\n");
        sem_post(&_prudesTurn);
        // _status = PRUDES_RULE;
    } else {
        if(_heathensCounter){
            sem_post(&_heathenTurn);
        } else {
            printf("Acabou os Heathens e também não tem mais Prudes.\n");
            // _status = NEUTRAL;
            // sem_post(&_heathenTurn);
            sem_post(&_prudesTurn);
        }
    }

    pthread_mutex_unlock(&_mutex);
    pthread_exit(0);

}

void *prudes(){
    // sleep(1);
    _prudesCounter++;
    printf("\nCHEGOU 1 PRUDE\n");
    
    sem_wait(&_prudesTurn);
    pthread_mutex_lock(&_mutex);

    _prudesCounter--;
    printf("\nPRUDES PASSANDO\nFaltam %d Prudes\t\te\t%d Heathens\n", _prudesCounter, _heathensCounter);

    if(_heathensCounter > _prudesCounter){
        printf("\nMudando a vez para Heathens\n");
        sem_post(&_heathenTurn);
        // _status = HEATHENS_RULE;
    } else{
        if(_prudesCounter){
            sem_post(&_prudesTurn);
        } else{
            printf("Acabou os Prudes e também não tem mais Heathens.\n");
            // _status = NEUTRAL;
            sem_post(&_heathenTurn);
            // sem_post(&_prudesTurn);
            
        }
    }
    pthread_mutex_unlock(&_mutex);
    pthread_exit(0);
    

}


void *heathens_old()
{ 
  sem_wait(&_heathenTurn);
  sem_post(&_heathenTurn);

  pthread_mutex_lock(&_mutex);
  printf("Passando pelo mutex do heathens\n");
  _heathensCounter++;
  printf("\nH STATUS: %d\n", _status);
  printf("Quantidade de heathens = %d\tQuantidade de prudes = %d\n", _heathensCounter, _prudesCounter);
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
          printf("H: TRANSITION_TO_HEATHENS -> HEATHENS_RULE\n");
          pthread_mutex_unlock(&_mutex);
          // _status = HEATHENS_RULE;
          sem_wait(&_heathenQueue);
        break;
      default:
        // pthread_mutex_unlock(&_mutex);
        printf("a");  
  }
  // sleep(1);
  pthread_mutex_lock(&_mutex);
  printf("\nHeathens passando\n");
  printf("Quantidade de heathens = %d\tQuantidade de prudes = %d\n", _heathensCounter, _prudesCounter);
  _heathensCounter--;

  if (_heathensCounter == 0){
    if (_status == TRANSITION_TO_PRUDES){
      sem_post(&_prudesTurn);
      int valorTurn;
      sem_getvalue(&_heathenTurn, &valorTurn);
      printf("H -- TRANSITION_TO_PRUDES; heathenTurn = %d", valorTurn);
    }
    if(_prudesCounter){
      sem_post_many(&_prudesQueue, _prudesCounter);
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


void *prudes_old()
{

  sem_wait(&_prudesTurn);
  sem_post(&_prudesTurn);

  pthread_mutex_lock(&_mutex);
  _prudesCounter++;
  printf("\nP STATUS: %d\n", _status);
  printf("Quantidade de heathens = %d\tQuantidade de prudes = %d\n", _heathensCounter, _prudesCounter);

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
  // sleep(1);
  printf("\nPrudes passando\n");
  printf("Quantidade de heathens = %d\tQuantidade de prudes = %d\n", _heathensCounter, _prudesCounter);
  _prudesCounter--;

  if (_prudesCounter == 0){
    if (_status == TRANSITION_TO_HEATHENS){
      sem_post(&_heathenTurn);
    }
    if(_heathensCounter){
      sem_post_many(&_heathenQueue,_prudesCounter);
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

  //Heathens tem a preferencia em passar caso o ambiente seja neutro
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
  
  printf("\n------------Inicio-------------\n");
  for (int i = 0; i < 20; i++){
    pthread_create(&_tidHeathens[i], NULL, &heathens, NULL);
    pthread_create(&_tidPrudes[i], NULL, &prudes, NULL);
  }

  for (int j = 0; j < 20; j++){
    pthread_join(_tidHeathens[j], NULL);
    pthread_join(_tidPrudes[j], NULL);
  }

  printf("\nTerminou\n");
   
  return 0;
}