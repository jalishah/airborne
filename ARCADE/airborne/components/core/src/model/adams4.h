#ifndef __MULTISTEP_INT_H__
#define __MULTISTEP_INT_H__

typedef struct 
{
    float *f0,*f1,*f2,*f3;
    unsigned int dim;
}Adams4_f;

void adams4_run(Adams4_f *f, float *x, float Ts,unsigned int enabled);
int adams4_init(Adams4_f *f,const unsigned int dim);
void adams4_term(Adams4_f *f);

#endif /* __MULTISTEP_INT_H__ */