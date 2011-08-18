#ifndef AST_MISC_H
#define AST_MISC_H

#define STRINGIFY(x) #x

/*
 * Returns true if t1 is more recent than t2. Handles integer wrap around.
 * Assumes that t1 and t2 are near each other.
 */
bool compareTicks(unsigned int t1, unsigned int t2);

#endif /* AST_MISC_H */

