/**
 * @file ring_buffer.h
 * @author Ricardo Jorge Dias Sampaio
 * @brief Header file for the ring buffer data structure for the Secure VLC Project.
 * @version 1.0
 * @date 2024-09-03
 * \par License:
 *   \ref mit_license "MIT License".
 * @details This header file includes the declarations of functions and data structures
 * required for the implementation of a ring buffer, providing efficient data storage
 * and retrieval for the Secure VLC Project.
 * */

#ifndef RING_BUFFER_H
#define RING_BUFFER_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "config.h"

/**
 * @brief Structure representing a ring buffer.
 * @details This structure defines a ring buffer, including the buffer array,
 * head and tail indices, and the current size of the buffer.
 */
typedef struct {
    uint32_t buffer[BUFFER_MAX_SIZE]; /**< Array to hold the buffer data */
    size_t head;                      /**< Index of the head of the buffer */
    size_t tail;                      /**< Index of the tail of the buffer */
    size_t size;                      /**< Current number of elements in the buffer */
} RingBuffer;

/**
 * @brief Creates a new ring buffer.
 * @details This function allocates memory for a new RingBuffer structure and initializes its members.
 * @return Pointer to the newly created RingBuffer, or NULL if allocation fails.
 */
RingBuffer* createRingBuffer();

/**
 * @brief Frees the memory allocated for the ring buffer.
 * @details This function deallocates the memory used by the RingBuffer structure.
 * @param rb Pointer to the RingBuffer to be freed.
 */
void freeRingBuffer(RingBuffer* rb);

/**
 * @brief Pushes a volatile value onto the ring buffer.
 * @details This function adds a new value to the tail of the ring buffer.
 * @param rb Pointer to the RingBuffer.
 * @param value The volatile value to be pushed.
 * @return true if the value was successfully pushed, false if the buffer is full.
 */
bool ringBufferPush(RingBuffer* rb, volatile uint32_t value);

/**
 * @brief Pops a value from the ring buffer into a volatile variable.
 * @details This function removes and returns the value at the head of the ring buffer.
 * @param rb Pointer to the RingBuffer.
 * @param value Pointer to volatile uint32_t to store the popped value.
 * @return true if a value was successfully popped, false if the buffer is empty.
 */
bool ringBufferPop(RingBuffer* rb, volatile uint32_t* value);

/**
 * @brief Checks if the ring buffer is full.
 * @details This function determines whether the ring buffer has reached its maximum capacity.
 * @param rb Pointer to the RingBuffer.
 * @return true if the buffer is full, false otherwise.
 */
bool ringBufferIsFull(const RingBuffer* rb);

/**
 * @brief Checks if the ring buffer is empty.
 * @details This function determines whether the ring buffer contains no elements.
 * @param rb Pointer to the RingBuffer.
 * @return true if the buffer is empty, false otherwise.
 */
bool ringBufferIsEmpty(const RingBuffer* rb);


#endif // RING_BUFFER_H
