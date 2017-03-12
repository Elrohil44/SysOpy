#ifndef CONTACTBOOK_H
#define CONTACTBOOK_H

struct BSTContact;
typedef struct BSTContact BSTContact;
struct DLContact;
typedef struct DLContact DLContact;
struct Contact;
typedef struct Contact Contact;
struct BSTBook;
typedef struct BSTBook BSTBook;
struct DLBook;
typedef struct DLBook DLBook;

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
Contact deleteBSTContact(BSTBook* b, BSTContact* c);
Contact deleteDLContact(DLBook* b, DLContact* c);

BSTContact* searchBSTBook(BSTBook* b, char* name);
DLContact* searchDLBook(DLBook* b, char* name);

void rebuild(BSTBook* b, int fieldID);
void sort(DLBook* b, int fieldID);


#endif
