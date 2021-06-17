#include <stdlib.h>
#include <math.h>

/*initial size of datatype hash table/array*/
#define INITSIZE 17
/*when keysize is 0, the ADT will expect strings*/
#define STRING 0

typedef enum bool {false, true} bool;

/*data pair / hash table*/
typedef struct datatype{
   void* key;
   void* data;
} datatype;

typedef struct assoc {
   struct datatype* arr;
   /*amount of data pairs within hash table*/
   int size;
   /*maximum current capacity*/
   int capacity;
   /*keysize*/
   int ks;
} assoc;
