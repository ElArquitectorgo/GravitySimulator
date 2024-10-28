#ifndef VECTOR2_H
#define VECTOR2_H

typedef struct {
    float x;
    float y;
} Vector2;

/**
 * Adds the components of b in a.
 *
 * \param a target vector
 * \param b second vector
 *
 */
void add(Vector2* a, Vector2* b);

/**
 * Subtracts the components of b in a.
 *
 * \param a target vector
 * \param b second vector
 *
 */
void subs(Vector2* a, Vector2* b);

/**
 * Compute the euclidean distance between two vectors.
 *
 * \param a vector to be substracted
 * \param b second vector
 * \returns Distance from a to b.
 *
 */
float distance(Vector2 a, Vector2 b);

/**
 * Subtracts b in a.
 *
 * \param a vector to be substracted
 * \param b second vector
 * \returns A new vector resulting from the subtraction of b in a.
 *
 */
Vector2 sub(Vector2 a, Vector2 b);

/**
 * Compute the dot product between two vectors.
 *
 * \param a first vector
 * \param b second vector
 * \returns The dot product of a and b.
 *
 */
float dot(Vector2 a, Vector2 b);

/**
 * Multiplies a vector by a scalar.
 *
 * \param a vector
 * \param scalar float number
 * \returns The components of a multiplied by scalar.
 *
 */
Vector2 mult(Vector2 a, float scalar);

/**
 * Normalize the input vector.
 *
 * \param v vector
 *
 */
void normalize(Vector2* v);

/**
 * Copy input vector.
 *
 * \param v vector to copy
 * \returns A new vector with the same components.
 *
 */
Vector2 copy(Vector2 v);

/**
 * Prints vector components.
 * 
 */
void print_vector(Vector2* v);

#endif
