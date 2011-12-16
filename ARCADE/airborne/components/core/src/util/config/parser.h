/* Internal header for config parser stuff
 *
 * Author: Jan Roemisch */

#ifndef PARSER_H_
#define PARSER_H_

/* Value types */
#define CONFIG_FLOAT 1
#define CONFIG_INT      2
#define CONFIG_STRING   3

/* A single value may consist of those three types */
union value_u
{
   char *s;
   float f;
   int i;
};

/* Option object
 *
 * ident: name in config file
 * type: one of the three types
 * value: value union
 * next: pointer to next object (linked list) */
struct option_s
{
   char *ident;
   int type;
   union value_u value;
   struct option_s *next;
};

/* Module object (like srf10, controller, ...)
 *
 * name: name in config file
 * options: pointer to option object list
 * next: pointer to next module object (linked list) */
struct module_s
{
   char *name;
   struct option_s *options;
   struct module_s *next;
};

#endif /* PARSER_H_ */
