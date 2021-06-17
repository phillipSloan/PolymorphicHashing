#include "specific.h"
#include "../assoc.h"

#define SCALEFACTOR 2

/*                    TESTING                             */
void test(void);
bool _testHashFunctions(int* hash, int* rehash, int capacity);
/**               PRIVATE FUNCTIONS                      */
/*        Hashing functions - Double Hashing used        */
int _hash(int cap, int len, void *s);
int _rehash(int cap, int key, void*s);
/*   Functions resizing/rehashing of associative array   */
bool _isOverSixtyPercentFull(int size, int capacity);
int _nextPrime(int prm);
bool _checkPrime(int prm);
void _assocResizeHashTable(assoc** a);
bool _rehashIntoTable(assoc* b, void* key, void* data, int hash);
/*           Comparing keys with memcmp/strcmp           */
bool _compareKeys(assoc* b, void* key, int index);


assoc* assoc_init(int keysize)
{
   assoc* a;
/*   static int guard = 0;

   if(guard == 0){
      guard = 1;
      test();
   }
*/
   if (keysize < 0){
      on_error("Please enter a valid keysize.");
   }

   a = (assoc*)ncalloc(1, sizeof(assoc));
   a->arr = (datatype*)ncalloc(INITSIZE, sizeof(datatype));
   a->capacity = INITSIZE;
   a->ks = keysize;

   return a;
}

void assoc_insert(assoc** a, void* key, void* data)
{
   int hashIndex;
   assoc* b = *a;

   if(!a || !key){
      on_error("Please enter valid inputs for assoc_insert.");
   }

   if (_isOverSixtyPercentFull(b->size, b->capacity)){
      _assocResizeHashTable(a);
   }

   hashIndex = _hash(b->capacity, b->ks, key);
   /*if there's space, fill hash table with key and data*/
   if (b->arr[hashIndex].key == NULL){
      b->arr[hashIndex].data = data;
      b->arr[hashIndex].key = key;
      b->size++;
   }
   else {
      /*if the key is the same, only update the data*/
      if(_compareKeys(b, key, hashIndex)){
         b->arr[hashIndex].data = data;
      }
      else {
         /*otherwise rehash to find a space for the entry*/
         if(_rehashIntoTable(b, key, data, hashIndex)){
            b->size++;
         }
      }
   }
}

unsigned int assoc_count(assoc* a)
{
   if(!a){
      on_error("Please enter a valid input for assoc_count.");
   }

   return a->size;
}


void* assoc_lookup(assoc* a, void* key)
{
   int stepSize, hashIndex, rehashIndex;

   if(!a || !key){
      on_error("Please enter valid inputs for assoc_lookup.");
   }
   /*find hash index*/
   hashIndex = _hash(a->capacity, a->ks, key);
   /*if the key matches the first hash entry, return data*/
   if(_compareKeys(a, key, hashIndex)){
      return a->arr[hashIndex].data;
   }
   /*step through the hashtable until the key/NULL is found*/
   stepSize = _rehash(a->capacity, a->ks, key);
   rehashIndex = (hashIndex + stepSize) % a->capacity;
   while (a->arr[rehashIndex].key != NULL){
      if(_compareKeys(a, key, rehashIndex)){
         return a->arr[rehashIndex].data;
      }
   rehashIndex = (rehashIndex + stepSize) % a->capacity;
   }
   return NULL;
}

void assoc_free(assoc* a)
{
   if (a == NULL){
      return;
   }

   if(a->arr){
      free(a->arr);
   }
   free(a);
}

/*Derived from lectures & djb2 @ cse.yorku.ca/~oz/hash.html*/
int _hash(int cap, int key, void *s){
   unsigned char* ss = (unsigned char*)s;
   unsigned long hash = 5381;
   unsigned int c, len, count = 0;

   if (key == STRING){
      len = strlen(s);
   }
   else {
      len = key;
   }

   while((count++ < len)){
      c = (*ss++);
      hash = ((hash << 5) + hash) + c;
   }
   return((int)(hash%cap));
}

/* SDBM from cse.yorku.ca/~oz/hash.html */
int _rehash(int cap, int key, void*s){
   unsigned char* ss = (unsigned char*)s;
   unsigned long hash = 65599;
   unsigned int c;
   int len, count = 0, stepSize, linearProbe = 1;

   if (key == STRING){
      len = strlen(s);
   }
   else {
      len = key;
   }

   while((count++ < len)){
      c = (*ss++);
      hash = c + (hash << 6) + (hash << 16) - hash;
   }
   stepSize = (int)(hash%cap);
   /*stepSize cannot be zero or capacity otherwise it can
   enter an infinite loop.  If so, move to linear probing
   for that key.
   */
   if(stepSize == 0 || stepSize == cap){
      stepSize = linearProbe;
   }
   return stepSize;
}

bool _isOverSixtyPercentFull(int size, int capacity)
{
   double usedPercentage;
   int threshold = 60;

   usedPercentage = ((double)size / capacity)*100;
   if ((int)usedPercentage >= threshold){
      return true;
   }
   return false;
}

bool _checkPrime(int prm)
{
    int chk, firstPrime = 2;
    for (chk = firstPrime; chk < sqrt(prm); chk++){
       if (prm % chk == 0){
          return false;
       }
    }
    return true;
}

int _nextPrime(int prm)
{
   if(_checkPrime(prm)){
      return prm;
   }
   while(!_checkPrime(prm)){
      prm++;
   }
   return prm;
}

void _assocResizeHashTable(assoc** a)
{
   int oldCap, hashIndex, j;
   assoc* b = *a;
   datatype* tmp = b->arr;

   oldCap = b->capacity;
   b->capacity = _nextPrime(b->capacity * SCALEFACTOR);
   b->arr = (datatype*)ncalloc(b->capacity, sizeof(datatype));

   /*rehash old data*/
   for (j = 0; j < oldCap; j++){
      if(tmp[j].key != NULL){
         hashIndex = _hash(b->capacity, b->ks, tmp[j].key);
         if(b->arr[hashIndex].key == NULL){
            b->arr[hashIndex].key = tmp[j].key;
            b->arr[hashIndex].data = tmp[j].data;
         }
         else {
            _rehashIntoTable(b, tmp[j].key, tmp[j].data, hashIndex);
         }
      }
   }
   free(tmp);
}

bool _rehashIntoTable(assoc* b, void* key, void* data, int hash)
{
   int stepSize, rehashIndex;
   stepSize = _rehash(b->capacity, b->ks, key);
   rehashIndex = (hash + stepSize) % b->capacity;
   while (b->arr[rehashIndex].key != NULL){
      if(_compareKeys(b, key, rehashIndex)){
         b->arr[rehashIndex].data = data;
         return false;
      }
      rehashIndex = (rehashIndex + stepSize) % b->capacity;
   }
   b->arr[rehashIndex].data = data;
   b->arr[rehashIndex].key = key;
   return true;
}

bool _compareKeys(assoc* b, void* key, int index)
{
   if (b->ks == STRING){
      if((b->arr[index].key != NULL) &&
        (strcmp((char*)b->arr[index].key,(char*)key)) == 0){
         return true;
      }
   }
   else {
      if((b->arr[index].key != NULL) &&
        (memcmp(b->arr[index].key, key, b->ks) == 0)){
         return true;
      }
   }
   return false;
}

void test(void)
{
   int i, hash = 0, rehash = 0, primeNum = 3, idx;
   assoc* a;
   FILE *fp;
   char words[50][50] = {{0}};
   int arr[100];
   double dblArr[100];
   short shrtArr[100];
   long longArr[100];
   int* datapointer;

   fp = nfopen("../../Data/Words/eng_370k_shuffle.txt", "rt");
   for(i = 0; i < 50; i++){
      if(fscanf(fp, "%s", words[i]) != 1){
         on_error("Can't read in words");
      }
   }
   fclose(fp);

   /*testing threshold (59% & 60%)*/
   assert(_isOverSixtyPercentFull(60, 100));
   assert(!_isOverSixtyPercentFull(59, 100));

   /*checking Prime functions*/
   assert(_checkPrime(primeNum));
   assert(!_checkPrime(16));
   assert(!_checkPrime(160));

   for (i = 0; i < 250; i++){
      _nextPrime(primeNum);
      assert(_checkPrime(primeNum));
   }

   /*testing hash functions */
   _testHashFunctions(&hash, &rehash, INITSIZE);
   assert(hash < INITSIZE);
   assert(rehash < INITSIZE);

   a = (assoc_init(0));
   assert(a);
   assert(a->arr);
   assert(a->capacity == INITSIZE);
   assert(!a->size);
   assert(a->ks == STRING);

   /*duplicates don't get added*/
   assoc_insert(&a, words[0], NULL);
   assert(a->size == 1);
   assoc_insert(&a, words[0], NULL);
   assert(a->size == 1);

   /*use hash function to find where word[0] has been stored*/
   idx = _hash(INITSIZE, STRING, words[0]);
   assert(_compareKeys(a, words[0], idx));
   assoc_free(a);

   /*testing decimals*/
   a = (assoc_init(sizeof(double)));
   assert(a);
   assert(a->ks == sizeof(double));

   for (i=0; i < 50; i++){
      dblArr[i] = (rand()/(double)(RAND_MAX));
      assoc_insert(&a, &dblArr[i], NULL);
   }
   assert(a->size = i);
   idx = _hash(a->capacity, sizeof(double), &dblArr[5]);
   assert(_compareKeys(a, &dblArr[5], idx));
   assoc_free(a);

   /*testing shorts (2 bytes)*/
   a = assoc_init(sizeof(short));
   for (i=0; i<50;i++){
      shrtArr[i] = i;
      arr[i] = i;
      assoc_insert(&a, &shrtArr[i], &arr[i]);
   }
   assert(a->size = i);
   idx = _hash(a->capacity, sizeof(short), &shrtArr[5]);
   assert(_compareKeys(a, &shrtArr[5], idx));
   datapointer = assoc_lookup(a, &shrtArr[5]);
   assert(*datapointer == 5);
   assoc_free(a);

   /*testing longs (8 byte)*/
   a = assoc_init(sizeof(long));
   for (i=0; i<50;i++){
      longArr[i] = rand();
      arr[i] = i;
      assoc_insert(&a, &longArr[i], &arr[i]);
   }
   assert(a->size = i);
   idx = _hash(a->capacity, sizeof(long), &longArr[5]);
   assert(_compareKeys(a, &longArr[5], idx));
   datapointer = assoc_lookup(a, &longArr[5]);
   assert(*datapointer == arr[5]);
   assoc_free(a);

}

/*
Using a histogram to find the highest hash  number
produced by the hash function to ensure it doesn't go above
capacity
*/
bool _testHashFunctions(int* hash, int* rehash, int capacity)
{
   int i, h1, h2, highestHash = 0;
   int hashHisto[INITSIZE] = {0};
   int rehashHisto[INITSIZE] = {0};
   char words[1000][50] = {{0}};
   FILE *fp;

   fp = fopen("../../Data/Words/eng_370k_shuffle.txt", "rt");

   for(i = 0; i < 1000; i++){
      if(fscanf(fp, "%s", words[i]) != 1){
         return false;
      }
   }
   fclose(fp);

   for (i = 0; i < 500; i++){
      h1 = _hash(capacity, STRING, words[i]);
      ++hashHisto[h1];
   }

   for (i = 500; i < 1000; i++){
      h2 = _hash(capacity, STRING, words[i]);
      ++rehashHisto[h2];
   }

   for(i = 0; i < INITSIZE; i++){
      if (hashHisto[i] > 0){
         highestHash = i;
      }
   }
   *hash = highestHash;
   highestHash = 0;

   for(i = 0; i < INITSIZE; i++){
      if (rehashHisto[i] > 0){
         highestHash = i;
      }
   }
   *rehash = highestHash;
   return true;
}
