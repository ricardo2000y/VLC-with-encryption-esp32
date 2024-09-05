/**
 * @file ring_buffer.c
 * @author Ricardo Jorge Dias Sampaio
 * @brief Implementation of a ring buffer data structure for the Secure VLC Project.
 * @version 1.0
 * @date 2024-09-03
 *
 * \par License:
 *   \ref mit_license "MIT License".
 * @details This file contains the implementation of a ring buffer data structure
 * used for efficient data storage and retrieval in the Secure VLC Project. 
 */

#include "ring_buffer.h"
#include <stdlib.h>

/**
 * @brief Creates a new ring buffer.
 *
 * This function allocates memory for a new RingBuffer structure and initializes its members.
 *
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
 *
 * This function deallocates the memory used by the RingBuffer structure.
 *
 * @param rb Pointer to the RingBuffer to be freed.
 */
void freeRingBuffer(RingBuffer* rb) {
    free(rb);
}

/**
 * @brief Pushes a volatile value onto the ring buffer.
 *
 * This function adds a new value to the tail of the ring buffer.
 *
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
 *
 * This function removes and returns the value at the head of the ring buffer.
 *
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
 *
 * This function determines whether the ring buffer has reached its maximum capacity.
 *
 * @param rb Pointer to the RingBuffer.
 * @return true if the buffer is full, false otherwise.
 */
bool ringBufferIsFull(const RingBuffer* rb) {
    return rb->size == BUFFER_MAX_SIZE;
}

/**
 * @brief Checks if the ring buffer is empty.
 *
 * This function determines whether the ring buffer contains no elements.
 *
 * @param rb Pointer to the RingBuffer.
 * @return true if the buffer is empty, false otherwise.
 */
bool ringBufferIsEmpty(const RingBuffer* rb) {
    return rb->size == 0;
}
