#ifndef CONFIG_H_
#define CONFIG_H_

/* structure for defining a modules object
 *
 * used as NULL terminated array like:
 * config_t options = {
 *    {"option_identifier0", &target_var},
 *    {"option_ident1", &target},
 *    {NULL, NULL}
 * };
 */
typedef struct
{
   char *ident;
   const void *dest;
} config_t;

/* Fill target variables */
void config_apply(char *module_name, config_t *options);

/* Read in config file and build internal structure */
void config_parse(char *filename);

/* Evaluate an argument for changing things ad-hoc, like:
 *
 * module_name:option_identifier=1.0
 *
 * Strings must not be in quotes.
 * Should be called after parse_config() */
void config_eval_arg(char *arg);

void usage(char *name);

int handle_cmd_line(int argc, char **argv);

#endif /* CONFIG_H_ */
