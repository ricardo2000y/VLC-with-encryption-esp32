#include "ring_buffer.h"
#include <stdlib.h>

/**
 * @brief Creates a new ring buffer.
 * @return Pointer to the newly created RingBuffer, or NULL if allocation fails.
 */
RingBuffer* createRingBuffer() {
    RingBuffer* rb = (RingBuffer*)malloc(sizeof(RingBuffer));
    if (rb != NULL) {
        rb->head = 0;
        rb->tail = 0;
        rb->size = 0;
    }
    return rb;
}

/**
 * @brief Frees the memory allocated for the ring buffer.
 * @param rb Pointer to the RingBuffer to be freed.
 */
void freeRingBuffer(RingBuffer* rb) {
    free(rb);
}

/**
 * @brief Pushes a volatile value onto the ring buffer.
 * @param rb Pointer to the RingBuffer.
 * @param value The volatile value to be pushed.
 * @return true if the value was successfully pushed, false if the buffer is full.
 */
bool ringBufferPush(RingBuffer* rb, volatile uint32_t value) {
    if (ringBufferIsFull(rb)) {
        return false; // Buffer is full, cannot push
    }
    rb->buffer[rb->tail] = (uint32_t)value; // Cast volatile to non-volatile
    rb->tail = (rb->tail + 1) % BUFFER_MAX_SIZE; // Wrap around
    rb->size++;
    return true; // Successfully pushed
}

/**
 * @brief Pops a value from the ring buffer into a volatile variable.
 * @param rb Pointer to the RingBuffer.
 * @param value Pointer to volatile uint32_t to store the popped value.
 * @return true if a value was successfully popped, false if the buffer is empty.
 */
bool ringBufferPop(RingBuffer* rb, volatile uint32_t* value) {
    if (ringBufferIsEmpty(rb)) {
        return false; // Buffer is empty, cannot pop
    }
    *value = (volatile uint32_t)rb->buffer[rb->head]; // Cast non-volatile to volatile
    rb->head = (rb->head + 1) % BUFFER_MAX_SIZE; // Wrap around
    rb->size--;
    return true; // Successfully popped
}

/**
 * @brief Checks if the ring buffer is full.
 * @param rb Pointer to the RingBuffer.
 * @return true if the buffer is full, false otherwise.
 */
bool ringBufferIsFull(const RingBuffer* rb) {
    return rb->size == BUFFER_MAX_SIZE;
}

/**
 * @brief Checks if the ring buffer is empty.
 * @param rb Pointer to the RingBuffer.
 * @return true if the buffer is empty, false otherwise.
 */
bool ringBufferIsEmpty(const RingBuffer* rb) {
    return rb->size == 0;
}

/**
 * @brief Gets the current number of elements in the ring buffer.
 * @param rb Pointer to the RingBuffer.
 * @return The number of elements currently in the buffer.
 */
size_t ringBufferSize(const RingBuffer* rb) {
    return rb->size;
}