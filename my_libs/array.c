
#ifndef ARRAY_C
#define ARRAY_C


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct ArrayHeader {
    int size; 
    int length;
    int item_size;
    int padding;
} ArrayHeader;

ArrayHeader *array_header(const void *array) {
    return ((ArrayHeader *)array - 1);
}

void * _create_array(int item_size, int size) {
    void *m = malloc(item_size * size + sizeof(ArrayHeader));

    if (m == NULL) {
        printf("_create_array: couldn't allocate memory! \n");
        return NULL;
    }

    ArrayHeader *header = m;
    header->size = size;
    header->length = 0;
    header->item_size = item_size;
    header->padding = 1;


    return header + 1;
} 

int array_length(const void *array) {
    if (array == NULL) return -1;
    ArrayHeader *header = array_header(array);
    if (header->padding != 1) {
        printf("Tried calling array_length() on a non-array! \n");
        *(char *)0 = 0;
        return -1;
    }

    return header->length;
}

int array_size(const void *array) {
    if (array == NULL) return -1;
    ArrayHeader *header = array_header(array);

    return header->size;
}

void array_remove(void *array, int i) {
    ArrayHeader *header = array_header(array);

    if (i < 0 || i >= header->length) {
        fprintf(stderr, "While removing: Index out of bounds! i: %d \n", i);
        return;
    }

    if (i == header->length - 1) {
        goto end;
    }

    memmove(
        (char *)array + i * header->item_size,
        (char *)array + (i + 1) * header->item_size,
        header->item_size * (header->length - i - 1)
    );
    // 1 2 3 4

    
    
    end: header->length--;

}

void _array_free(void *array) {
    if (!array) {
        printf("Tried to free null array!");
        return;
    }
    free(array_header(array));
}

#define array_free(arr) \
    do { \
        _array_free(arr); \
        arr = NULL; \
    } while (0)

void _expand_array(void **array) {
    ArrayHeader *header = array_header(*array);

    header->size *= 2;
    ArrayHeader *new_header = realloc(header, header->size * header->item_size + sizeof(ArrayHeader));
    *array = new_header + 1;

}

void _array_ensure_capacity(void **array) {
    ArrayHeader *header = array_header(*array);

    if (header->padding != 1) {
        printf("Header not properly initialized! Definitely gonna be a bad time. \n");
    }

    if (header->length >= header->size) {
        _expand_array(array);
    }
}

// PASS ME AROUND AND I WILL BITE
#define array(type, size) _create_array(sizeof(type), size)
#define array_from_literal(type, ...) \
    ({ \
        type __arr__[] = __VA_ARGS__; \
        int __len__ = sizeof(__arr__) / sizeof(type); \
        type *result = _create_array(sizeof(type), __len__); \
        array_header(result)->length = __len__; \
        for (int i = 0; i < __len__; i++) { \
            result[i] = __arr__[i]; \
        } \
        result; \
    })

// MIGHT CHANGE THE ADDRESS OF THE ARRAY
#define array_append(array, val) { \
    _array_ensure_capacity((void **)&(array)); \
    (array)[array_length(array)] = val; \
    array_header(array)->length++; \
}

// Note: i = array_length is possible because the bounds of the insert are 0 -> array_length (including)
#define array_insert(array, val, i) do { \
    _array_ensure_capacity((void **)&(array)); \
    memmove( \
        (char *)(array) + (i + 1) * array_header((void *)array)->item_size, \
        (char *)(array) + (i) * array_header((void *)array)->item_size, \
        (array_length((void *)array) - (i)) * array_header((void *)array)->item_size \
    ); \
    array[i] = val; \
    array_header(array)->length++; \
} while (0)

// #END
#endif // ARRAY_C