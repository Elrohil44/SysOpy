#include "contactBook.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>



Contact toContact(const char* firstname, const char* lastname,
                   const char* birthdate, const char* mail,
                   const char* phone, const char* address)
{
  Contact c;
  c.firstname=malloc(strlen(firstname)+1);
  strcpy(c.firstname,firstname);
  c.lastname=malloc(strlen(lastname)+1);
  strcpy(c.lastname,lastname);
  c.birthdate=malloc(strlen(birthdate)+1);
  strcpy(c.birthdate,birthdate);
  c.mail=malloc(strlen(mail)+1);
  strcpy(c.mail,mail);
  c.phone=malloc(strlen(phone)+1);
  strcpy(c.phone,phone);
  c.address=malloc(strlen(address)+1);
  strcpy(c.address,address);
  return c;
}


void freeContact(Contact c)
{
  free(c.firstname);
  free(c.lastname);
  free(c.birthdate);
  free(c.mail);
  free(c.phone);
  free(c.address);
}

const char* getField(Contact c, int primaryid)
{
  switch(primaryid)
  {
    case 1:
      return c.lastname;
    case 2:
      return c.birthdate;
    case 3:
      return c.mail;
    case 4:
      return c.phone;
  }
  return c.lastname;
}

BSTBook* createBSTBook()
{
  BSTBook* tmp = (BSTBook*) malloc(sizeof(BSTBook));
  tmp->primaryid=1;
  tmp->c = NULL;
  return tmp;
}

DLBook* createDLBook(Contact c)
{
  DLBook* tmp = (DLBook*) malloc(sizeof(DLBook));
  tmp->c = NULL;
  return tmp;
}

void deleteTree(BSTContact* c)
{
  if (c==NULL) return;
  deleteTree(c->left);
  deleteTree(c->right);
  freeContact(c->c);
  free(c);
}

void deleteBSTBook(BSTBook* b)
{
  if (b==NULL) return;
  if(b->c != NULL) deleteTree(b->c);
  free(b);
}


void deleteDLBook(DLBook* b)
{
  if (b==NULL) return;
  if (b->c == NULL)
  {
    free(b);
    return;
  }
  DLContact* tmp = b->c->next;
  freeContact(b->c->c);
  free(b->c);
  while (tmp != NULL)
  {
    b->c=tmp;
    tmp=tmp->next;
    freeContact(b->c->c);
    free(b->c);
  }
  free(b);
}



void addBSTContact(BSTBook* b , const char* firstname, const char* lastname,
                   const char* birthdate, const char* mail,
                   const char* phone, const char* address)
{
      if (b==NULL) return;
      Contact  c = toContact(firstname, lastname, birthdate, mail, phone, address);
      BSTContact* curr = b->c;
      BSTContact* parent = NULL;
      while(curr!=NULL)
      {
        parent=curr;
        if(strcmp(getField(curr->c,b->primaryid),getField(c,b->primaryid))>=0)
        {
          curr = curr->right;
        }
        else curr = curr->left;
      }
      if (parent==NULL)
      {
        b->c = (BSTContact*) malloc (sizeof(BSTContact));
        b->c->c = c;
        b->c->left = b->c->right = NULL;
        b->c->parent = NULL;
        return;
      }
      if(strcmp(getField(parent->c,b->primaryid),getField(c,b->primaryid))>=0)
      {
        parent->right =(BSTContact*) malloc (sizeof(BSTContact));
        curr = parent->right;
      }
      else
      {
        parent->left =(BSTContact*) malloc (sizeof(BSTContact));
        curr = parent->left;
      }
      curr->c = c;
      curr->parent=parent;
      curr->left=curr->right=NULL;
}

void addDLContact(DLBook* b, const char* firstname, const char* lastname,
                   const char* birthdate, const char* mail,
                   const char* phone, const char* address)
{
  if (b==NULL) return;
  Contact c = toContact(firstname, lastname, birthdate, mail, phone, address);
  DLContact* curr =(DLContact*) malloc (sizeof(DLContact));
  curr->c=c;
  curr->prev=NULL;
  curr->next=b->c;
  if(b->c!=NULL) b->c->prev=curr;
  b->c=curr;
}

BSTContact* succ(BSTContact* c)
{
  if(c==NULL) return NULL;
  BSTContact* curr=c;
  if(c->right!=NULL)
  {
    curr=curr->right;
    while(curr->left!=NULL) curr=curr->left;
    return curr;
  }
  while(curr->parent!=NULL && curr->parent->right==curr)
  {
    curr=curr->parent;
  }
  return curr->parent;
}


BSTContact* pred(BSTContact* c)
{
  if(c==NULL) return NULL;
  BSTContact* curr=c;
  if(c->left!=NULL)
  {
    curr=curr->left;
    while(curr->right!=NULL) curr=curr->right;
    return curr;
  }
  while(curr->parent!=NULL && curr->parent->left==curr)
  {
    curr=curr->parent;
  }
  return curr->parent;
}

void replaceNode(BSTBook* b, BSTContact* c, BSTContact* newpointer)
{
  BSTContact* parent=c->parent;
  if (parent==NULL)
  {
    b->c=newpointer;
    if(newpointer!=NULL) newpointer->parent=NULL;
  }
  else
  {
    if(c==parent->left) parent->left=newpointer;
    else parent->right=newpointer;
    if(newpointer!=NULL) newpointer->parent=parent;
  }
  free(c);
}

void deleteBSTContact(BSTBook* b, BSTContact* c)
{
  if(c==NULL) return;
  freeContact(c->c);
  if(c->left == NULL && c-> right==NULL)
  {
    replaceNode(b,c,NULL);
  }
  else if(c->left == NULL)
  {
    replaceNode(b,c,c->right);
  }
  else if(c->right == NULL)
  {
    replaceNode(b,c,c->left);
  }
  else
  {
    BSTContact* succ1 = succ(c);
    c->c=succ1->c;
    if (succ1->parent==c)
    {
      c->right=succ1->right;
      if(succ1->right!=NULL)
        succ1->right->parent=c;
    }
    else
    {
      succ1->parent->left=succ1->right;
      if(succ1->right!=NULL)
        succ1->right->parent=succ1->parent;
    }
    free(succ1);
  }
}
void deleteDLContact(DLBook* b, DLContact* c)
{
  if(c==NULL) exit;
  freeContact(c->c);
  if (c==b->c)
  {
    b->c=c->next;
    if (b->c!=NULL) b->c->prev=NULL;
  }
  else if (c->next==NULL)
  {
    c->prev->next=NULL;
  }
  else
  {
    c->prev->next=c->next;
    c->next->prev=c->prev;
  }
  free(c);
}

BSTContact* searchBSTBook(BSTBook* b,const char* name)
{
  if(b==NULL) return NULL;
  BSTContact* c = b->c;
  if (c==NULL) return NULL;
  if(b->primaryid==1)
  {
    while(c!=NULL)
    {
      if(strcmp(c->c.lastname,name)>0) c=c->right;
      else if(strcmp(c->c.lastname,name)<0) c=c->left;
      else return c;
    }
  }
  else
  {
    while(c->left!=NULL) c=c->left;
    while(c!=NULL && strcmp(name, c->c.lastname)!=0) c=succ(c);
  }
  return c;
}
DLContact* searchDLBook(DLBook* b,const char* name)
{
  if (b==NULL) return NULL;
  DLContact* c=b->c;
  while(c!=NULL && strcmp(name, c->c.lastname)!=0)
  {
    c=c->next;
  }
  return c;
}

void addBSTNode (BSTContact** root, BSTContact* node, int primaryid)
{
  if (node==NULL) return;
  addBSTNode(root,node->left,primaryid);
  addBSTNode(root,node->right,primaryid);
  BSTContact* curr=*root;
  BSTContact* parent = NULL;
  node->parent=node->left=node->right=NULL;

  if(*root==NULL)
  {
    *root=node;
  }
  else
  {
    while(curr!=NULL)
    {
      parent=curr;
      if(strcmp(getField(curr->c,primaryid),getField(node->c,primaryid))>=0)
      {
        curr = curr->right;
      }
      else curr = curr->left;
    }

    if(strcmp(getField(parent->c,primaryid),getField(node->c,primaryid))>=0)
    {
      parent->right=node;
    }
    else
    {
      parent->left=node;
    }
    node->parent=parent;
  }
}

void rebuild(BSTBook* b, int fieldID)
{
  if (b==NULL) return;
  b->primaryid=fieldID;
  BSTContact* root=NULL;
  addBSTNode(&root,b->c,fieldID);
  b->c=root;
}

void removeNode(DLBook* b, DLContact* node)
{
  if (node==NULL) return;
  if(node->prev==NULL)
  {
    if (node->next!=NULL)
    {
      node->next->prev=NULL;
    }
    b->c=node->next;
  }
  else if(node->next==NULL)
  {
    node->prev->next=NULL;
  }
  else
  {
    node->prev->next=node->next;
    node->next->prev=node->prev;
  }
}

void sort(DLBook* b, int fieldID)
{
  if (b==NULL) return;
  if (b->c==NULL) return;
  DLContact* tmp = NULL;
  DLContact* max;
  DLContact* curr;
  while(b->c!=NULL)
  {
    max=b->c;
    curr=b->c;
    while(curr!=NULL)
    {
      if(strcmp(getField(curr->c,fieldID),getField(max->c,fieldID))>=0)
        max = curr;
      curr=curr->next;
    }
    removeNode(b,max);
    max->next=tmp;
    max->prev=NULL;
    if(tmp!=NULL) tmp->prev=max;
    tmp=max;
  }
  b->c=tmp;
}
