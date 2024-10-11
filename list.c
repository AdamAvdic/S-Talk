// list.c

#include "list.h"
#include <stddef.h>

// Static array to store nodes
static Node nodes[LIST_MAX_NUM_NODES];

// Static array to store lists
static List lists[LIST_MAX_NUM_HEADS];

// Index of the next available node
static int nextNodeIndex = 0;

// Index of the next available list
static int nextListIndex = 0;

// Internal function to initialize the list module
static void List_initialize() {
    if (nextNodeIndex == 0 && nextListIndex == 0) {
    }
}

// Internal function to get a new node from the pool
static Node* getNewNode() {
    if (nextNodeIndex < LIST_MAX_NUM_NODES) {
        return &nodes[nextNodeIndex++];
    }
    return NULL; 
}

// Internal function to get a new list from the pool
static List* getNewList() {
    if (nextListIndex < LIST_MAX_NUM_HEADS) {
        return &lists[nextListIndex++];
    }
    return NULL; 
}

// Internal function to reset a list
static void resetList(List* list) {
    if (list != NULL) {
        list->head = NULL;
        list->current = NULL;
    }
}

List* List_create() {
  // Initialize the list module and return a new list
    List_initialize();
    return getNewList();
}

int List_count(List* pList) {
  // Count the number of nodes in the list
    if (pList != NULL) {
        Node* temp = pList->head;
        int count = 0;
        while (temp != NULL) {
            count++;
            temp = temp->next;
        }
        return count;
    }
    return LIST_FAIL;
}

void* List_first(List* pList) {
    if (pList != NULL) {
        pList->current = pList->head;
        return (pList->head != NULL) ? pList->head->data : NULL;
    }
    return NULL;
}

void* List_last(List* pList) {
    if (pList != NULL) {
        pList->current = pList->head;
        while (pList->current != NULL && pList->current->next != NULL) {
            pList->current = pList->current->next;
        }
        return (pList->current != NULL) ? pList->current->data : NULL;
    }
    return NULL;
}

void* List_next(List* pList) {
  // Move the current pointer to the next node and return its data
    if (pList != NULL && pList->current != NULL) {
        pList->current = pList->current->next;
        return (pList->current != NULL) ? pList->current->data : NULL;
    }
    return NULL;
}

void* List_prev(List* pList) {
    // Move the current pointer to the previous node and return its data
    if (pList != NULL && pList->current != NULL) {
        Node* temp = pList->current->prev;

        // If the current pointer is before the start of the list
        if (temp == NULL) {
            pList->current = NULL;
            return NULL;
        }

        pList->current = temp;
        return temp->data;
    }
    return NULL;
}


void* List_curr(List* pList) {
   // if it exists return the data of the current nod 
    return (pList != NULL && pList->current != NULL) ? pList->current->data : NULL;
}

int List_insert_after(List* pList, void* pItem) {
    if (pList != NULL) {
        Node* newNode = getNewNode();
        if (newNode != NULL) {
            newNode->data = pItem;

            if (pList->current == NULL) {
                // If the current pointer is before the start of the pList
                newNode->next = pList->head;
                pList->head = newNode;
            } else {
                newNode->next = pList->current->next;
                pList->current->next = newNode;
            }

            pList->current = newNode;
            return LIST_SUCCESS;
        }
    }
    return LIST_FAIL;
}

int List_insert_before(List* pList, void* pItem) {
    // Insert a new node before the current node
    if (pList != NULL) {
        Node* newNode = getNewNode();
        if (newNode != NULL) {
            newNode->data = pItem;

            if (pList->current == NULL) {
                // If the current pointer is before the start of the list
                newNode->next = pList->head;
                pList->head = newNode;
            } else {
                newNode->next = pList->current;
                Node* temp = pList->head;
                while (temp != NULL && temp->next != pList->current) {
                    temp = temp->next;
                }
                if (temp != NULL) {
                    temp->next = newNode;
                }
            }

            pList->current = newNode;
            return LIST_SUCCESS;
        }
    }
    return LIST_FAIL;
}

int List_append(List* pList, void* pItem) {
    // Append a new node to the end of the list
    if (pList != NULL) {
        Node* newNode = getNewNode();
        if (newNode != NULL) {
            newNode->data = pItem;

            if (pList->head == NULL) {
                // If the list is empty
                pList->head = newNode;
            } else {
                Node* temp = pList->head;
                while (temp->next != NULL) {
                    temp = temp->next;
                }
                temp->next = newNode;
            }

            pList->current = newNode;
            return LIST_SUCCESS;
        }
    }
    return LIST_FAIL;
}

int List_prepend(List* pList, void* pItem) {
    // Prepend a new node to the beginning of the list
    if (pList != NULL) {
        Node* newNode = getNewNode();
        if (newNode != NULL) {
            newNode->data = pItem;
            newNode->next = pList->head;
            pList->head = newNode;
            pList->current = newNode;
            return LIST_SUCCESS;
        }
    }
    return LIST_FAIL;
}


void* List_remove(List* pList) {
  // Remove the current node from the list and return its data
    if (pList != NULL && pList->current != NULL) {
        Node* removedNode = pList->current;
        Node* temp = pList->head;

        if (removedNode == pList->head) {
            pList->head = removedNode->next;
            pList->current = removedNode->next;
        } else {
            while (temp != NULL && temp->next != removedNode) {
                temp = temp->next;
            }
            if (temp != NULL) {
                temp->next = removedNode->next;
                pList->current = removedNode->next;
            }
        }

        return removedNode->data;
    }
    return NULL;
}

void* List_trim(List* pList) {
   // Remove the last node from the list and return its data
    if (pList != NULL && pList->head != NULL) {
        Node* lastNode = NULL;
        Node* temp = pList->head;

        while (temp->next != NULL) {
            lastNode = temp;
            temp = temp->next;
        }

        if (lastNode != NULL) {
            lastNode->next = NULL;
            pList->current = lastNode;
        } else {
            // Only one node in the list
            pList->head = NULL;
            pList->current = NULL;
        }

        return temp->data;
    }
    return NULL;
}

void List_concat(List* pList1, List* pList2) {
    // Concatenate the second list to the end of the first list
    if (pList1 != NULL && pList2 != NULL) {
        if (pList1->head == NULL) {
            // If the first list is empty set it to the second list
            pList1->head = pList2->head;
        } else {
            // Create new nodes for the second list and append them to the first list
            Node* temp = pList1->head;
            while (temp->next != NULL) {
                temp = temp->next;
            }

            Node* current2 = pList2->head;
            while (current2 != NULL) {
                Node* newNode = getNewNode();
                if (newNode != NULL) {
                    newNode->data = current2->data;
                    temp->next = newNode;
                    temp = newNode;
                }
                current2 = current2->next;
            }
        }

        pList1->current = pList1->head;

        // Reset the second list
        resetList(pList2);
    }
}


typedef void (*FREE_FN)(void* pItem);
void List_free(List* pList, FREE_FN pItemFreeFn) {
  // Free the memory occupied by the nodes in the list
    if (pList != NULL) {
        Node* current = pList->head;
        while (current != NULL) {
            Node* temp = current;
            current = current->next;

            if (pItemFreeFn != NULL) {
                pItemFreeFn(temp->data);
            }
        }

        // Reset the list
        resetList(pList);
    }
}



// Search pList, starting at the current item, until the end is reached or a match is found. 
// In this context, a match is determined by the comparator parameter. This parameter is a
// pointer to a routine that takes as its first argument an item pointer, and as its second 
// argument pComparisonArg. Comparator returns 0 if the item and comparisonArg don't match, 
// or 1 if they do. Exactly what constitutes a match is up to the implementor of comparator. 
// 
// If a match is found, the current pointer is left at the matched item and the pointer to 
// that item is returned. If no match is found, the current pointer is left beyond the end of 
// the list and a NULL pointer is returned.
// 
// If the current pointer is before the start of the pList, then start searching from
// the first node in the list (if any).
typedef bool (*COMPARATOR_FN)(void* pItem, void* pComparisonArg);
void* List_search(List* pList, COMPARATOR_FN pComparator, void* pComparisonArg) {
  // Search the list based on the comparator function
    if (pList != NULL && pComparator != NULL) {
        Node* current = (pList->current != NULL) ? pList->current : pList->head;

        while (current != NULL) {
            if (pComparator(current->data, pComparisonArg)) {
                pList->current = current;
                return current->data;
            }
            current = current->next;
        }

        // If no match is found, set the current pointer beyond the end of the list
        pList->current = NULL;
    }
    return NULL;
}