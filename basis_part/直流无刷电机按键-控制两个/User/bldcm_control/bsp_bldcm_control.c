/**
  ******************************************************************************
  * @file    bsp_bldcm_control.c
  * @author  fire
  * @version V1.0
  * @date    2019-xx-xx
  * @brief   无刷电机控制接口
  ******************************************************************************
  * @attention
  *
  * 实验平台:野火  STM32 F407 开发板 
  * 论坛    :http://www.firebbs.cn
  * 淘宝    :http://firestm32.taobao.com
  *
  ******************************************************************************
  */ 

#include ".\bldcm_control\bsp_bldcm_control.h"

/* 私有变量 */
static bldcm_data_t motor1_bldcm_data;
static bldcm_data_t motor2_bldcm_data;

/* 局部函数 */
static void sd_gpio_config(void);

/**
  * @brief  电机初始化
  * @param  无
  * @retval 无
  */
void bldcm_init(void)
{
  TIMx_Configuration();    // 电机控制定时器，引脚初始化
  hall_motor1_tim_config();       // 霍尔传感器初始化
	hall_motor2_tim_config();       // 霍尔传感器初始化
  sd_gpio_config();        // sd 引脚初始化
}

/**
  * @brief  电机 SD 控制引脚初始化
  * @param  无
  * @retval 无
  */
static void sd_gpio_config(void)
{
  GPIO_InitTypeDef GPIO_InitStruct;
  
  /* 定时器通道功能引脚端口时钟使能 */
	MOTOR1_SHUTDOWN_GPIO_CLK_ENABLE();
 	MOTOR2_SHUTDOWN_GPIO_CLK_ENABLE(); 
	
  /* 引脚IO初始化 */
	/*设置输出类型*/
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	/*设置引脚速率 */ 
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
	/*选择要控制的GPIO引脚*/	
	GPIO_InitStruct.Pin = MOTOR1_SHUTDOWN_PIN;
  
	/*调用库函数，使用上面配置的GPIO_InitStructure初始化GPIO*/
  HAL_GPIO_Init(MOTOR1_SHUTDOWN_GPIO_PORT, &GPIO_InitStruct);
	/*选择要控制的GPIO引脚*/	
	GPIO_InitStruct.Pin = MOTOR2_SHUTDOWN_PIN;
	HAL_GPIO_Init(MOTOR2_SHUTDOWN_GPIO_PORT, &GPIO_InitStruct);
}

/**
  * @brief  设置电机速度
  * @param  v: 速度（占空比）
  * @retval 无
  */
void set_motor1_bldcm_speed(uint16_t v)
{
  motor1_bldcm_data.dutyfactor = v;
  
  set_motor1_pwm_pulse(v);     // 设置速度
}

void set_motor2_bldcm_speed(uint16_t v)
{
  motor2_bldcm_data.dutyfactor = v;
  
  set_motor2_pwm_pulse(v);     // 设置速度
}

/**
  * @brief  设置电机方向
  * @param  无
  * @retval 无
  */
void set_motor1_bldcm_direction(motor_dir_t dir)
{
  motor1_bldcm_data.direction = dir;
}

void set_motor2_bldcm_direction(motor_dir_t dir)
{
  motor2_bldcm_data.direction = dir;
}

/**
  * @brief  获取电机当前方向
  * @param  无
  * @retval 无
  */
motor_dir_t get_motor1_bldcm_direction(void)
{
  return motor1_bldcm_data.direction;
}

motor_dir_t get_motor2_bldcm_direction(void)
{
  return motor2_bldcm_data.direction;
}

/**
  * @brief  使能电机
  * @param  无
  * @retval 无
  */
void set_motor1_bldcm_enable(void)
{
  MOTOR1_BLDCM_ENABLE_SD();
  hall_motor1_enable();
}

void set_motor2_bldcm_enable(void)
{
  MOTOR2_BLDCM_ENABLE_SD();
  hall_motor2_enable();
}

/**
  * @brief  禁用电机
  * @param  无
  * @retval 无
  */
void set_motor1_bldcm_disable(void)
{
  /* 禁用霍尔传感器接口 */
  hall_motor1_disable();
  
  /* 停止 PWM 输出 */
  stop_motor1_pwm_output();
  
  /* 关闭 MOS 管 */
  MOTOR1_BLDCM_DISABLE_SD();
}

void set_motor2_bldcm_disable(void)
{
  /* 禁用霍尔传感器接口 */
  hall_motor2_disable();
  
  /* 停止 PWM 输出 */
  stop_motor2_pwm_output();
  
  /* 关闭 MOS 管 */
  MOTOR2_BLDCM_DISABLE_SD();
}

/*********************************************END OF FILE**********************/
