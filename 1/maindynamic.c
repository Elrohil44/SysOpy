//#include "lib/contactBook.h"
#include <dlfcn.h>
#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include <sys/times.h>

struct Contact;
typedef struct Contact Contact;
struct BSTContact;
typedef struct BSTContact BSTContact;
struct DLContact;
typedef struct DLContact DLContact;
struct BSTBook;
typedef struct BSTBook BSTBook;
struct DLBook;
typedef struct DLBook DLBook;

struct Contact
{
  char* firstname;
  char* lastname;
  char* birthdate;
  char* mail;
  char* phone;
  char* address;
};


struct BSTContact
{
  BSTContact *parent, *left, *right;
  Contact c;
};

struct DLContact
{
  DLContact *next, *prev;
  Contact c;
};

struct BSTBook
{
  BSTContact* c;
  int primaryid;
};

struct DLBook
{
  DLContact* c;
};

clock_t start_time;
clock_t last_time;
clock_t a_time;
static struct tms start_tms;
static struct tms last_tms;
static struct tms a_tms;

void getStartTime()
{
  start_time = last_time = clock();
  times(&start_tms);
  last_tms = start_tms;
}

void print_times()
{
  times(&a_tms);
  a_time = clock();
  printf("real\tdelta_last= %.6lf, delta_start = %.6lf\n",
  (double) (a_time - last_time) / CLOCKS_PER_SEC, (double) (a_time - start_time) / CLOCKS_PER_SEC);
  printf("user\tdelta_last= %.6lf, delta_start = %.6lf\n",
  (double) (a_tms.tms_utime - last_tms.tms_utime) / CLOCKS_PER_SEC,
  (double) (a_tms.tms_utime - start_tms.tms_utime) / CLOCKS_PER_SEC);
  printf("sys\tdelta_last= %.6lf, delta_start = %.6lf\n",
  (double) (a_tms.tms_stime - last_tms.tms_stime) / CLOCKS_PER_SEC,
  (double) (a_tms.tms_stime - start_tms.tms_stime) / CLOCKS_PER_SEC);
  printf("#####\t##################################\n");
  last_time = a_time;
  last_tms = a_tms;
}

BSTBook* (*createBSTBook)();
void (*addBSTContact)();
void (*deleteBSTContact)();
BSTContact* (*searchBSTBook)();
void (*rebuild)();
void (*deleteBSTBook)();

DLBook* (*createDLBook)();
void (*addDLContact)();
void (*deleteDLContact)();
DLContact* (*searchDLBook)();
void (*sort)();
void (*deleteDLBook)();


int main()
{
  getStartTime();
  printf("Inicjacja:\n");
  srand(time(NULL));
  void* lib;
  lib = dlopen("./libdynamic.so", RTLD_NOW);
  *(void **)(&createBSTBook) = dlsym(lib,"createBSTBook");
  *(void **)(&addBSTContact) = dlsym(lib,"addBSTContact");
  *(void **)(&deleteBSTContact) = dlsym(lib,"deleteBSTContact");
  *(void **)(&searchBSTBook) = dlsym(lib,"searchBSTBook");
  *(void **)(&rebuild) = dlsym(lib,"rebuild");
  *(void **)(&deleteBSTBook) = dlsym(lib,"deleteBSTBook");

  *(void **)(&createDLBook) = dlsym(lib,"createDLBook");
  *(void **)(&addDLContact) = dlsym(lib,"addDLContact");
  *(void **)(&deleteDLContact) = dlsym(lib,"deleteDLContact");
  *(void **)(&searchDLBook) = dlsym(lib,"searchDLBook");
  *(void **)(&sort) = dlsym(lib,"sort");
  *(void **)(&deleteDLBook) = dlsym(lib,"deleteDLBook");

  const char* s[] = {
    "12324","2132","13287","09702","123213",
    "7865","9824","94380","63245","23059",
  };
  int l = sizeof(s)/sizeof(s[0]);
  print_times();
  printf("Tworzenie ksiazki BST:\n");
  BSTBook* bstbook = (*createBSTBook)();
  print_times();
  printf("Dodawanie pojedynczych elementow BST:\n");
  for(int i=0;i<1000;i++)
  {
    (*addBSTContact)(bstbook,s[rand()%l],s[rand()%l],s[rand()%l],s[rand()%l],s[rand()%l],s[rand()%l]);
    print_times();
  }
  printf("Usuwanie pojedynczych elementow BST:\n");
  for(int i=0;i<100;i++)
  {
    (*deleteBSTContact)(bstbook,bstbook->c);
    print_times();
  }
  printf("Wyszukiwanie elementu BST:\n");
  (*searchBSTBook)(bstbook,s[0]);
  print_times();
  (*searchBSTBook)(bstbook,"aa");
  print_times();
  printf("Przebudowanie BST:\n");
  (*rebuild)(bstbook,3);
  print_times();
  printf("Wyszukiwanie elementu w przebudowanym BST:\n");
  (*searchBSTBook)(bstbook,s[0]);
  print_times();
  (*searchBSTBook)(bstbook,"aa");
  print_times();
  printf("Usuwanie BST:\n");
  (*deleteBSTBook)(bstbook);
  print_times();

  printf("Tworzenie ksiazki DL:\n");
  DLBook* dlbook = (*createDLBook)();
  print_times();
  printf("Dodawanie pojedynczego elementu DL:\n");
  (*addDLContact)(dlbook,s[rand()%l],s[rand()%l],s[rand()%l],s[rand()%l],s[rand()%l],s[rand()%l]);
  DLContact* last = dlbook->c;
  print_times();
  for(int i=0;i<1000;i++)
  {
    (*addDLContact)(dlbook,s[rand()%l],s[rand()%l],s[rand()%l],s[rand()%l],s[rand()%l],s[rand()%l]);
    print_times();
  }
  printf("Usuwanie elementu ze srodka DL:\n");
  for(int i=0;i<100;i++)
  {
    (*deleteDLContact)(dlbook,dlbook->c->next);
    print_times();
  }
  printf("Usuwanie elementu z poczatku DL:\n");
  (*deleteDLContact)(dlbook,dlbook->c);
  print_times();
  printf("Usuwanie elementu z konca DL:\n");
  (*deleteDLContact)(dlbook,last);
  print_times();
  printf("Wyszukiwanie elementu DL:\n");
  (*searchDLBook)(dlbook,s[0]);
  print_times();
  (*searchDLBook)(dlbook,"aa");
  print_times();
  printf("Sortowanie DL:\n");
  (*sort)(dlbook,2);
  print_times();
  printf("Usuwanie DL:\n");
  (*deleteDLBook)(dlbook);
  print_times();
  int state = dlclose(lib);
  if(state) return 1;
  return 0;
}
