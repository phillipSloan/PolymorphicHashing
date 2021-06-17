#include "specific.h"
#include "../assoc.h"

#define SCALEFACTOR 2

/**               PRIVATE FUNCTIONS                      */
void test(void);
bool _testHashFunctions(int* hash, int* rehash, int capacity);

void _cuckooArr1(assoc** a, void* key, void* data, int bounce);
void _cuckooArr2(assoc** a, void* key, void* data, int bounce);
void _assocResizeHashTables(assoc** a);
/*        Hashing functions        */
int _hash(int cap, int len, void *s);
int _rehash(int cap, int key, void*s);
/*   Helper Functions   */
int _nextPrime(int prm);
bool _checkPrime(int prm);
int computeLog(int n);
bool _cmpKeys(assoc* b, void* key, int index);
bool _cmpKeysArr1(assoc* b, void* key, int index);
bool _cmpKeysArr2(assoc* b, void* key, int index);

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
   a->arr1 = (datatype*)ncalloc(INITSIZE, sizeof(datatype));
   a->arr2 = (datatype*)ncalloc(INITSIZE, sizeof(datatype));
   a->capacity = INITSIZE;
   a->ks = keysize;

   return a;
}

void assoc_insert(assoc** a, void* key, void* data)
{
   int hashIndex, bounce = 0;
   assoc* b = *a;
   void *oldData, *oldKey;
   b->inserted = false;

   if(!a || !key){
      on_error("Please enter valid inputs for assoc_insert.");
   }

   hashIndex = _hash(b->capacity, b->ks, key);
   /*if there's space in hash table, fill with key and data*/
   if (b->arr1[hashIndex].key == NULL){
      b->arr1[hashIndex].data = data;
      b->arr1[hashIndex].key = key;
      b->inserted = true;
   }
   else {
   /*if the key has been seen before, update the data only*/
      if(_cmpKeys(b, key, hashIndex)){
         b->arr1[hashIndex].data = data;
      }
      else {
      /*evict the old data and move to second array*/
         oldKey = b->arr1[hashIndex].key;
         oldData = b->arr1[hashIndex].data;
         b->arr1[hashIndex].data = data;
         b->arr1[hashIndex].key = key;
         bounce++;
         _cuckooArr2(a, oldKey, oldData, bounce);
      }
   }
   if(b->inserted){
      b->size++;
   }
}

unsigned int assoc_count(assoc* a)
{
   if(!a){
      on_error("Please enter valid input for assoc_count.");
   }
   return a->size;
}

void* assoc_lookup(assoc* a, void* key)
{
   int hashIndex;
   if(!a || !key){
      on_error("Please enter valid inputs for assoc_lookup.");
   }
   hashIndex = _hash(a->capacity, a->ks, key);
   if(_cmpKeysArr1(a, key, hashIndex)){
      return a->arr1[hashIndex].data;
   }

   hashIndex = _rehash(a->capacity, a->ks, key);
   if(_cmpKeysArr2(a, key, hashIndex)){
      return a->arr2[hashIndex].data;
   }
   return NULL;
}

void assoc_free(assoc* a)
{
   if (a == NULL){
      return;
   }

   if(a->arr1){
      free(a->arr1);
   }
   if(a->arr2){
      free(a->arr2);
   }
   free(a);
}

/*hash derived from lectures & djb2 @ cse.yorku.ca/~oz/hash.html*/
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
      hash = ((hash << 5) + hash) ^ c;
   }
   return((int)(hash%cap));
}

/* hash is SDBM from cse.yorku.ca/~oz/hash.html */
int _rehash(int cap, int key, void*s){
   unsigned char* ss = (unsigned char*)s;
   unsigned long hash = 65599;
   unsigned int c;
   int len, count = 0;

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
   return ((int)(hash%cap));
}

void _cuckooArr1(assoc** a, void* key, void* data, int bounce)
{
   int hashIndex;
   assoc* b = *a;
   void *oldData, *oldKey;
   /*if resize attempts gets above log(capacity) then resize*/
   if (bounce > computeLog(b->capacity)){
      _assocResizeHashTables(a);
   }

   hashIndex = _hash(b->capacity, b->ks, key);
   /*if hashed index is empty, fill with data*/
   if(b->arr1[hashIndex].key == NULL){
      b->arr1[hashIndex].data = data;
      b->arr1[hashIndex].key = key;
      b->inserted = true;
   }
   else {
   /*if the key is the same, update the data only*/
      if(_cmpKeysArr1(b, key, hashIndex)){
         b->arr1[hashIndex].data = data;
      }
      else {
      /*evict the old data and move to other array*/
         oldKey = b->arr1[hashIndex].key;
         oldData = b->arr1[hashIndex].data;
         b->arr1[hashIndex].data = data;
         b->arr1[hashIndex].key = key;
         bounce++;
         _cuckooArr2(a, oldKey, oldData, bounce);
      }
   }
}

void _cuckooArr2(assoc** a, void* key, void* data, int bounce)
{
   int hashIndex;
   assoc* b = *a;
   void *oldData, *oldKey;
   /*if resize attempts gets above log(capacity) then resize*/
   if (bounce > computeLog(b->capacity)){
      _assocResizeHashTables(a);
   }

   hashIndex = _rehash(b->capacity, b->ks, key);
   /*if hashed index is empty, fill with data*/
   if(b->arr2[hashIndex].key == NULL){
      b->arr2[hashIndex].data = data;
      b->arr2[hashIndex].key = key;
      b->inserted = true;
   }
   else {
      if(_cmpKeysArr2(b, key, hashIndex)){
         b->arr2[hashIndex].data = data;
      }
      else {
         oldKey = b->arr2[hashIndex].key;
         oldData = b->arr2[hashIndex].data;
         b->arr2[hashIndex].data = data;
         b->arr2[hashIndex].key = key;
         bounce++;
         _cuckooArr1(a, oldKey, oldData, bounce);
      }
   }
}

void _assocResizeHashTables(assoc** a)
{
   int oldCap, hashIndex, j, bounce = 0;
   assoc* b = *a;
   datatype* tmp1 = b->arr1;
   datatype* tmp2 = b->arr2;
   void *oldKey, *oldData;

   oldCap = b->capacity;
   b->capacity = _nextPrime(b->capacity * SCALEFACTOR);
   b->arr1 = (datatype*)ncalloc(b->capacity, sizeof(datatype));
   b->arr2 = (datatype*)ncalloc(b->capacity, sizeof(datatype));

   /*rehash old data into resized array*/
   for (j = 0; j < oldCap; j++){
      if(tmp1[j].key != NULL){
         hashIndex = _hash(b->capacity, b->ks, tmp1[j].key);
         if(b->arr1[hashIndex].key == NULL){
            b->arr1[hashIndex].key = tmp1[j].key;
            b->arr1[hashIndex].data = tmp1[j].data;
         }
         else {
            oldKey = tmp1[j].key;
            oldData = tmp1[j].data;
            _cuckooArr2(a, oldKey, oldData, bounce);
         }
      }
   }
   for (j = 0; j < oldCap; j++){
      if(tmp2[j].key != NULL){
         hashIndex = _rehash(b->capacity, b->ks, tmp2[j].key);
         if(b->arr2[hashIndex].key == NULL){
            b->arr2[hashIndex].key = tmp2[j].key;
            b->arr2[hashIndex].data = tmp2[j].data;
         }
         else {
            oldKey = tmp2[j].key;
            oldData = tmp2[j].data;
            _cuckooArr1(a, oldKey, oldData, bounce);
         }
      }
   }
   free(tmp1);
   free(tmp2);
}

bool _cmpKeys(assoc* b, void* key, int index)
{
   if (_cmpKeysArr1(b, key, index)){
      return true;
   }
   if(_cmpKeysArr2(b, key, index)){
      return true;
   }
   return false;
}

bool _cmpKeysArr1(assoc* b, void* key, int index)
{
   if (b->ks == STRING){
      if((b->arr1[index].key != NULL) &&
        (strcmp((char*)b->arr1[index].key,(char*)key)) == 0){
         return true;
      }
   }
   else {
      if((b->arr1[index].key != NULL) &&
        (memcmp(b->arr1[index].key, key, b->ks) == 0)){
         return true;
      }
   }
   return false;
}

bool _cmpKeysArr2(assoc* b, void* key, int index)
{
   if (b->ks == STRING){
      if((b->arr2[index].key != NULL) &&
        (strcmp((char*)b->arr2[index].key,(char*)key)) == 0){
         return true;
      }
   }
   else {
      if((b->arr2[index].key != NULL) &&
        (memcmp(b->arr2[index].key, key, b->ks) == 0)){
         return true;
      }
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

int computeLog(int n)
{
   return (int)(log((double)n)/log(2));
}


void test(void)
{
   int i, hash = 0, rehash = 0, primeNum = 3, idx;
   assoc* a;
   FILE *fp;
   char words[50][50] = {{0}};

   fp = nfopen("../../Data/Words/eng_370k_shuffle.txt", "rt");
   for(i = 0; i < 50; i++){
      if(fscanf(fp, "%s", words[i]) != 1){
         on_error("Can't read in words");
      }
   }
   fclose(fp);

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
   assert(a->arr1);
   assert(a->arr2);
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
   assert(_cmpKeys(a, words[0], idx));
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
