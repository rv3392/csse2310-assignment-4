#ifndef LIST_H
#define LIST_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <semaphore.h>

/* This semaphore will only be used to regulate threads and not processes */
#define SEMAPHORE_THREAD_ONLY 0
/* The number of threads that can concurrently access this List */
#define SEMAPHORE_MAX_CONCURRENT 1

typedef struct List List;
typedef void* ListItem;

/**
 * A ListItemStr takes any ListItem and converts it to a string.
 * (Note: This function follows the same format as snprintf so that snprintf
 *      can be used with List with a simple wrapper)
 * Parameters:
 *  - strBuffer - the buffer to write the string to
 *  - bufferSize - the size of the buffer being written to
 *  - format - the format string to use
 *  - conversionItem - the arguments to write to the string
 * 
 * Returns:
 *  - The number of characters written to the provided buffer.
 */
typedef int (*ListItemStr)(char* strBuffer, size_t bufferSize,
        ListItem conversionItem);

/**
 * A ListItemComparator takes two ListItems and compares them.
 * (Note: This function follows the same format as strcmp so that strcmp can
 *      be used as a comparator for List with a simple wrapper)
 * Returns:
 *  - 0 -> if the two ListItems are equal
 *  - Negative Value -> if item1 < item2
 *  - Positive Value -> if item1 > item2
 */
typedef int (*ListItemCmp)(const void* item1, const void* item2);

/**
 * A generic thread-safe List type.
 * For use in mapper2310, control2310 and roc2310.
 * 
 * Members:
 *  - content -> contains a pointer to an array of the items in the list.
 *  - length -> number of items in the list
 *  - itemSize -> the size of each item in the list in bytes
 *  - printer -> a pointer to the function used to convert the item to a
 *      string version of it.
 *  - listAccessSemaphore -> the semaphore used for regulating access to the
 *      list.
 */ 
struct List {
    ListItem* content;
    int length;
    size_t itemSize;
    ListItemStr toString;
    ListItemCmp compare;
    sem_t* listAccessSemaphore;
};

/* The error codes for List-related functions */
enum ListError {
    LIST_OK,
    LIST_NOT_OK
};
typedef enum ListError ListError;

/**
 * Creates a List in the provided list struct using the provided parameters
 * 
 * Parameters:
 *  - list -> the buffer to write the List to
 *  - itemSize -> the size of an item within the list in bytes
 *  - toString -> the function used to convert the type stored in this list
 *      to a string. This should be a wrapper around snprintf.
 *  - compare -> the function used to compare two values of the type stored
 *      in this list.
 * 
 * Returns:
 *  - LIST_OK -> if the list is successfully created
 */ 
ListError create_list(List* list, size_t itemSize,
        ListItemStr toString, ListItemCmp compare);

/**
 * Get an item at a specific index in this List and write it to the memory
 * at itemBuffer. If the index does not exist then the buffer is unchanged
 * and LIST_NOT_OK is returned.
 * 
 * Parameters:
 *  - list -> the List being accessed
 *  - index -> the index to read
 *  - itemBuffer -> the buffer to write the item at the index to
 * 
 * Returns:
 *  - LIST_OK -> if the item is successfully written to the provided buffer
 *  - LIST_NOT_OK -> if there was an issue copying the item to the provided
 *      buffer or if the index does not exist.
 */
ListError get_list_item(List* list, int index, ListItem* itemBuffer);

/**
 * Gets the entire list's string representation using the provided 
 * list->compare function. Each item in the list is delimitted by a newline
 * (\n). There is no newline after the final item in the list.
 * 
 * Parameters:
 *  - list -> the list to convert to a string
 *  - buffer -> a pointer to the buffer to write the converted string to
 *  - capacity -> a pointer to the size of "buffer" in bytes
 * 
 * Returns:
 *  - LIST_OK -> if the list was successfully converted to a string.
 */
ListError get_list_as_str(List* list, char** buffer, size_t* capacity);

/**
 * Adds an item to the end of the list provided.
 * 
 * Parameters:
 *  - list -> the list to add to
 *  - item -> the item to be added. This should be a pointer to the item
 *      in heap memory and the item should be of the same type as all other
 *      items in the list (or at least the same size in bytes as specified by 
 *      list->itemSize).
 * 
 * Returns:
 *  - LIST_OK -> if the item is successfully added to the list
 *  - LIST_NOT_OK -> if there was an issue reallocing memory for the list
 *      content or if the item provided is not the same size as specified in
 *      list->itemSize.
 */
ListError add_list_item(List* list, ListItem item);

/**
 * Searches the list for the provided searchKey using a simple linear scan. 
 * For comparisons the specified list->compare function is used.
 * 
 * Parameters:
 *  - list -> the list to be searched
 *  - searchKey -> a search term that should be of the same type as this list
 *      stores. I guess if you write list->compare to do so, it could be
 *      possible to store other stuff in here too.
 *  - itemBuffer -> a buffer to store a pointer to the memory containing
 *      any list item matching the searchKey.
 * 
 * Returns:
 *  - LIST_OK -> If an item is found and the pointer successfully copied to
 *      the provided memory.
 *  - LIST_NOT_OK -> If there was an issue copying the item pointer to the
 *      buffer. In this case the search needs to find a result as well.
 */
ListError search_list(List* list, ListItem searchKey, ListItem* itemBuffer);

/**
 * Sorts the content of the list using the built-in qsort function. The list 
 * is sorted into ascending order and the order depends on list->compare which
 * is used to compare values within the list.
 * 
 * Parameters:
 *  - list -> the list to sort
 * 
 * Returns:
 *  - LIST_OK -> if the list is successfully sorted.
 */
ListError sort_list(List* list);

#endif