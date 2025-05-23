/******************************************************************************
 *
 * Component: OGDI Core C API
 * Purpose: Implementation of in-memory hash tables for ecs and ecs-based
 *	    applications.
 * 
 ******************************************************************************
 * Copyright (c) 1991-1993 The Regents of the University of California.
 * Copyright (c) 1994 Sun Microsystems, Inc.
 *
 * See the file "license.terms" for information on usage and redistribution
 * of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 ******************************************************************************
 *
 * $Log: ecshash.c,v $
 * Revision 1.1  2005/07/20 19:26:30  syarrow
 * New version of GDAL and OGDI.
 *
 * Revision 1.3  2001/04/09 15:04:34  warmerda
 * applied new source headers
 *
 */

#include "ecs.h"

ECS_CVSID("$Id: ecshash.c,v 1.1 2005/07/20 19:26:30 syarrow Exp $");

/*
 * When there are this many entries per bucket, on average, rebuild
 * the hash table to make it larger.
 */

#define REBUILD_MULTIPLIER	3


/*
 * The following macro takes a preliminary integer hash value and
 * produces an index into a hash tables bucket list.  The idea is
 * to make it so that preliminary values that are arbitrarily similar
 * will end up in different buckets.  The hash function was taken
 * from a random-number generator.
 */

#define RANDOM_INDEX(tablePtr, i) (((((long) (i))*1103515245) >> (tablePtr)->downShift) & (tablePtr)->mask)

/*
 * Procedure prototypes for static procedures in this file:
 */

static ecs_HashEntry *ArrayFind _ANSI_ARGS_((ecs_HashTable *tablePtr,
					     char *key));
static ecs_HashEntry*	ArrayCreate _ANSI_ARGS_((ecs_HashTable *tablePtr,
						 char *key, int *newPtr));
static ecs_HashEntry*	BogusFind _ANSI_ARGS_((ecs_HashTable *tablePtr,
			    char *key));
static ecs_HashEntry*	BogusCreate _ANSI_ARGS_((ecs_HashTable *tablePtr,
			    char *key, int *newPtr));
static unsigned int	HashString _ANSI_ARGS_((char *string));
static void		RebuildTable _ANSI_ARGS_((ecs_HashTable *tablePtr));
static ecs_HashEntry*	StringFind _ANSI_ARGS_((ecs_HashTable *tablePtr,
			    char *key));
static ecs_HashEntry*	StringCreate _ANSI_ARGS_((ecs_HashTable *tablePtr,
			    char *key, int *newPtr));
static ecs_HashEntry*	OneWordFind _ANSI_ARGS_((ecs_HashTable *tablePtr,
			    char *key));
static ecs_HashEntry*	OneWordCreate _ANSI_ARGS_((ecs_HashTable *tablePtr,
			    char *key, int *newPtr));

/*
 *----------------------------------------------------------------------
 *
 * ecs_InitHashTable --
 *
 *	Given storage for a hash table, set up the fields to prepare
 *	the hash table for use.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	TablePtr is now ready to be passed to ecs_FindHashEntry and
 *	ecs_CreateHashEntry.
 *
 *----------------------------------------------------------------------
 */

void
ecs_InitHashTable(tablePtr, keyType)
    register ecs_HashTable *tablePtr;	/* Pointer to table record, which
					 * is supplied by the caller. */
    int keyType;			/* Type of keys to use in table:
					 * ECS_STRING_KEYS, ECS_ONE_WORD_KEYS,
					 * or an integer >= 2. */
{
    tablePtr->buckets = tablePtr->staticBuckets;
    tablePtr->staticBuckets[0] = tablePtr->staticBuckets[1] = 0;
    tablePtr->staticBuckets[2] = tablePtr->staticBuckets[3] = 0;
    tablePtr->numBuckets = ECS_SMALL_HASH_TABLE;
    tablePtr->numEntries = 0;
    tablePtr->rebuildSize = ECS_SMALL_HASH_TABLE*REBUILD_MULTIPLIER;
    tablePtr->downShift = 28;
    tablePtr->mask = 3;
    tablePtr->keyType = keyType;
    if (keyType == ECS_STRING_KEYS) {
	tablePtr->findProc = StringFind;
	tablePtr->createProc = StringCreate;
    } else if (keyType == ECS_ONE_WORD_KEYS) {
	tablePtr->findProc = OneWordFind;
	tablePtr->createProc = OneWordCreate;
    } else {
	tablePtr->findProc = ArrayFind;
	tablePtr->createProc = ArrayCreate;
    };
}

/*
 *----------------------------------------------------------------------
 *
 * ECS_DeleteHashEntry --
 *
 *	Remove a single entry from a hash table.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	The entry given by entryPtr is deleted from its table and
 *	should never again be used by the caller.  It is up to the
 *	caller to free the clientData field of the entry, if that
 *	is relevant.
 *
 *----------------------------------------------------------------------
 */

void
ecs_DeleteHashEntry(entryPtr)
    ecs_HashEntry *entryPtr;
{
    register ecs_HashEntry *prevPtr;

    if (*entryPtr->bucketPtr == entryPtr) {
	*entryPtr->bucketPtr = entryPtr->nextPtr;
    } else {
	for (prevPtr = *entryPtr->bucketPtr; ; prevPtr = prevPtr->nextPtr) {
	    if (prevPtr == NULL) {
	    }
	    if (prevPtr->nextPtr == entryPtr) {
		prevPtr->nextPtr = entryPtr->nextPtr;
		break;
	    }
	}
    }
    entryPtr->tablePtr->numEntries--;
    free((char *) entryPtr);
}

/*
 *----------------------------------------------------------------------
 *
 * ecs_DeleteHashTable --
 *
 *	Free up everything associated with a hash table except for
 *	the record for the table itself.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	The hash table is no longer useable.
 *
 *----------------------------------------------------------------------
 */

void
ecs_DeleteHashTable(tablePtr)
    register ecs_HashTable *tablePtr;		/* Table to delete. */
{
    register ecs_HashEntry *hPtr, *nextPtr;
    int i;

    /*
     * Free up all the entries in the table.
     */

    for (i = 0; i < tablePtr->numBuckets; i++) {
	hPtr = tablePtr->buckets[i];
	while (hPtr != NULL) {
	    nextPtr = hPtr->nextPtr;
	    free((char *) hPtr);
	    hPtr = nextPtr;
	}
    }

    /*
     * Free up the bucket array, if it was dynamically allocated.
     */

    if (tablePtr->buckets != tablePtr->staticBuckets) {
	free((char *) tablePtr->buckets);
    }

    /*
     * Arrange for panics if the table is used again without
     * re-initialization.
     */

    tablePtr->findProc = BogusFind;
    tablePtr->createProc = BogusCreate;
}

/*
 *----------------------------------------------------------------------
 *
 * ecs_FirstHashEntry --
 *
 *	Locate the first entry in a hash table and set up a record
 *	that can be used to step through all the remaining entries
 *	of the table.
 *
 * Results:
 *	The return value is a pointer to the first entry in tablePtr,
 *	or NULL if tablePtr has no entries in it.  The memory at
 *	*searchPtr is initialized so that subsequent calls to
 *	ecs_NextHashEntry will return all of the entries in the table,
 *	one at a time.
 *
 * Side effects:
 *	None.
 *
 *----------------------------------------------------------------------
 */

ecs_HashEntry *
ecs_FirstHashEntry(tablePtr, searchPtr)
    ecs_HashTable *tablePtr;		/* Table to search. */
    ecs_HashSearch *searchPtr;		/* Place to store information about
					 * progress through the table. */
{
    searchPtr->tablePtr = tablePtr;
    searchPtr->nextIndex = 0;
    searchPtr->nextEntryPtr = NULL;
    return ecs_NextHashEntry(searchPtr);
}

/*
 *----------------------------------------------------------------------
 *
 * ecs_NextHashEntry --
 *
 *	Once a hash table enumeration has been initiated by calling
 *	ecs_FirstHashEntry, this procedure may be called to return
 *	successive elements of the table.
 *
 * Results:
 *	The return value is the next entry in the hash table being
 *	enumerated, or NULL if the end of the table is reached.
 *
 * Side effects:
 *	None.
 *
 *----------------------------------------------------------------------
 */

ecs_HashEntry *
ecs_NextHashEntry(searchPtr)
    ecs_HashSearch *searchPtr;	/* Place to store information about
				 * progress through the table.  Must
				 * have been initialized by calling
				 * ecs_FirstHashEntry. */
{
    ecs_HashEntry *hPtr;

    while (searchPtr->nextEntryPtr == NULL) {
	if (searchPtr->nextIndex >= searchPtr->tablePtr->numBuckets) {
	    return NULL;
	}
	searchPtr->nextEntryPtr =
		searchPtr->tablePtr->buckets[searchPtr->nextIndex];
	searchPtr->nextIndex++;
    }
    hPtr = searchPtr->nextEntryPtr;
    searchPtr->nextEntryPtr = hPtr->nextPtr;
    return hPtr;
}

/*
 *----------------------------------------------------------------------
 *
 * ecs_HashStats --
 *
 *	Return statistics describing the layout of the hash table
 *	in its hash buckets.
 *
 * Results:
 *	The return value is a malloc-ed string containing information
 *	about tablePtr.  It is the caller's responsibility to free
 *	this string.
 *
 * Side effects:
 *	None.
 *
 *----------------------------------------------------------------------
 */

char *
ecs_HashStats(tablePtr)
    ecs_HashTable *tablePtr;		/* Table for which to produce stats. */
{
#define NUM_COUNTERS 10
    int count[NUM_COUNTERS], overflow, i, j;
    double average, tmp;
    register ecs_HashEntry *hPtr;
    char *result, *p;

    /*
     * Compute a histogram of bucket usage.
     */

    for (i = 0; i < NUM_COUNTERS; i++) {
	count[i] = 0;
    }
    overflow = 0;
    average = 0.0;
    for (i = 0; i < tablePtr->numBuckets; i++) {
	j = 0;
	for (hPtr = tablePtr->buckets[i]; hPtr != NULL; hPtr = hPtr->nextPtr) {
	    j++;
	}
	if (j < NUM_COUNTERS) {
	    count[j]++;
	} else {
	    overflow++;
	}
	tmp = j;
	average += (tmp+1.0)*(tmp/tablePtr->numEntries)/2.0;
    }

    /*
     * Print out the histogram and a few other pieces of information.
     */

    result = (char *) malloc((unsigned) ((NUM_COUNTERS*60) + 300));
    sprintf(result, "%d entries in table, %d buckets\n",
	    tablePtr->numEntries, tablePtr->numBuckets);
    p = result + strlen(result);
    for (i = 0; i < NUM_COUNTERS; i++) {
	sprintf(p, "number of buckets with %d entries: %d\n",
		i, count[i]);
	p += strlen(p);
    }
    sprintf(p, "number of buckets with %d or more entries: %d\n",
	    NUM_COUNTERS, overflow);
    p += strlen(p);
    sprintf(p, "average search distance for entry: %.1f", average);
    return result;
}

/*
 *----------------------------------------------------------------------
 *
 * HashString --
 *
 *	Compute a one-word summary of a text string, which can be
 *	used to generate a hash index.
 *
 * Results:
 *	The return value is a one-word summary of the information in
 *	string.
 *
 * Side effects:
 *	None.
 *
 *----------------------------------------------------------------------
 */

static unsigned int
HashString(string)
    register char *string;	/* String from which to compute hash value. */
{
    register unsigned int result;
    register int c;

    /*
     * I tried a zillion different hash functions and asked many other
     * people for advice.  Many people had their own favorite functions,
     * all different, but no-one had much idea why they were good ones.
     * I chose the one below (multiply by 9 and add new character)
     * because of the following reasons:
     *
     * 1. Multiplying by 10 is perfect for keys that are decimal strings,
     *    and multiplying by 9 is just about as good.
     * 2. Times-9 is (shift-left-3) plus (old).  This means that each
     *    character's bits hang around in the low-order bits of the
     *    hash value for ever, plus they spread fairly rapidly up to
     *    the high-order bits to fill out the hash value.  This seems
     *    works well both for decimal and non-decimal strings.
     */

    result = 0;
    while (1) {
	c = *string;
	string++;
	if (c == 0) {
	    break;
	}
	result += (result<<3) + c;
    }
    return result;
}

/*
 *----------------------------------------------------------------------
 *
 * StringFind --
 *
 *	Given a hash table with string keys, and a string key, find
 *	the entry with a matching key.
 *
 * Results:
 *	The return value is a token for the matching entry in the
 *	hash table, or NULL if there was no matching entry.
 *
 * Side effects:
 *	None.
 *
 *----------------------------------------------------------------------
 */

static ecs_HashEntry *
StringFind(tablePtr, key)
    ecs_HashTable *tablePtr;	/* Table in which to lookup entry. */
    char *key;			/* Key to use to find matching entry. */
{
    register ecs_HashEntry *hPtr;
    register char *p1, *p2;
    int index;

    index = HashString(key) & tablePtr->mask;

    /*
     * Search all of the entries in the appropriate bucket.
     */

    for (hPtr = tablePtr->buckets[index]; hPtr != NULL;
	    hPtr = hPtr->nextPtr) {
	for (p1 = key, p2 = hPtr->key.string; ; p1++, p2++) {
	    if (*p1 != *p2) {
		break;
	    }
	    if (*p1 == '\0') {
		return hPtr;
	    }
	}
    }
    return NULL;
}

/*
 *----------------------------------------------------------------------
 *
 * StringCreate --
 *
 *	Given a hash table with string keys, and a string key, find
 *	the entry with a matching key.  If there is no matching entry,
 *	then create a new entry that does match.
 *
 * Results:
 *	The return value is a pointer to the matching entry.  If this
 *	is a newly-created entry, then *newPtr will be set to a non-zero
 *	value;  otherwise *newPtr will be set to 0.  If this is a new
 *	entry the value stored in the entry will initially be 0.
 *
 * Side effects:
 *	A new entry may be added to the hash table.
 *
 *----------------------------------------------------------------------
 */

static ecs_HashEntry *
StringCreate(tablePtr, key, newPtr)
    ecs_HashTable *tablePtr;	/* Table in which to lookup entry. */
    char *key;			/* Key to use to find or create matching
				 * entry. */
    int *newPtr;		/* Store info here telling whether a new
				 * entry was created. */
{
    register ecs_HashEntry *hPtr;
    register char *p1, *p2;
    int index;

    index = HashString(key) & tablePtr->mask;

    /*
     * Search all of the entries in this bucket.
     */

    for (hPtr = tablePtr->buckets[index]; hPtr != NULL;
	    hPtr = hPtr->nextPtr) {
	for (p1 = key, p2 = hPtr->key.string; ; p1++, p2++) {
	    if (*p1 != *p2) {
		break;
	    }
	    if (*p1 == '\0') {
		*newPtr = 0;
		return hPtr;
	    }
	}
    }

    /*
     * Entry not found.  Add a new one to the bucket.
     */

    *newPtr = 1;
    hPtr = (ecs_HashEntry *) malloc((unsigned)
	    (sizeof(ecs_HashEntry) + strlen(key) - (sizeof(hPtr->key) -1)));
    hPtr->tablePtr = tablePtr;
    hPtr->bucketPtr = &(tablePtr->buckets[index]);
    hPtr->nextPtr = *hPtr->bucketPtr;
    hPtr->clientData = 0;
    strcpy(hPtr->key.string, key);
    *hPtr->bucketPtr = hPtr;
    tablePtr->numEntries++;

    /*
     * If the table has exceeded a decent size, rebuild it with many
     * more buckets.
     */

    if (tablePtr->numEntries >= tablePtr->rebuildSize) {
	RebuildTable(tablePtr);
    }
    return hPtr;
}

/*
 *----------------------------------------------------------------------
 *
 * OneWordFind --
 *
 *	Given a hash table with one-word keys, and a one-word key, find
 *	the entry with a matching key.
 *
 * Results:
 *	The return value is a token for the matching entry in the
 *	hash table, or NULL if there was no matching entry.
 *
 * Side effects:
 *	None.
 *
 *----------------------------------------------------------------------
 */

static ecs_HashEntry *
OneWordFind(tablePtr, key)
    ecs_HashTable *tablePtr;	/* Table in which to lookup entry. */
    register char *key;		/* Key to use to find matching entry. */
{
    register ecs_HashEntry *hPtr;
    int index;

    index = RANDOM_INDEX(tablePtr, key);

    /*
     * Search all of the entries in the appropriate bucket.
     */

    for (hPtr = tablePtr->buckets[index]; hPtr != NULL;
	    hPtr = hPtr->nextPtr) {
	if (hPtr->key.oneWordValue == key) {
	    return hPtr;
	}
    }
    return NULL;
}

/*
 *----------------------------------------------------------------------
 *
 * OneWordCreate --
 *
 *	Given a hash table with one-word keys, and a one-word key, find
 *	the entry with a matching key.  If there is no matching entry,
 *	then create a new entry that does match.
 *
 * Results:
 *	The return value is a pointer to the matching entry.  If this
 *	is a newly-created entry, then *newPtr will be set to a non-zero
 *	value;  otherwise *newPtr will be set to 0.  If this is a new
 *	entry the value stored in the entry will initially be 0.
 *
 * Side effects:
 *	A new entry may be added to the hash table.
 *
 *----------------------------------------------------------------------
 */

static ecs_HashEntry *
OneWordCreate(tablePtr, key, newPtr)
    ecs_HashTable *tablePtr;	/* Table in which to lookup entry. */
    register char *key;		/* Key to use to find or create matching
				 * entry. */
    int *newPtr;		/* Store info here telling whether a new
				 * entry was created. */
{
    register ecs_HashEntry *hPtr;
    int index;

    index = RANDOM_INDEX(tablePtr, key);

    /*
     * Search all of the entries in this bucket.
     */

    for (hPtr = tablePtr->buckets[index]; hPtr != NULL;
	    hPtr = hPtr->nextPtr) {
	if (hPtr->key.oneWordValue == key) {
	    *newPtr = 0;
	    return hPtr;
	}
    }

    /*
     * Entry not found.  Add a new one to the bucket.
     */

    *newPtr = 1;
    hPtr = (ecs_HashEntry *) malloc(sizeof(ecs_HashEntry));
    hPtr->tablePtr = tablePtr;
    hPtr->bucketPtr = &(tablePtr->buckets[index]);
    hPtr->nextPtr = *hPtr->bucketPtr;
    hPtr->clientData = 0;
    hPtr->key.oneWordValue = key;
    *hPtr->bucketPtr = hPtr;
    tablePtr->numEntries++;

    /*
     * If the table has exceeded a decent size, rebuild it with many
     * more buckets.
     */

    if (tablePtr->numEntries >= tablePtr->rebuildSize) {
	RebuildTable(tablePtr);
    }
    return hPtr;
}

/*
 *----------------------------------------------------------------------
 *
 * ArrayFind --
 *
 *	Given a hash table with array-of-int keys, and a key, find
 *	the entry with a matching key.
 *
 * Results:
 *	The return value is a token for the matching entry in the
 *	hash table, or NULL if there was no matching entry.
 *
 * Side effects:
 *	None.
 *
 *----------------------------------------------------------------------
 */

static ecs_HashEntry *
ArrayFind(tablePtr, key)
    ecs_HashTable *tablePtr;	/* Table in which to lookup entry. */
    char *key;			/* Key to use to find matching entry. */
{
    register ecs_HashEntry *hPtr;
    int *arrayPtr = (int *) key;
    register int *iPtr1, *iPtr2;
    int index, count;

    for (index = 0, count = tablePtr->keyType, iPtr1 = arrayPtr;
	    count > 0; count--, iPtr1++) {
	index += *iPtr1;
    }
    index = RANDOM_INDEX(tablePtr, index);

    /*
     * Search all of the entries in the appropriate bucket.
     */

    for (hPtr = tablePtr->buckets[index]; hPtr != NULL;
	    hPtr = hPtr->nextPtr) {
	for (iPtr1 = arrayPtr, iPtr2 = hPtr->key.words,
		count = tablePtr->keyType; ; count--, iPtr1++, iPtr2++) {
	    if (count == 0) {
		return hPtr;
	    }
	    if (*iPtr1 != *iPtr2) {
		break;
	    }
	}
    }
    return NULL;
}

/*
 *----------------------------------------------------------------------
 *
 * ArrayCreate --
 *
 *	Given a hash table with one-word keys, and a one-word key, find
 *	the entry with a matching key.  If there is no matching entry,
 *	then create a new entry that does match.
 *
 * Results:
 *	The return value is a pointer to the matching entry.  If this
 *	is a newly-created entry, then *newPtr will be set to a non-zero
 *	value;  otherwise *newPtr will be set to 0.  If this is a new
 *	entry the value stored in the entry will initially be 0.
 *
 * Side effects:
 *	A new entry may be added to the hash table.
 *
 *----------------------------------------------------------------------
 */

static ecs_HashEntry *
ArrayCreate(tablePtr, key, newPtr)
    ecs_HashTable *tablePtr;	/* Table in which to lookup entry. */
    register char *key;		/* Key to use to find or create matching
				 * entry. */
    int *newPtr;		/* Store info here telling whether a new
				 * entry was created. */
{
    register ecs_HashEntry *hPtr;
    int *arrayPtr = (int *) key;
    register int *iPtr1, *iPtr2;
    int index, count;

    for (index = 0, count = tablePtr->keyType, iPtr1 = arrayPtr;
	    count > 0; count--, iPtr1++) {
	index += *iPtr1;
    }
    index = RANDOM_INDEX(tablePtr, index);

    /*
     * Search all of the entries in the appropriate bucket.
     */

    for (hPtr = tablePtr->buckets[index]; hPtr != NULL;
	    hPtr = hPtr->nextPtr) {
	for (iPtr1 = arrayPtr, iPtr2 = hPtr->key.words,
		count = tablePtr->keyType; ; count--, iPtr1++, iPtr2++) {
	    if (count == 0) {
		*newPtr = 0;
		return hPtr;
	    }
	    if (*iPtr1 != *iPtr2) {
		break;
	    }
	}
    }

    /*
     * Entry not found.  Add a new one to the bucket.
     */

    *newPtr = 1;
    hPtr = (ecs_HashEntry *) malloc((unsigned) (sizeof(ecs_HashEntry)
	    + (tablePtr->keyType*sizeof(int)) - 4));
    hPtr->tablePtr = tablePtr;
    hPtr->bucketPtr = &(tablePtr->buckets[index]);
    hPtr->nextPtr = *hPtr->bucketPtr;
    hPtr->clientData = 0;
    for (iPtr1 = arrayPtr, iPtr2 = hPtr->key.words, count = tablePtr->keyType;
	    count > 0; count--, iPtr1++, iPtr2++) {
	*iPtr2 = *iPtr1;
    }
    *hPtr->bucketPtr = hPtr;
    tablePtr->numEntries++;

    /*
     * If the table has exceeded a decent size, rebuild it with many
     * more buckets.
     */

    if (tablePtr->numEntries >= tablePtr->rebuildSize) {
	RebuildTable(tablePtr);
    }
    return hPtr;
}

/*
 *----------------------------------------------------------------------
 *
 * BogusFind --
 *
 *	This procedure is invoked when an ecs_FindHashEntry is called
 *	on a table that has been deleted.
 *
 * Results:
 *	If panic returns (which it shouldn't) this procedure returns
 *	NULL.
 *
 * Side effects:
 *	Generates a panic.
 *
 *----------------------------------------------------------------------
 */

	/* ARGSUSED */
static ecs_HashEntry *
BogusFind(tablePtr, key)
    ecs_HashTable *tablePtr;	/* Table in which to lookup entry. */
    char *key;			/* Key to use to find matching entry. */
{
    (void) tablePtr;
    (void) key;

    return NULL;
}

/*
 *----------------------------------------------------------------------
 *
 * BogusCreate --
 *
 *	This procedure is invoked when an ecs_CreateHashEntry is called
 *	on a table that has been deleted.
 *
 * Results:
 *	If panic returns (which it shouldn't) this procedure returns
 *	NULL.
 *
 * Side effects:
 *	Generates a panic.
 *
 *----------------------------------------------------------------------
 */

	/* ARGSUSED */
static ecs_HashEntry *
BogusCreate(tablePtr, key, newPtr)
    ecs_HashTable *tablePtr;	/* Table in which to lookup entry. */
    char *key;			/* Key to use to find or create matching
				 * entry. */
    int *newPtr;		/* Store info here telling whether a new
				 * entry was created. */
{
    (void) tablePtr;
    (void) key;
    (void) newPtr;

    return NULL;
}

/*
 *----------------------------------------------------------------------
 *
 * RebuildTable --
 *
 *	This procedure is invoked when the ratio of entries to hash
 *	buckets becomes too large.  It creates a new table with a
 *	larger bucket array and moves all of the entries into the
 *	new table.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	Memory gets reallocated and entries get re-hashed to new
 *	buckets.
 *
 *----------------------------------------------------------------------
 */

static void
RebuildTable(tablePtr)
    register ecs_HashTable *tablePtr;	/* Table to enlarge. */
{
    int oldSize, count, index;
    ecs_HashEntry **oldBuckets;
    register ecs_HashEntry **oldChainPtr, **newChainPtr;
    register ecs_HashEntry *hPtr;

    oldSize = tablePtr->numBuckets;
    oldBuckets = tablePtr->buckets;

    /*
     * Allocate and initialize the new bucket array, and set up
     * hashing constants for new array size.
     */

    tablePtr->numBuckets *= 4;
    tablePtr->buckets = (ecs_HashEntry **) malloc((unsigned)
	    (tablePtr->numBuckets * sizeof(ecs_HashEntry *)));
    for (count = tablePtr->numBuckets, newChainPtr = tablePtr->buckets;
	    count > 0; count--, newChainPtr++) {
	*newChainPtr = NULL;
    }
    tablePtr->rebuildSize *= 4;
    tablePtr->downShift -= 2;
    tablePtr->mask = (tablePtr->mask << 2) + 3;

    /*
     * Rehash all of the existing entries into the new bucket array.
     */

    for (oldChainPtr = oldBuckets; oldSize > 0; oldSize--, oldChainPtr++) {
	for (hPtr = *oldChainPtr; hPtr != NULL; hPtr = *oldChainPtr) {
	    *oldChainPtr = hPtr->nextPtr;
	    if (tablePtr->keyType == ECS_STRING_KEYS) {
		index = HashString(hPtr->key.string) & tablePtr->mask;
	    } else if (tablePtr->keyType == ECS_ONE_WORD_KEYS) {
		index = RANDOM_INDEX(tablePtr, hPtr->key.oneWordValue);
	    } else {
		register int *iPtr;
		int count;

		for (index = 0, count = tablePtr->keyType,
			iPtr = hPtr->key.words; count > 0; count--, iPtr++) {
		    index += *iPtr;
		}
		index = RANDOM_INDEX(tablePtr, index);
	    }
	    hPtr->bucketPtr = &(tablePtr->buckets[index]);
	    hPtr->nextPtr = *hPtr->bucketPtr;
	    *hPtr->bucketPtr = hPtr;
	}
    }

    /*
     * Free up the old bucket array, if it was dynamically allocated.
     */

    if (oldBuckets != tablePtr->staticBuckets) {
	free((char *) oldBuckets);
    }
}
