#include "./pid/bsp_pid.h"

//定义全局变量

_pid pid;

/**
  * @brief  PID参数初始化
	*	@note 	无
  * @retval 无
  */
void PID_param_init()
{
		/* 初始化参数 */
    pid.target_val=40.0;				
    pid.actual_val=0.0;
    pid.err=0.0;
    pid.err_last=0.0;
    pid.integral=0.0;

		pid.Kp = 0;
		pid.Ki = 3.5;
		pid.Kd = 0;

#if defined(PID_ASSISTANT_EN)
    float pid_temp[3] = {pid.Kp, pid.Ki, pid.Kd};
    set_computer_value(SEND_P_I_D_CMD, CURVES_CH1, pid_temp, 3);     // 给通道 1 发送 P I D 值
#endif
}

/**
  * @brief  设置目标值
  * @param  val		目标值
	*	@note 	无
  * @retval 无
  */
void set_pid_target(float temp_val)
{
  pid.target_val = temp_val;    // 设置当前的目标值
}

/**
  * @brief  获取目标值
  * @param  无
	*	@note 	无
  * @retval 目标值
  */
float get_pid_target(void)
{
  return pid.target_val;    // 设置当前的目标值
}

/**
  * @brief  设置比例、积分、微分系数
  * @param  p：比例系数 P
  * @param  i：积分系数 i
  * @param  d：微分系数 d
	*	@note 	无
  * @retval 无
  */
void set_p_i_d(float p, float i, float d)
{
  	pid.Kp = p;    // 设置比例系数 P
		pid.Ki = i;    // 设置积分系数 I
		pid.Kd = d;    // 设置微分系数 D
}

/**
  * @brief  PID算法实现
  * @param  actual_val:实际值
	*	@note 	无
  * @retval 通过PID计算后的输出
  */
float PID_realize(float actual_val)
{
		/* 限制电流幅值,野火电机空载时最大电流在100ma左右,如果过大容易积分饱和 */
		if(pid.target_val >= 120) pid.target_val = 120.0;
		else if(pid.target_val <= 5) pid.target_val = 0.0;

		/*计算目标值与实际值的误差*/
    pid.err=pid.target_val-actual_val;
	
//		/* 死区设定,电流精度限制,误差在5ma就不累计误差 */
//		if(pid.err <= 5 && pid.err >= -5) 
//			pid.err = 0;
	
    /*误差累积*/
    pid.integral+=pid.err;
		/*PID算法实现*/
    pid.actual_val=pid.Kp*pid.err+pid.Ki*pid.integral+pid.Kd*(pid.err-pid.err_last);
		/*误差传递*/
    pid.err_last=pid.err;
		/*返回当前实际值*/
    return pid.actual_val;
}

/**
  * @brief  定时器周期调用函数
  * @param  无
	*	@note 	无
  * @retval 无
  */
//一直调节
//void time_period_fun()
//{
//	float set_point=200.0;
//	float val=PID_realize(set_point);
//	printf("val,%f;act,%f\n",set_point,val);
//}
//调节趋于稳定后停止
void time_period_fun()
{
	static int flag=0;
	static int num=0;
	static int run_i=0;
		
	float set_point=200.0;
	if(!flag)
	{
		float val=PID_realize(set_point);
		printf("val,%f;act,%f\n",set_point,val);	
		run_i++;
		if(abs(val-set_point)<=1)
		{
			num++;
		}
		else//必须满足连续次数
		{
			num=0;
		}
		if(num>20)//稳定次数
		{
			printf("PID算法运行%d 次后稳定\r\n",run_i);
			flag=1;
		}
	}
}




