

template <class T>
    class DynamicArray {
        public:
            T *array;
            size_t used;
            size_t size;

            void initArray(size_t initialSize)
            {
                array = (T *)malloc(initialSize * sizeof(T));
                used = 0;
                size = initialSize;
            }

            void insertArray(T element)
            {
                // a->used is the number of used entries, because a->array[a->used++] updates a->used only *after* the array has been accessed.
                // Therefore a->used can go up to a->size
                if (used == size)
                {
                    size *= 2;
                    array = (T *)realloc(array, size * sizeof(T));
                }
                array[used++] = element;
            }

            void freeArray()
            {
                free(array);
                array = NULL;
                used = size = 0;
            }
    };