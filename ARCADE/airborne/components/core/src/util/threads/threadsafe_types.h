
/*
 * @file threadsafe_types.h
 * @brief definition and access macros for threadsafe types/variables
 * @uthor Tobias Simon (Ilmenau University of Technology)
 *
 * used to declare thread-safe variables
 * usage:
 *
 * declaration at module level:
 *    DECLARE_THREADSAFE_TYPE(float);
 *
 * declaration of individual variable:
 *    threadsafe_float f;
 *
 * initialization phase (once):
 *    threadsafe_float_init(&f);
 *
 * in writing thread:
 *    threadsafe_float_set(&f, 1.0);
 *
 * in reading thread:
 *    threadsafe_float_get(&f));
 */


#ifndef THREADSAFE_TYPES_H
#define THREADSAFE_TYPES_H


#include <pthread.h>


#define DECLARE_THREADSAFE_TYPE(type) \
   \
   typedef struct \
   { \
      type value; \
      pthread_mutex_t mutex; \
   } \
   threadsafe_##type##_t; \
   \
   static inline void threadsafe_##type##_init(threadsafe_##type##_t *ts_var, type val) \
   { \
      pthread_mutex_init(&ts_var->mutex, NULL); \
      ts_var->value = val; \
   } \
   \
   static inline type threadsafe_##type##_get(threadsafe_##type##_t *ts_var) \
   { \
      type copy; \
      pthread_mutex_lock(&ts_var->mutex); \
      copy = ts_var->value; \
      pthread_mutex_unlock(&ts_var->mutex); \
      return copy; \
   } \
   \
   static inline void threadsafe_##type##_set(threadsafe_##type##_t *ts_var, type data) \
   { \
      pthread_mutex_lock(&ts_var->mutex); \
      ts_var->value = data; \
      pthread_mutex_unlock(&ts_var->mutex); \
   } \
   \
   static inline void threadsafe_##type##_copy(threadsafe_##type##_t *ts_var_a, threadsafe_##type##_t *ts_var_b) \
   { \
      pthread_mutex_lock(&ts_var_a->mutex); \
      pthread_mutex_lock(&ts_var_b->mutex); \
      ts_var_a->value = ts_var_b->value; \
      pthread_mutex_unlock(&ts_var_b->mutex); \
      pthread_mutex_unlock(&ts_var_a->mutex); \
   }


typedef char * string;

DECLARE_THREADSAFE_TYPE(float);
DECLARE_THREADSAFE_TYPE(int);
DECLARE_THREADSAFE_TYPE(string);


#endif /* THREADSAFE_TYPES_H */
