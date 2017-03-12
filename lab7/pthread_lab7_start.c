#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <time.h>

#define NUM_THREADS 5
#define NUM_BROKERS 4
#define NUM_STOCKS 3

//struct for stock on the exchange
struct stock {
  char name;
  float price;
  int count;
};

//struct for investor stock details
struct investor_stock {
  char name;
  float buy_price;
  float buy_count;
  float sell_price;
  float sell_count;
};

struct investor_portfolio {
  struct investor_stock s[NUM_STOCKS];
};

//declare shared variables and corresponding
//mutexes and condition variables
struct stock my_stock[NUM_STOCKS] = {{'A', 0.01, 1000},
                           {'B', 0.01, 500},
                           {'C', 0.01, 1000}} ;
pthread_mutex_t my_stock_mutex[NUM_STOCKS];
pthread_cond_t my_stock_price_cond[NUM_STOCKS];

int market_running = 1;

//declare routines for threads...
void *stock_broker(void *t);
void *stock_monitor(void *t);
void *stock_exchange(void *t);

void update_stock(int id, int chance, float price_var);

int main(void) {
  /* create each stock broker's portfolio */
  struct investor_portfolio portfolios[NUM_BROKERS];
  {
    struct investor_stock s00 = {'A', 1.10, 5, 100.00, 5};
    portfolios[0].s[0] = s00;
    struct investor_stock s01 = {'B', -1, -1, -1, -1}; //no buy or sell
    portfolios[0].s[1] = s01;
    struct investor_stock s02 = {'C', 5.00, 10, 10.00, 6};
    portfolios[0].s[2] = s02;

    struct investor_stock s10 = {'A',0.90f,6,95.00f,5};
    portfolios[1].s[0] = s10;
    struct investor_stock s11 = {'B',2.50f,3,140.0f,8};
    portfolios[1].s[1] = s11;
    struct investor_stock s12 = {'C',-1,-1,-1,-1};
    portfolios[1].s[2] = s12;

    struct investor_stock s20 = {'A',-1,-1,120.0f,7};
    portfolios[2].s[0] = s20;
    struct investor_stock s21 = {'B',1.50f,8,80.0f,5};
    portfolios[2].s[1] = s21;
    struct investor_stock s22 = {'C',-1,-1,86.0f,20};
    portfolios[2].s[2] = s22;

    struct investor_stock s30 = {'A',0.90f,6,95.00f,5};
    portfolios[3].s[0] = s30;
    struct investor_stock s31 = {'B',2.50f,3,140.0f,8};
    portfolios[3].s[1] = s31;
    struct investor_stock s32 = {'C',4.78f,9,-1,-1};
    portfolios[3].s[2] = s32;
  }

  int i, error;
  pthread_t threads[NUM_THREADS];
  pthread_attr_t attr;

  //Initialize mutex and condition variables
  for (i=0; i < NUM_STOCKS; i++) {
    pthread_mutex_init(&my_stock_mutex[i], NULL);
    pthread_cond_init(&my_stock_price_cond[i], NULL);
  }

  //Create threads that are joinable
  pthread_attr_init(&attr);
  pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
  error = pthread_create(&threads[0], &attr, stock_exchange, NULL);
  if (error) {
    printf("ERROR: create() monitor_stock_price: %d\n", error);
    exit(-1);
  }

  //Activity #3
  for(i = NUM_THREADS - NUM_BROKERS; i < NUM_THREADS;++i){
    error = pthread_create(&threads[i], &attr, stock_broker, (void *) &portfolios[i - 1]);
    if (error) {
      printf("ERROR: create() buy_stock: %d %d\n",i, error);
      exit(-1);
    }
  }

  //wait for threads to complete/join
  for (i=0; i < NUM_THREADS; i++) {
    error = pthread_join(threads[i], NULL);
    if (error) {
      printf("ERROR: join() thread %d: %d\n", i, error);
      exit(-1);
    }
  }

  //reinitialize/destroy pthread variables
  pthread_attr_destroy(&attr);
  for (i=0; i < NUM_STOCKS; i++) {
    pthread_mutex_destroy(&my_stock_mutex[i]);
    pthread_cond_destroy(&my_stock_price_cond[i]);
    }
  pthread_exit(NULL);
}

#define MONITOR_THREADS 3

/* struct used as parameter type for stock monitor threads */
struct stock_monitor_data{
  int id;
  struct investor_stock is;
};

/* simulates a broker, takes an investor_stock struct as an argument */
void *stock_broker(void *t) {
  int i, error;
  pthread_t threads[MONITOR_THREADS];
  pthread_attr_t attr;

  struct stock_monitor_data monitor_data[MONITOR_THREADS];

  /* want to be able to join the monitor threads */
  pthread_attr_init(&attr);
  pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_JOINABLE);

  //Activity #2
  /* create stock monitors for each individual stock that the broker
   * is interested in
   */
  for(i = 0; i < MONITOR_THREADS;++i){
    monitor_data[i].id = i;
    monitor_data[i].is = ((struct investor_portfolio *) t)->s[i];
    if((error = pthread_create(&threads[i],&attr, stock_monitor,&monitor_data[i]))){
      fprintf(stderr,"ERROR: create() stock_monitor: %d\n",error);
      exit(-1);
    }
  }

  /* join stock monitors when they are finished */
  for(i = 0; i < MONITOR_THREADS; ++i){
    if((error = pthread_join(threads[i],NULL))){
      fprintf(stderr,"ERROR: join stock_monitor %d: %d\n",i,error);
      exit(-1);
    }
  }

  pthread_exit(NULL);
}

/* monitors a single stock from a portfolio */
void *stock_monitor(void *t){
  struct stock_monitor_data *smd = (struct stock_monitor_data *) t;
  while(market_running){
    pthread_mutex_lock(&my_stock_mutex[smd->id]);
    /* wait until price changes */
    pthread_cond_wait(&my_stock_price_cond[smd->id],&my_stock_mutex[smd->id]);

    /* buy if price is cheap and there's available stock */
    if(my_stock[smd->id].price < smd->is.buy_price && smd->is.buy_price > 0.0f){
      my_stock[smd->id].count -= (my_stock[smd->id].count > smd->is.buy_count) ?
        smd->is.buy_count: my_stock[smd->id].count;
      printf("Purchasing %c\n",smd->is.name);
    }

    /* sell if price is past a threshold */
    if(my_stock[smd->id].price > smd->is.sell_price && smd->is.sell_price > 0.0f){
      my_stock[smd->id].count += smd->is.sell_count;
      printf("Selling %c\n",smd->is.name);
    }

    pthread_mutex_unlock(&my_stock_mutex[smd->id]);
  }

  pthread_exit(NULL);
}

void *stock_exchange(void *t) {
  //check the value of a stock and signal waiting threads
  //when a price is reached
  printf("Starting of trading on Stock %c\n", my_stock[0].name);
  int day;
  srand(time(NULL));
  for (day=0; day < 365; day++) {
    printf("day = %d\n", day);
    update_stock(0, rand()%3, (rand()%10 - 3.6)/2.3);
    update_stock(1, rand()%7, (rand()%12 - 5)/2.3);
    update_stock(2, rand()%6, (rand()%7 - 1)/2.1);
    sleep(1);
  }
  market_running = 0;
  /* send final signal that market is finished */
  sleep(2);
  for(int i = 0; i < NUM_STOCKS; ++i){
    pthread_mutex_lock(&my_stock_mutex[i]);
    pthread_cond_broadcast(&my_stock_price_cond[i]);
    pthread_mutex_unlock(&my_stock_mutex[i]);
  }
  pthread_exit(NULL);
}


void update_stock(int id, int chance, float price_var) {
  if (chance == 0) {
    //Activity #1
    pthread_mutex_lock(&my_stock_mutex[id]);
    const float newprice = my_stock[id].price + price_var;
    /* price can't go under 1 cent */
    if(newprice > 0.0f){
      my_stock[id].price = newprice;
    }else{
      my_stock[id].price = 0.01f;
    }
    printf("Stock price of %c: $%.2f\n",my_stock[id].name,my_stock[id].price);
    pthread_cond_broadcast(&my_stock_price_cond[id]);
    pthread_mutex_unlock(&my_stock_mutex[id]);
  }
}
