

#include <malloc.h>

#include "adams4.h"


void adams4_init(Adams4_f *f,const unsigned int dim)
{
    f->dim = dim;
    f->f0 = malloc(dim * sizeof(float));
    f->f1 = malloc(dim * sizeof(float));
    f->f2 = malloc(dim * sizeof(float));
    f->f3 = malloc(dim * sizeof(float));

    for (int i = 0; i<dim; i++)
    {   
        f->f0[i] = 0.0;
        f->f1[i] = 0.0;
        f->f2[i] = 0.0;
        f->f3[i] = 0.0;
    }
    return 0;
}

void adams4_term(Adams4_f *f)
{
    free(f->f0);
    free(f->f1);
    free(f->f2);
    free(f->f3);
    free(f);
}

/*
 * x_(k+1) = x_(k) + Ts*(55 * x_(k) - 59 * x_(k-1) + 37 * x_(k-2) - 9 * x_(k-3))/24
*/
void adams4_run(Adams4_f *f, float *x,float Ts,unsigned int enabled)
{
    if (enabled>0)
    {
        unsigned int i;
        for (i=0; i<f->dim; i++)
        {
            x[i] += Ts * (55*f->f0[i] - 59 * f->f1[i] + 37 * f->f2[i] - 9 * f->f3[i])/24;
        }
    }
    float *tempPtr = f->f3;
    f->f3 = f->f2;
    f->f2 = f->f1;
    f->f1 = f->f0;
    f->f0 = tempPtr;
}
