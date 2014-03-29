/*ioctl define*/
#define PWM_FLAG 'k'
#define PWM_ON _IO(PWM_FLAG,0)
#define PWM_OFF _IO(PWM_FLAG,1)
#define PWM_PRE _IO(PWM_FLAG,2)
#define PWM_CNT _IO(PWM_FLAG,3)
