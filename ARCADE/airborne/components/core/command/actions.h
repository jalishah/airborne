
/*
 * purpose: actions interface
 * author: Tobias Simon, Ilmenau University of Technology
 */

#ifndef __ACTIONS_H__
#define __ACTIONS_H__


/*
 * initializes the actions module
 */
void actions_init(void);


/*
 * starts motors and waits until they are running
 * returns 0 on success or
 *        -1 if spin-up failed
 */
int action_spin_up(void);


/*
 * stops motors and waits until they are halted
 */
void action_spin_down(void);


#endif /* __ACTIONS_H__ */

