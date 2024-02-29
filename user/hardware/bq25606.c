/**
 * @brief   : BQ25606电池充放电管理
 * @author  : yanglx2022
 * @version : V0.1.0
 * @date    : 2024-02-29
 * @note    : 
 */

#include "bq25606.h"

// PG14 STAT(充电状态)
#define CHARGE_STATE_GPIO               GPIOG
#define CHARGE_STATE_PIN                LL_GPIO_PIN_14

// // PA4 PG#(PowerGood)
// #define POWER_GOOD_GPIO                 GPIOA
// #define POWER_GOOD_PIN                  LL_GPIO_PIN_4


void BQ25606_Init(void)
{

}

