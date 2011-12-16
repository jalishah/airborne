#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "config.h"
#include "parser.h"
#include <getopt.h>


extern struct module_s *module_list;


/* Search for module in linked list (of modules)
 *
 * returns NULL if module does not exist */
static struct module_s *find_module(char *module_name)
{
   struct module_s *ptr;

   for (ptr = module_list; ptr; ptr = ptr->next)
   {
      if (strcmp(module_name, ptr->name) == 0)
      {
         return ptr;
      }
   }

   return NULL;
}


/* The same for an option in a given module */
static struct option_s *find_parameter(struct module_s *mod, char *ident)
{
   struct option_s *ptr;

   for (ptr = mod->options; ptr; ptr = ptr->next)
   {
      if (strcmp(ident, ptr->ident) == 0)
      {
         return ptr;
      }
   }

   return NULL;
}


/* Evaluate string for changing options ad-hoc, i.e. on command line */
void config_eval_arg(char *arg)
{
   char **ap, *argv[3];
   struct module_s *mod;
   struct option_s *opt;

   /* Separate strings */
   for (ap = argv; (*ap = strsep(&arg, ":=")) != NULL;)
   {
      if (**ap != '\0')
      {
         if (++ap >= &argv[3])
         {
            break;
         }
      }
   }

   /* Check if there are enough arguments */
   if (ap < &argv[3])
   {
      printf("WARNING: Missing something on command line.\n");
      return;
   }

   /* Search for given module by name */
   mod = find_module(argv[0]);
   if (!mod)
   {
      printf("WARNING: No module named '%s'.\n", argv[0]);
      return;
   }

   /* Search for parameter in found object */
   opt = find_parameter(mod, argv[1]);
   if (!opt)
   {
      printf("WARNING: No option named '%s' in module '%s'.\n",
             argv[1], argv[0]);
      return;
   }

   /* Update values in option object */
   switch (opt->type)
   {
      case CONFIG_INT:
         opt->value.i = (int)strtouq(argv[2], NULL, 0);
         break;

      case CONFIG_FLOAT:
         opt->value.f = (float)atof(argv[2]);
         break;

      case CONFIG_STRING:
         opt->value.s = strdup(argv[2]);
         break;

      default:
         printf("WARNING: Major fuckup.\n");
   }

}


/* Called by module to update internal variables from config */
void config_apply(char *module_name, config_t *options)
{
   int i, j = 0, found;
   struct module_s *mod = find_module(module_name);
   struct option_s *ptr;

   if (!mod)
   {
      fprintf(stderr, "WARNING: No module named '%s' found in config file.\n",
              module_name);
      return;
   }

   /* Walk through given config_t array */
   for (i = 0; options[i].ident; i++)
   {
      found = 0;

      /* Walk through module's parameters */
      for (ptr = mod->options, j = 0; ptr; ptr = ptr->next, j++)
      {
         if (strcmp(options[i].ident, ptr->ident) == 0)
         {
            found = 1;
            /* Copy values */
            switch (ptr->type)
            {
               case CONFIG_INT:
                  *(int *)(options[i].dest) = ptr->value.i;
                  break;

               case CONFIG_FLOAT:
                  *(float *)(options[i].dest) = ptr->value.f;
                  break;

               case CONFIG_STRING:
                  *(char **)(options[i].dest) = ptr->value.s;
                  break;

               default:
                  fprintf(stderr, "WARNING: Unknown option type encountered (%i).", ptr->type);
            }
         }
      }
      if (!found)
      {
         fprintf(stderr, "ERROR: Could not find '%s:%s' in config file.\n", module_name, options[i].ident);
         exit(EXIT_FAILURE);
      }
   }

   if (j > i)
   {
      fprintf(stderr, "NOTICE: Excess parameters in config file for module \"%s\".\n", module_name);
   }
}


void usage(char *name)
{
   fprintf(stderr, "%s -f file [-h] [-o module:parameter=value -o ...]\n"
           "\t-f\tFile used for configuration. Must be first argument.\n"
           "\t-o\tChange something from config ad-hoc, ie:\n"
           "\t\t\t-o alt_controller:pid_d=0.2\n",
           name);
}


int handle_cmd_line(int argc, char **argv)
{
   int cfg_given = 0;
   int ch;

   while ((ch = getopt(argc, argv, "hf:o:")) != -1)
   {
      switch (ch)
      {
         case 'f':
            config_parse(optarg);
            cfg_given = 1;
            break;

         case 'o':
            if (cfg_given)
            {
               config_eval_arg(optarg);
               break;
            }
            fprintf(stderr, "ERROR: No configuration file passed.\n");

         case 'h':
         default:
            usage(argv[0]);
            exit(EXIT_FAILURE);
      }
   }

   if (!cfg_given)
   {
      fprintf(stderr, "ERROR: No configuration file passed.\n");
      usage(argv[0]);
      exit(EXIT_FAILURE);
   }

   return optind;
}

