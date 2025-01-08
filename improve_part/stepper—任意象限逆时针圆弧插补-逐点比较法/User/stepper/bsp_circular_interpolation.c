/**
  ******************************************************************************
  * @file    bsp_stepper_liner_interpolation.c
  * @author  fire
  * @version V1.0
  * @date    2020-xx-xx
  * @brief   任意象限逆时针圆弧插补-逐点比较法
  ******************************************************************************
  * @attention
  *
  * 实验平台:野火  STM32 F407 开发板  
  * 论坛    :http://www.firebbs.cn
  * 淘宝    :http://firestm32.taobao.com
  *
  ******************************************************************************
  */
#include "./stepper/bsp_circular_interpolation.h"
#include "./usart/bsp_debug_usart.h"
#include <stdlib.h>

Axis_TypeDef axis;
Quadrant_TypeDef quadrant;
CircularInterpolation_TypeDef interpolation_para = {0};

/**
  * @brief  设置进给方向
  * @param  coord_x
  * @param  coord_y
  * @retval 无
  */
static void Set_Feed_DIR(int32_t coord_x, int32_t coord_y)
{
  if(coord_x > 0)/* x正半轴 */
  {
    if(coord_y >= 0)/* 第一象限 */
    {
      interpolation_para.crood_pos = quadrant_1st;
      interpolation_para.dir_x = CCW;
      interpolation_para.dir_y = CW;
      MOTOR_DIR(step_motor[x_axis].dir_port, step_motor[x_axis].dir_pin, CCW);
      MOTOR_DIR(step_motor[y_axis].dir_port, step_motor[y_axis].dir_pin, CW);
    }
    else/* 第四象限 */
    {
      interpolation_para.crood_pos = quadrant_4th;
      interpolation_para.dir_x = CW;
      interpolation_para.dir_y = CW;
      MOTOR_DIR(step_motor[x_axis].dir_port, step_motor[x_axis].dir_pin, CW);
      MOTOR_DIR(step_motor[y_axis].dir_port, step_motor[y_axis].dir_pin, CW);
    }
  }
  else if(coord_x < 0)/* x负半轴 */
  {
    if(coord_y > 0)/* 第二象限 */
    {
      interpolation_para.crood_pos = quadrant_2nd;
      interpolation_para.dir_x = CCW;
      interpolation_para.dir_y = CCW;
      MOTOR_DIR(step_motor[x_axis].dir_port, step_motor[x_axis].dir_pin, CCW);
      MOTOR_DIR(step_motor[y_axis].dir_port, step_motor[y_axis].dir_pin, CCW);
    }
    else/* 第三象限 */
    {
      interpolation_para.crood_pos = quadrant_3rd;
      interpolation_para.dir_x = CW;
      interpolation_para.dir_y = CCW;
      MOTOR_DIR(step_motor[x_axis].dir_port, step_motor[x_axis].dir_pin, CW);
      MOTOR_DIR(step_motor[y_axis].dir_port, step_motor[y_axis].dir_pin, CCW);
    }
  }
  else if(coord_x == 0)/* x=0，当前点在Y轴上 */
  {
    if(coord_y >= 0)/* 第二象限 */
    {
      interpolation_para.crood_pos = quadrant_2nd;
      interpolation_para.dir_x = CCW;
      interpolation_para.dir_y = CCW;
      MOTOR_DIR(step_motor[x_axis].dir_port, step_motor[x_axis].dir_pin, CCW);
      MOTOR_DIR(step_motor[y_axis].dir_port, step_motor[y_axis].dir_pin, CCW);
    }
    else/* 第四象限 */
    {
      interpolation_para.crood_pos = quadrant_4th;
      interpolation_para.dir_x = CW;
      interpolation_para.dir_y = CW;
      MOTOR_DIR(step_motor[x_axis].dir_port, step_motor[x_axis].dir_pin, CW);
      MOTOR_DIR(step_motor[y_axis].dir_port, step_motor[y_axis].dir_pin, CW);
    }
  }
}
  
/**
  * @brief  任意象限顺圆插补运动
  * @param  start_x：圆弧起点坐标X
  * @param  start_y：圆弧起点坐标Y
  * @param  stop_x：圆弧终点坐标X
  * @param  stop_y：圆弧终点坐标Y
  * @param  speed：进给速度
  * @retval 无
  */
void Circular_InterPolation_CW(int32_t start_x, int32_t start_y, int32_t stop_x, int32_t stop_y, uint16_t speed)
{
  /* 判断当前是否正在做插补运动 */
  if(interpolation_para.motionstatus != 0)
    return;
  
  /* 检查起点、终点坐标是否在同一个圆上 */
  if(((start_x * start_x) + (start_y * start_y)) != ((stop_x * stop_x) + (stop_y * stop_y)))
    return;
  
  /* 偏差清零 */
  interpolation_para.deviation = 0;
  
  /* 起点坐标 */
  interpolation_para.startpoint_x = start_x;
  interpolation_para.startpoint_y = start_y;
  /* 终点坐标 */
  interpolation_para.endpoint_x = stop_x;
  interpolation_para.endpoint_y = stop_y;
  /* 所需脉冲数是从起点到终点的脉冲数之和 */
  interpolation_para.endpoint_pulse = abs(stop_x - start_x) + abs(stop_y - start_y);
  
  /* 根据坐标设置X、Y运动方向 */
  Set_Feed_DIR(interpolation_para.startpoint_x, interpolation_para.startpoint_y);
  
  /* 起点坐标y=0，说明起点在x轴上，直接向y轴进给可减小误差 */
  if(interpolation_para.startpoint_y == 0)
  {
    /* 第一步活动轴为Y轴 */
    interpolation_para.active_axis = y_axis;
    /* 计算偏差 */
    if(interpolation_para.dir_y == CW)
      interpolation_para.deviation += (2 * interpolation_para.startpoint_y + 1);
    else
      interpolation_para.deviation -= (2 * interpolation_para.startpoint_y + 1);
  }
  else
  {
    /* 第一步活动轴为X轴 */
    interpolation_para.active_axis = x_axis;
    /* 计算偏差 */
    if(interpolation_para.dir_x == CW)
      interpolation_para.deviation += (2 * interpolation_para.startpoint_x + 1);
    else
      interpolation_para.deviation -= (2 * interpolation_para.startpoint_x + 1);
  }
  
  /* 设置速度 */
  __HAL_TIM_SET_COMPARE(&TIM_StepperHandle, step_motor[x_axis].pul_channel, speed);
  __HAL_TIM_SET_COMPARE(&TIM_StepperHandle, step_motor[y_axis].pul_channel, speed);
  __HAL_TIM_SET_AUTORELOAD(&TIM_StepperHandle, speed * 2);
  
  /* 使能主输出 */
  __HAL_TIM_MOE_ENABLE(&TIM_StepperHandle);
  /* 开启活动轴比较通道输出 */
  TIM_CCxChannelCmd(MOTOR_PUL_TIM, step_motor[interpolation_para.active_axis].pul_channel, TIM_CCx_ENABLE);
  HAL_TIM_Base_Start_IT(&TIM_StepperHandle);
  
  interpolation_para.motionstatus = 1;
}

/**
  * @brief  定时器比较中断回调函数
  * @param  htim：定时器句柄指针
	*	@note   无
  * @retval 无
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  uint32_t last_axis = 0;
  
  /* 记录上一步的进给活动轴 */
  last_axis = interpolation_para.active_axis;
  
  /* 根据进给方向刷新坐标 */
  switch(last_axis)
  {
    case x_axis:
      switch(interpolation_para.dir_x)
      {
        case CCW: interpolation_para.startpoint_x--; break;
        case CW:  interpolation_para.startpoint_x++; break;
      }
      break;
    case y_axis:
      switch(interpolation_para.dir_y)
      {
        case CCW: interpolation_para.startpoint_y--; break;
        case CW:  interpolation_para.startpoint_y++; break;
      }
      break;
  }
  
  /* 根据上一步的偏差，判断的进给方向，并计算下一步的偏差 */
  /* 偏差>=0，在圆弧外侧，应向圆内进给 */
  if(interpolation_para.deviation >= 0)
  {
    switch(interpolation_para.crood_pos)/* 判断当前插补象限 */
    {
      case quadrant_1st:/* 一三象限 */
      case quadrant_3rd:
        interpolation_para.active_axis = x_axis;
        if(interpolation_para.dir_x == CW)
          interpolation_para.deviation += (2 * interpolation_para.startpoint_x + 1);
        else
          interpolation_para.deviation -= (2 * interpolation_para.startpoint_x + 1);
        break;
      case quadrant_2nd:/* 二四象限 */
      case quadrant_4th:
        interpolation_para.active_axis = y_axis;
        if(interpolation_para.dir_y == CW)
          interpolation_para.deviation += (2 * interpolation_para.startpoint_y + 1);
        else
          interpolation_para.deviation -= (2 * interpolation_para.startpoint_y + 1);
        break;
    }
  }
  /* 偏差<0，在圆弧内侧，应向圆外进给 */
  else
  {
    switch(interpolation_para.crood_pos)/* 判断当前象限 */
    {
      case quadrant_1st:
      case quadrant_3rd:
        interpolation_para.active_axis = y_axis;
        if(interpolation_para.dir_y == CW)
          interpolation_para.deviation += (2 * interpolation_para.startpoint_y + 1);
        else
          interpolation_para.deviation -= (2 * interpolation_para.startpoint_y + 1);
        break;
      case quadrant_2nd:
      case quadrant_4th:
        interpolation_para.active_axis = x_axis;
        if(interpolation_para.dir_x == CW)
          interpolation_para.deviation += (2 * interpolation_para.startpoint_x + 1);
        else
          interpolation_para.deviation -= (2 * interpolation_para.startpoint_x + 1);
        break;
    }
  }
  
  /* 下一步的活动轴与上一步的不一致时，需要换轴 */
  if(last_axis != interpolation_para.active_axis)
  {
    TIM_CCxChannelCmd(htim->Instance, step_motor[last_axis].pul_channel, TIM_CCx_DISABLE);
    TIM_CCxChannelCmd(htim->Instance, step_motor[interpolation_para.active_axis].pul_channel, TIM_CCx_ENABLE);
  }
  
  /* 进给总步数减1 */
  interpolation_para.endpoint_pulse--;
  
  /* 判断是否完成插补 */
  if(interpolation_para.endpoint_pulse == 0)
  {
    /* 关闭定时器 */
    TIM_CCxChannelCmd(htim->Instance, step_motor[last_axis].pul_channel, TIM_CCx_DISABLE);
    TIM_CCxChannelCmd(htim->Instance, step_motor[interpolation_para.active_axis].pul_channel, TIM_CCx_DISABLE);
    __HAL_TIM_MOE_DISABLE(htim);
    HAL_TIM_Base_Stop_IT(htim);
    interpolation_para.motionstatus = 0;
  }
}
