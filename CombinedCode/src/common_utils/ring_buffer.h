#ifndef RING_BUFFER_H
#define RING_BUFFER_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "config.h"

/**
 * @brief Structure representing a ring buffer.
 */
typedef struct {
    uint32_t buffer[BUFFER_MAX_SIZE]; /**< Array to hold the buffer data */
    size_t head;                      /**< Index of the head of the buffer */
    size_t tail;                      /**< Index of the tail of the buffer */
    size_t size;                      /**< Current number of elements in the buffer */
} RingBuffer;

/**
 * @brief Creates a new ring buffer.
 * @return Pointer to the newly created RingBuffer, or NULL if allocation fails.
 */
RingBuffer* createRingBuffer();

/**
 * @brief Frees the memory allocated for the ring buffer.
 * @param rb Pointer to the RingBuffer to be freed.
 */
void freeRingBuffer(RingBuffer* rb);

/**
 * @brief Pushes a volatile value onto the ring buffer.
 * @param rb Pointer to the RingBuffer.
 * @param value The volatile value to be pushed.
 * @return true if the value was successfully pushed, false if the buffer is full.
 */
bool ringBufferPush(RingBuffer* rb, volatile uint32_t value);

/**
 * @brief Pops a value from the ring buffer into a volatile variable.
 * @param rb Pointer to the RingBuffer.
 * @param value Pointer to volatile uint32_t to store the popped value.
 * @return true if a value was successfully popped, false if the buffer is empty.
 */
bool ringBufferPop(RingBuffer* rb, volatile uint32_t* value);

/**
 * @brief Checks if the ring buffer is full.
 * @param rb Pointer to the RingBuffer.
 * @return true if the buffer is full, false otherwise.
 */
bool ringBufferIsFull(const RingBuffer* rb);

/**
 * @brief Checks if the ring buffer is empty.
 * @param rb Pointer to the RingBuffer.
 * @return true if the buffer is empty, false otherwise.
 */
bool ringBufferIsEmpty(const RingBuffer* rb);

/**
 * @brief Gets the current number of elements in the ring buffer.
 * @param rb Pointer to the RingBuffer.
 * @return The number of elements currently in the buffer.
 */
size_t ringBufferSize(const RingBuffer* rb);

#endif // RING_BUFFER_H