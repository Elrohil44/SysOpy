#ifndef CONTACTBOOK_H
#define CONTACTBOOK_H


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

void freeContact(Contact c);
Contact toContact(const char* firstname, const char* lastname,
                   const char* birthdate, const char* mail,
                   const char* phone, const char* address);
BSTBook* createBSTBook();
DLBook* createDLBook();
void deleteBSTBook(BSTBook* b);
void deleteDLBook(DLBook* b);
void addBSTContact(BSTBook* b , const char* firstname, const char* lastname,
                   const char* birthdate, const char* mail,
                   const char* phone, const char* address);
void addDLContact(DLBook* b, const char* firstname, const char* lastname,
                   const char* birthdate, const char* mail,
                   const char* phone, const char* address);
void deleteBSTContact(BSTBook* b, BSTContact* c);
void deleteDLContact(DLBook* b, DLContact* c);

BSTContact* searchBSTBook(BSTBook* b,const char* name);
DLContact* searchDLBook(DLBook* b,const char* name);

void rebuild(BSTBook* b, int fieldID);
void sort(DLBook* b, int fieldID);


#endif
