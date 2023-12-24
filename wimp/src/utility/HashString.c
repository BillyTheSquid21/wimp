/*********** File: HashString.c *********************************************************/
/*  Purpose: Defines HashString Helper Functions Declared in HashString.h               */
/*                                                                                      */
/*  Author: jedi453                                                                     */
/****************************************************************************************/



#include <stdlib.h> /* malloc(), free(), NULL */
#include <string.h> /* strcmp(), strlen(), strcpy() */

#include "HashString.h" /* HashString type + Associated Functions */

/** @fn HashString* HashString_create( int size )
 *  @brief  Creates a HashString and Returns Pointer to it
 *
 *  Creates ( like malloc ) a Given HashString and its Table of Given Size
 *  and Returns a Pointer to that HashString or NULL Upon Failure
 *
 *  @param[in] size  int, the Number of Unique Table Entries for the HashString to Store
 *
 *  @return HashString* to the malloc'd HashString or NULL upon Failure
 */
HashString* HashString_create( int size )
{
  HashString* result; /**< The Resulting HashString */
  int i;
  
  /* malloc Container */
  result = (HashString*) malloc( sizeof(HashString) );
  if ( result == NULL )
    return (HashString*) NULL;  /* malloc Failed */

  /* malloc table */
  result->table = (HashStringEntry**) malloc(sizeof(HashStringEntry)*size);
  if ( result->table == NULL )
  {
    /* Table malloc failed, free Container, Return NULL */
    free( result );
    return (HashString*) NULL;
  }
  
  /* Set Size */
  result->size = size;

  /* Initialize Table Entries to NULL */
  for ( i = 0; i < size; ++i )
    result->table[i] = (HashStringEntry*) NULL;

  return result;
}




/** @fn void HashString_destroy( HashString* hash )
 *  @brief  Free a HashString and Entries from Memory
 *
 *  Frees a HashString and all of its Entries from Memory
 *
 *  @param[in] hash  HashString* to the HashString to Free
 *
 *  @return void
 */
void HashString_destroy( HashString* hash )
{
  HashStringEntry*  delEntry;  /**< The Entry Currently Being Deleted */
  HashStringEntry*  nextEntry; /**< The Next Entry */
  int i;
  
  /* Iterate through all the Base Elements in the Hash */
  for ( i = 0; i < hash->size; ++i )
  {
    delEntry = hash->table[i];
    /* while there's a Next Item in the Linked List */
    while ( delEntry != NULL )
    {
      nextEntry = delEntry->next;
      free( delEntry->key );
      //free( delEntry->value ); User will be responsible for this now
      free( delEntry );
      delEntry = nextEntry;
    }
  }
}






/* I chose the FNV1a Hash because it's Easy to Memorize and Seems to Still
 *  Keeps Up Well for Distribution, Speed and Collisions
 *  
 *  Compile with HASH_MURMUR Defined "cc -DHASH_MURMUR ..." to Use Murmur2 Hash
 *
 *  To See what I think is a Good Comparison of Some Popular Hash
 *   Functions, See Here:
 *  http://programmers.stackexchange.com/questions/49550/which-hashing-algorithm-is-best-for-uniqueness-and-speed
 */


#ifndef HASH_MURMUR

/*  FNV1a Resources Used:
 *    http://www.isthe.com/chongo/tech/comp/fnv/index.html#FNV-source
 *  
 *  NOTE: This Assumes the Compiler Your Using Defines unsigned int as 32 Bits
 *          and unsigned char as 8 Bits
 */
#define FNV_OFFSET_BASIS  0x811C9DC5u
#define FNV_PRIME         0x01000193u

/** @fn unsigned int HashString_hash( HashString* hash, unsigned char* key )
 *  
 *  @brief Computes Index of Given key for Given hash
 *
 *  Computes Index within HashString's Hash Table to But Entry with Given
 *  key.  If NULL is Provided Instead of a HashTable, Returns the Full
 *  Value of the FNV1A Hash
 *
 *  @param[in] hash HashString* to the Hash Table to Compute Index for (or NULL)
 *  @param[in] key unsigned char* to the String to Compute Hash of
 *
 *  @returns unsigned int, the Index within hash for key, or the Hash
 *  @returns  if hash == NULL
 */
unsigned int HashString_hash( HashString* hash, unsigned char* key )
{
  /** The Value of the FNV1a Hash of key */
  unsigned int hashValue = FNV_OFFSET_BASIS;

  if ( key == NULL )
    return 0;
  
  /** Compute the FNV1a Hash of key */
  for ( ; *key; ++key )
  {
    hashValue ^= *key;
    hashValue *= FNV_PRIME;
  }

  /** If Input HashString isn't a NULL Pointer,
   *   Adjust hashValue to Index for hash->table
   */
  if ( hash != NULL )
    return ( hashValue % hash->size );
  
  /** If Input HashString was NULL Pointer, Return Full Hash Value */
  return hashValue;
}

#else

/** Murmur 2 Resources Used:
 *    https://sites.google.com/site/murmurhash/
 *    https://sites.google.com/site/murmurhash/MurmurHash2.cpp?attredirects=0
 *
 *  All Credit for the Murmur2 Hash Algorithm+Constants Goes to Austin Appleby
 *
 *  NOTE: This Assumes your Compiler has 32-bit unsigned int
 *         and 8-bit unsigned char
 */

#include <string.h> /* strlen */

/** Murmur 2 Mix Constants that Work Well, Thanks to Austin Appleby */
#define MURMUR2_MIX1  0x5BD1E995u
#define MURMUR2_MIX2  24

/** Murmur 2 Random Seed, floor ( 2^32 * gamma )
 *    See Here:
 *      http://en.wikipedia.org/wiki/Euler-Mascheroni_constant
 */
#define MURMUR2_SEED  0x93C467E3u


/** @fn unsigned int HashString_hash( HashString* hash, unsigned char* key )
 *  
 *  @brief Computes Index of Given key for Given hash
 *
 *  Computes Index within HashString's Hash Table to But Entry with Given
 *  key.  If NULL is Provided Instead of a HashTable, Returns the Full
 *  Value of the Murmur2 Hash
 *
 *  @param[in] hash HashString* to the Hash Table to Compute Index for (or NULL)
 *  @param[in] key unsigned char* to the String to Compute Hash of
 *
 *  @returns unsigned int, the Index within hash for key, or the Hash
 *  @returns  if hash == NULL
 */
unsigned int HashString_hash( HashString* hash, unsigned char* key )
{
  /** Length of key in 8-bit Bytes */
  unsigned int len;
  /** Value of Hash */
  unsigned int hashValue;

  /** Don't Perform Hash if key is a NULL Pointer */
  if ( key == NULL )
    return 0;
  
  /** Set Initial Values */
  len = strlen( key );
  hashValue = MURMUR2_SEED ^ len; /**< Start off with Pseudo-Random Number */

  /** Hash 4 Bytes at a Time */
  while ( len >= 4 )
  {
    unsigned int tmp = *(unsigned int*)key;

    tmp *= MURMUR2_MIX1;
    tmp ^= tmp >> MURMUR2_MIX2;
    tmp *= MURMUR2_MIX1;

    hashValue *= MURMUR2_MIX1;
    hashValue ^= tmp;

    key += 4;
    len -= 4;
  }

  /** Handle Remaining Bytes ( if Len wasn't Divisible by 4 ) */
  /** Note there are no break Statements, eg. if len==3, all statements execute */
  /** Things aren't Really Mixed Well Here, so There's More Later */
  switch( len )
  {
    case 3: hashValue ^= key[2] << 16;
    case 2: hashValue ^= key[1] << 8;
    case 1: hashValue ^= *key;
            hashValue ^= MURMUR2_MIX1;
  };

  /** You can Never Overmix a Hash!  MOAR MIXIN' */
  /** Incase len wasn't Divisible by 4, Make Sure Everything is Mixed well */
  hashValue ^= hashValue >> 13;
  hashValue *= MURMUR2_MIX1;
  hashValue ^= hashValue >> 15;
  
  /** If hash isn't a NULL Pointer, Adjust hashValue for hash Table */
  if ( hash != NULL )
    return (hashValue % hash->size);

  /** hash Was NULL, Return Full Hash Value */
  return hashValue;
}

#endif




/** @fn HashStringEntry HashString_find( HashString* hash, char* key )
 *
 *  @brief  Finds Entry in Given HashString with Given Key, Returns Pointer
 *
 *  Finds Entry within Given HashString "hash" with Given Key "key" if any
 *  If it Exists, It Returns a HashStringEntry* to that Entry
 *  If it Doesn't Exist, It Returns NULL
 *
 *  @param[in] hash HashString* to the HashString to Search within
 *  @param[in] key  char* to a String Containing the Key to Search for
 *  
 *  @returns HashStringEntry* to the Entry if Found
 *  @returns NULL if Entry is not Found
 */
HashStringEntry* HashString_find( HashString* hash, char* key )
{
  HashStringEntry* entry;  /**< The Current Entry Being Parsed */
  unsigned int hashValue = HashString_hash( hash, key ); /**< The Result of the Hash Function */

  /** Search for Entry within HashString hash */
  for ( entry = hash->table[hashValue]; entry != NULL; entry = entry->next )
    if ( !strcmp( entry->key, key ) )
      return entry;

  /** Entry not Found, Return NULL */
  return (HashStringEntry*) NULL;
}




/** @fn int HashString_add( HashString* hash, char* key, char* value )
 *  @brief  Adds a Key, Value Pair to the HashString
 *
 *  Adds a Key, Value Pair to the HashString if the Key Doesn't Already Exist
 *  malloc's a New HashStringEntry and Makes new Copies of Key and Value Strings
 *
 *  @param hash HashString* to The HashString to Add to
 *  @param key    char* to the Key String to Add
 *  @param value  char* to the Value String to Add
 *
 *  @returns int the Status of the Addition
 *  @returns 0 - Success
 *  @returns 1 - Key Already Exists
 *  @returns 2 - malloc Failed
 *  @returns 3 - NULL Pointer Given
 */
int HashString_add( HashString* hash, char* key, void* value )
{
  HashStringEntry* oldEntry;  /**< The Exisiting Entry with the Same Key, NULL for None */
  HashStringEntry* newEntry;  /**< The New Entry to Insert into the HashString */
  unsigned int hashValue;    /**<  The Value of the HashString_hash() for this key, hash, newEntry Position in hash */
  
  /** Return Error if a NULL Pointer is Given */
  if ( ( hash == NULL ) || ( key == NULL ) || ( value == NULL ) )
    return 3;

  oldEntry = HashString_find( hash, key );
  
  if ( oldEntry != NULL )
  {
    /* Entry with Key key Already Exists in HashString hash */
    return 1;
  }

  /* Key is new, Create New Entry */
  newEntry = (HashStringEntry*) malloc(sizeof(HashStringEntry));

  if ( newEntry == NULL ) /* malloc() failed */
    return 2;
  
  /* Create new key String, Handle Errors */
  newEntry->key = (char*) malloc( sizeof(char)*(strlen(key)+1) );
  if ( newEntry->key == NULL )
  {
    /* key String malloc Failed */
    free( (void*) newEntry );
    return 2;
  }
  
  /* This has been changed, the hash map will store a void pointer to a value */
  /* Create new value String, Handle Errors */
  //newEntry->value = (char*) malloc( sizeof(char)*(strlen(value)+1) );
  //if ( newEntry->value == NULL )
  //{
  //  /* value String malloc Failed */
  //  free ( (void*) newEntry->key );
  //  free ( (void*) newEntry );
  //  return 2;
  //}

  newEntry->value = value;

  /* Get the Location for newEntry Within HashString hash */
  hashValue = HashString_hash( hash, key );

  /* Set Values for newEntry */
  strcpy( newEntry->key, key );
  //strcpy( newEntry->value, value );
  newEntry->next = hash->table[hashValue];  /* Set the First Entry with Same Hash Value as Second Entry */
  
  /* Insert newEntry into HashTable hash */
  hash->table[hashValue] = newEntry;

  /* Added New Entry Successfully */
  return 0;
}




/** @fn int HashString_remove( HashString* hash, char* key )
 *  @brief  Removes Entry with Given key from Given hash
 *
 *  Deletes HashStringEntry with key from HashString hash if one Exists
 *  Freeing all of the Memory it Consumed
 *
 *  @param hash HashString* to the HashString to Delete from
 *  @param key char* to String Containing Key to Search for, Delete Encapsulating Entry
 *
 *  @returns int the Status of the Deletion
 *  @returns 0 - Entry Found and Deleted
 *  @returns 1 - Entry not Found, No Change Made
 */
int HashString_remove( HashString* hash, char* key )
{
  HashStringEntry* prevEntry; /**< Entry Before Entry to Remove, Linked-List Pointer Must Change */
  HashStringEntry* currEntry;  /**< Current Entry to Look at */
  unsigned int hashValue = HashString_hash( hash, key );
  
  /* Get First Possible Entry with Given Key */
  currEntry = hash->table[ hashValue ];
    
  /* If there's no First Entry at this hashValue, There's No Entry with the Given Key */
  if ( currEntry == NULL )
    return 1; /* Entry Could not be Found, Nothing Removed */

  /* Check First Entry */
  if ( !strcmp(currEntry->key, key) )
  {
    /* currEntry is Entry to Delete, Set Next Entry as First, Free currEntry's Resources */
    hash->table[ hashValue ] = currEntry->next;
    
    free( currEntry->key );
    //free( currEntry->value ); User is reponsible for freeing entry
    free( currEntry );

    /* Entry Found and Removed */
    return 0;
  }
    
  /* Set currEntry to Next Entry */
  prevEntry = currEntry;
  currEntry = currEntry->next;

  while ( currEntry != NULL )
  {
    prevEntry = currEntry;
    currEntry = currEntry->next;
    
    /* Check if currEntry is the Entry to Delete */
    if ( !strcmp(currEntry->key, key) )
    {
      /* Remove currEntry from the Linked List and free its contents, it */
      prevEntry->next = currEntry->next;

      free( currEntry->key );
      //free( currEntry->value ); User is reponsible for freeing entry
      free( currEntry );

      /* Entry Found and Removed */
      return 0;
    }
  }

  /* Entry Could Not be Found, None Removed */
  return 1;
}




/** @fn void HashString_firstEntry( HashString* hash, HashStringEntry** entry, int* i )
 *
 *  @brief  Finds First Entry in HashString
 *
 *  Finds First Entry in HashString hash and Sets entry and i to Point to it and
 *  its table Index, Respectively
 *
 *  @param[in] hash HashString* to the HashString to Find First Entry in
 *  @param[out] entry HashStringEntry** to Address Holding Entry
 *  @param[out] i int* to the Address Containing the Table Index
 */
void HashString_firstEntry( HashString* hash, HashStringEntry** entry, int* i )
{
  /* Move Through All First Entries in Each Linked List */
  for ( *i = 0; ( *i < hash->size ) && ( *entry == NULL ); ++(*i) )
    *entry = hash->table[*i];
}




/** @fn void HashString_nextEntry( HashString* hash, HashStringEntry** entry, int* i )
 *
 *  @brief Finds Next Entry in HashString
 *
 *  Finds Next Entry in HashString and Sets entry and i to Point to it,
 *   Helper Function for HASH_STRING_ITER
 *
 *  @param[in] hash HashString* to the HashString to Search Through
 *  @param entry HashStringEntry** to Var Holding Current Entry, Where to Store Next
 *  @param i int* to Var Holding Current Table Index, Where to Store Next
 *
 */
void HashString_nextEntry( HashString* hash, HashStringEntry** entry, int* i )
{
  
  /* If Given a NULL Entry, Exit the HASH_STRING_ITER Loop */
  if ( *entry == NULL )
  {
    /* Given NULL Pointer, Exit HASH_STRING_ITER Loop */
    *i = hash->size;
    return;
  }

  /* Move to Next Entry */
  *entry = (*entry)->next;

  /* If the New Entry isn't NULL, the Next Entry was found, Return */
  if ( *entry != NULL )
    return;

  /* Loop Through the HashString Until the Next Entry is Found */
  for( ; (*i < hash->size) && (*entry == NULL); ++(*i) )
      *entry = hash->table[*i];
}