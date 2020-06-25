#include "list.h"

/* See list.h */
ListError create_list(List* list, size_t itemSize, 
        ListItemStr toString, ListItemCmp compare) {
    list->content = calloc(0, itemSize);
    list->length = 0;
    list->itemSize = itemSize;
    list->toString = toString;
    list->compare = compare;

    list->listAccessSemaphore = calloc(1, sizeof(sem_t));
    sem_init(list->listAccessSemaphore,
            SEMAPHORE_THREAD_ONLY, SEMAPHORE_MAX_CONCURRENT);

    return LIST_OK;
}

/* See list.h */
ListError add_list_item(List* list, ListItem item) {
    sem_wait(list->listAccessSemaphore);

    // If the item being added is too big or too small (i.e. the incorrect
    // item).
    if (sizeof(item) != list->itemSize) {
        sem_post(list->listAccessSemaphore);
        return LIST_NOT_OK;
    }

    ListItem* newContent = realloc(
            list->content, (list->length + 1) * list->itemSize);
    if (newContent == NULL) {
        sem_post(list->listAccessSemaphore);
        return LIST_NOT_OK;
    }

    list->content = newContent;
    list->content[list->length] = item;
    list->length++;

    sem_post(list->listAccessSemaphore);
    return LIST_OK;
}

/* See list.h */
ListError get_list_item(List* list, int index, ListItem* buffer) {
    sem_wait(list->listAccessSemaphore);
    if (index > list->length - 1) {
        sem_post(list->listAccessSemaphore);
        return LIST_NOT_OK;
    }
    if (memcpy(buffer, &(list->content[index]), list->itemSize) == NULL) {
        sem_post(list->listAccessSemaphore);
        return LIST_NOT_OK;
    }

    sem_post(list->listAccessSemaphore);
    return LIST_OK;
}

/* See list.h */
ListError search_list(List* list, ListItem searchKey, ListItem* buffer) {
    sem_wait(list->listAccessSemaphore);

    for (int i = 0; i < list->length; i++) {
        if (list->compare(searchKey, &list->content[i]) == 0) {
            sem_post(list->listAccessSemaphore);
            if (get_list_item(list, i, buffer) != LIST_OK) {
                return LIST_NOT_OK;
            }

            return LIST_OK;
        }
    }

    sem_post(list->listAccessSemaphore);
    return LIST_OK;
}

/* See list.h */
ListError sort_list(List* list) {
    sem_wait(list->listAccessSemaphore);
    qsort(list->content, list->length, list->itemSize, list->compare);

    sem_post(list->listAccessSemaphore);
    return LIST_OK;
}

/* See list.h */
ListError get_list_as_str(List* list, char** buffer, size_t* capacity) {
    sem_wait(list->listAccessSemaphore);
    for (int i = 0; i < list->length; i++) {
        int writePosition = strlen(*buffer);
        int numChars = list->toString(*buffer + writePosition,
                *capacity - writePosition, list->content[i]) + 2;

        while (numChars > *capacity - writePosition) {
            // Doubling the size is most efficient
            *capacity = *capacity * 2;
            *buffer = realloc(*buffer, *capacity * sizeof(char));
            numChars = list->toString(*buffer + writePosition, 
                    *capacity - writePosition, list->content[i]) + 2;
        }

        if (i < list->length - 1) {
            strcat(*buffer, "\n");
        }
    }

    sem_post(list->listAccessSemaphore);
    return LIST_OK;
}