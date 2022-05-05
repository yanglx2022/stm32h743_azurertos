# 基于STM32H743+AzureRTOS的综合示例项目

STM32H743与AzureRTOS学习记录总结……

[2022/05/05] 创建项目、移植HAL库与ThreadX

## 硬件平台
---
正点原子STM32H743核心板+自制底板，具体硬件资源如下表
<table>
    <tr>
        <th></th>
        <th>项目</th>
        <th>型号</th>
        <th>详情</th>
    </tr>
    <tr>
        <td rowspan="5">核心板</td>
        <td>MCU</td>
        <td>STM32H743IIT6(V版)</td>
        <td>480MHz, 2MB Flash, 1060KB SRAM</td>
    </tr>
    <tr>
        <td>SDRAM</td>
        <td>W9825G6KH-6</td>
        <td>32MB</td>
    </tr>
    <tr>
        <td>SPI FLASH</td>
        <td>W25Q256</td>
        <td>32MB</td>
    </tr>
    <tr>
        <td>NAND FLASH</td>
        <td>MT29F4G08</td>
        <td>512MB</td>
    </tr>
    <tr>
        <td>EEPROM</td>
        <td>24C02</td>
        <td>256B</td>
    </tr>
    <tr>
        <td rowspan="1">触摸屏</td>
        <td>RGBLCD</td>
        <td>GT9147</td>
        <td>4.3"，24位RGB888/565，800*480，电容5点触控</td>
    </tr>
    <tr>
        <td rowspan="16">底板</td>
        <td>六轴传感器</td>
        <td>MPU-6050</td>
        <td></td>
    </tr>
    <tr>
        <td>触摸按键</td>
        <td></td>
        <td></td>
    </tr>
    <tr>
        <td>红外接收</td>
        <td>IRM-H638T/TR2</td>
        <td></td>
    </tr>
    <tr>
        <td>LCD屏幕(SPI)</td>
        <td>ST7789</td>
        <td>1.14"，240*135，RGB彩色</td>
    </tr>
    <tr>
        <td>OLED屏幕(IIC)</td>
        <td>SSD1306</td>
        <td>0.96"，128*64，单色</td>
    </tr>
    <tr>
        <td>SD卡</td>
        <td>TF卡</td>
        <td></td>
    </tr>
    <tr>
        <td>蜂鸣器</td>
        <td>MLT-7525</td>
        <td>无源，2.7kHz</td>
    </tr>
    <tr>
        <td>2.4G无线</td>
        <td>nRF24L01模块</td>
        <td></td>
    </tr>
    <tr>
        <td>纽扣电池</td>
        <td>RTC</td>
        <td>CR927锂电池</td>
    </tr>
    <tr>
        <td>PD快充</td>
        <td>Fusb302mpx</td>
        <td></td>
    </tr>
    <tr>
        <td>锂电池充放电管理</td>
        <td>BQ25606RGER</td>
        <td></td>
    </tr>
    <tr>
        <td>电池电量计</td>
        <td>LTC2941</td>
        <td>库仑计</td>
    </tr>
    <tr>
        <td>MP3</td>
        <td>VS1053B</td>
        <td>麦克风播放，耳机录音，I2S输出接口</td>
    </tr>
    <tr>
        <td>耳机(Type-C)</td>
        <td>TS5USBA224RSWR</td>
        <td>支持线控</td>
    </tr>
    <tr>
        <td>按键</td>
        <td></td>
        <td>开关机按键，音量加减按键</td>
    </tr>
    <tr>
        <td>摄像头</td>
        <td>OV5640</td>
        <td>2592x1944，500万像素，自动对焦，闪光灯</td>
    </tr>
</table>

## 开发环境
---
* 操作系统：Win10
* 编辑器：VSCode
    * 在`.vscode/tasks.json`中配置 <kbd>编译</kbd> <kbd>清理</kbd> <kbd>下载</kbd> 三个任务（任务列表快捷键 <kbd>Ctrl</kbd>+<kbd>Shift</kbd>+<kbd>B</kbd> ），分别用于编译(make)、清理(make clean)和烧录固件到单片机（通过ST-LINK）
    * 在`.vscode/launch.json`中配置ST-LINK调试，以便连接ST-LINK进行在线调试
    * 用到的插件及用途
        * C/C++：代码提示跳转等
        * Cortex-Debug: 配合openocd实现调试
        * 非必须：
            * koroFileHeader: 生成格式化注释
            * Git Graph: 显示git图
            * Todo Tree: 汇总显示TODO
* 交叉编译工具：gcc-arm-none-eabi
    * [下载](https://developer.arm.com/tools-and-software/open-source-software/developer-tools/gnu-toolchain/gnu-rm/downloads)ZIP包后解压即可，`Makefile`、`.vscode/c_cpp_properties.json`及`.vscode/launch.json`中根据此路径配置
* 构建工具：mingw32-make
    * [下载](https://sourceforge.net/projects/mingw-w64/files/)x86_64-posix-seh后解压即可，`.vscode/tasks.json`中根据此路径配置
* 在线编程调试：OpenOCD
    * [下载](https://github.com/openocd-org/openocd/releases)解压即可，具体使用配置见`.vscode/tasks.json`与`.vscode/launch.json`
* 调试器(硬件)：ST-LINK V2

## 项目结构
---
* drivers
    * CMSIS------------------------------M7内核相关、启动文件、链接文件等
    * STM32H7xx_HAL_Driver-----------HAL、LL库源码
    * threadx-6.1.11_rel-----------------ThreadX源码
    * STM32H743x.svd------------------寄存器描述文件, 用于调试时查看CPU及外设寄存器
* user
    * app---------------------------------业务代码
    * hardware---------------------------外设驱动
* Makefile

## [STM32CubeH7库](https://github.com/STMicroelectronics/STM32CubeH7)
---
主要使用LL库，无独立LL库时使用HAL库（SD、LTDC、NAND），未使用STM32CubeMX的代码框架

移植关键步骤
* 添加源码（[STM32CubeH7](https://github.com/STMicroelectronics/STM32CubeH7)）
    * `Drivers/STM32H7xx_HAL_Driver/`
    * `Drivers/CMSIS/Include/`
    * `Drivers/CMSIS/Device/ST/STM32H7xx/`中H743相关代码（其中ld文件从`Projects/STM32H743I-EVAL/Templates/SW4STM32/STM32H743I_EVAL/STM32H743XIHx_FLASH_DTCMRAM.ld`拷贝）
* 拷贝`drivers/STM32H7xx_HAL_Driver/Inc/stm32h7xx_hal_conf_template.h`重命名至`user/hardware/stm32h7xx_hal_conf.h`，编辑文件屏蔽不需要的模块
* Makefile中需要添加的宏定义
    * `USE_FULL_LL_DRIVER`
    * `USE_HAL_DRIVER`
    * `STM32H743xx`
* 注意Makefile中使用通配添加所有HAL及LL库源码时需要排除`_template.c`后缀的文件

## [ThreadX](https://github.com/azure-rtos/threadx)
---
移植关键步骤（包括任务统计分析组件execution_profile, 用于统计CPU占用率）
* 添加源码（[ThreadX](https://github.com/azure-rtos/threadx/releases)）
    * `common_modules`、`common_smp`、`ports_module`、`ports_smp`不需要
    * `ports`只需要`cortex_m7/gnu`
    * 其他所有
* 拷贝`ports/cortex_m7/gnu/example_build/tx_initialize_low_level.S`至`user/threadx/tx_initialize_low_level.S`并修改
    * 设置RTOS节拍
    * 此示例文件中包含一些CM7启动的配置(设置堆栈、中断向量表、一些中断处理函)，但是又不完整，因此仍旧使用ARM官方的启动文件，而将这些冲突的代码注释掉
* 拷贝`common/inc/tx_user_sample.h`重命名至`user/threadx/tx_user.h`，在此文件中对ThreadX进行配置
* Makefile中需要添加的宏定义
    * `TX_INCLUDE_USER_DEFINE_FILE`：使能`tx_user.h`配置文件
    * `TX_EXECUTION_PROFILE_ENABLE`、`TX_CORTEX_M_EPK`：使能统计分析组件
    * `TX_EXECUTION_PROFILE_ENABLE`(汇编)：使能统计分析组件


