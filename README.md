# 基于STM32H743+AzureRTOS的综合示例项目

STM32H743与AzureRTOS学习记录总结……

[2022/05/05] 创建项目、移植HAL库与ThreadX

[2022/05/06] SDRAM驱动

[2022/05/08] 配置AXIRAM、SDRAM的data与bss段，并实现初始化

[2022/08/19] SD卡驱动，FileX文件系统移植(中文支持)

[2023/07/27] NandFlash驱动

[2023/08/07] ECC介绍、LevelX移植、FileX增加NandFlash

## 硬件平台
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
* 操作系统：Win10
* 编辑器：VSCode
    * 在`.vscode/tasks.json`中配置 <kbd>编译</kbd> <kbd>清理</kbd> <kbd>下载</kbd> 三个任务（任务列表快捷键 <kbd>Ctrl</kbd>+<kbd>Shift</kbd>+<kbd>B</kbd> ），分别用于编译(make)、清理(make clean)和烧录固件到单片机（通过CMSIS-DAP）
    * 在`.vscode/launch.json`中配置CMSIS-DAP调试，以便连接CMSIS-DAP进行在线调试
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
* 调试器(硬件)：WCH-Link

## 项目结构
* drivers
    * CMSIS------------------------------M7内核相关、启动文件、链接文件等
    * STM32H7xx_HAL_Driver-----------HAL、LL库源码
    * threadx-6.1.11_rel-----------------ThreadX源码
    * STM32H743x.svd------------------寄存器描述文件, 用于调试时查看CPU及外设寄存器
* user
    * app---------------------------------业务代码
    * hardware---------------------------外设驱动
* Makefile

## STM32CubeH7库
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

## ThreadX
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

## 内存使用
### 链接段介绍
内存映射及主要的段如下图所示

![内存映射](docs/memory_map.png)

编译工程输出的代码空间占用信息如下所示
```
arm-none-eabi-size build/debug/stm32h743_azurertos.elf
   text    data     bss     dec     hex filename
  23800    2536    5504   31840    7c60 build/debug/stm32h743_azurertos.elf
```
其中各项的的含义：

项目|包含的段|存储位置|说明
-|-|-|-
text|中断向量表、代码、常量等|FLASH|
data|非零初值的全局变量等|FLASH RAM|复位后由FLASH拷贝至RAM
bss|无初值/零初值的全局变量、堆、栈等|RAM|bss全局变量复位后赋初值0
dec/hex|text + data + bss||hex为16进制值

一般情况下
* RAM占用 = data + bss
* FLASH占用 = text + data（生成的bin文件）

但当text或data中有不连续的段时，FLASH占用/bin文件会大一些，因为text与data大小都是统计各自包含的段的大小之和，而bin文件为FALSH起始至data末尾，包含了不连续的部分

> 以上统计信息由命令`arm-none-eabi-size xxx.elf`生成（Makefile中配置的），默认为`-B`选项产生单行统计输出，还可以使用`-A`选项选择更详细的输出，此外`-o -d -x`选项可以选择输出数据的进制，下面的示例中可以看出`.isr_vector`与`.text`并不连续，这种情况就会造成生成的bin文件比text+data更大
```
> arm-none-eabi-size -Ax build/debug/stm32h743_azurertos.elf
build/debug/stm32h743_azurertos.elf  :
section                size         addr
.isr_vector           0x298    0x8000000
.text                0x54ac    0x80002a0
.rodata               0x5ac    0x8005750
.ARM                    0x8    0x8005cfc
.init_array             0x8    0x8005d04
.fini_array             0x4    0x8005d0c
.data                 0x1dc   0x20000000
.bss                  0xf7c   0x200001e0
._user_heap_stack     0x604   0x2000115c
.init.AXI_RAM         0x400   0x24000000
.noinit.AXI_RAM         0x0   0x24000400
.init.SDRAM           0x400   0xc0000000
.noinit.SDRAM           0x0   0xc0000400
.ARM.attributes        0x2e          0x0
.debug_info         0x1479e          0x0
.debug_abbrev        0x353e          0x0
.debug_loc           0x71ac          0x0
.debug_aranges        0x6a0          0x0
.debug_ranges         0x7b0          0x0
.debug_line          0x8447          0x0
.debug_str           0x46ef          0x0
.comment               0x49          0x0
.debug_frame         0x1ef0          0x0
.stab                  0xb4          0x0
.stabstr              0x183          0x0
Total               0x3660c
```
> 还可以直接查看map文件的`Linker script and memory map`部分，里面有对段最详细的描述，部分示例如下（中间有省略），每个函数或者变量等所在段的详细情况都可查找到
```
...
Linker script and memory map

LOAD c:/gcc-arm-none-eabi-10.3-2021.10/bin/../lib/gcc/arm-none-eabi/10.3.1/thumb/v7e-m+dp/hard/crti.o
...
...
                0x20020000                _estack = 0x20020000
                0x00000200                _Min_Heap_Size = 0x200
                0x00000400                _Min_Stack_Size = 0x400

.isr_vector     0x08000000      0x298
                0x08000000                . = ALIGN (0x4)
 *(.isr_vector)
 .isr_vector    0x08000000      0x298 build/debug/startup_stm32h743xx.o
                0x08000000                g_pfnVectors
                0x08000298                . = ALIGN (0x4)

.text           0x080002a0     0x54ac
                0x080002a0                . = ALIGN (0x4)
...
...
.fini_array    0x08005d0c        0x4 c:/gcc-arm-none-eabi-10.3-2021.10/bin/../lib/gcc/arm-none-eabi/10.3.1/thumb/v7e-m+dp/hard/crtbegin.o
                [!provide]                PROVIDE (__fini_array_end = .)
                0x08005d10                _sidata = LOADADDR (.data)

.data           0x20000000      0x1dc load address 0x08005d10
                0x20000000                . = ALIGN (0x4)
                0x20000000                _sdata = .
 *(.data)
 *(.data*)
 .data.SystemCoreClock
                0x20000000        0x4 build/debug/system_stm32h7xx.o
                0x20000000                SystemCoreClock
...
```

### 配置AXIRAM与SDRAM段
H743内部包含多块内存，其中128K的DTCM与CPU同频，因此选作主内存，默认全局变量以及堆栈都在此块内存中（ld文件中的`.data`、`.bss`与`._user_heap_stack`段都设置在DTCMRAM）。

当需要较大的缓冲区时，可使用512K的AXIRAM(RAM_D1)，更大的内存需求，比如GUI显存则需要外部SDRAM。对于这种内存区域最直接的使用方式就是根据内存地址直接去访问，但这样需要自己管理内存，不是很方便，另一方面编译器无法得知使用情况，也就无法检测到内存用超等情况。

可以在ld文件中定义相应的段，然后定义变量时只需指定段名称，如下所示，这样就让编译器去管理内存使用了。当然也可以直接使用动态申请内存的方式去管理，本项目主要为静态使用。
```
uint8_t axi_data[1024] __attribute__ ((section (".noinit.AXI_RAM")));
```
下面以SDRAM为例介绍ld文件中段的配置以及变量初始化，AXIRAM类似。

首先在[ld文件](drivers/CMSIS/Device/ST/STM32H7xx/Source/Templates/gcc/linker/STM32H743IITx.ld)中添加SDRAM内存区域描述
```
MEMORY
{
DTCMRAM (xrw)   : ORIGIN = 0x20000000, LENGTH = 128K
RAM_D1  (xrw)   : ORIGIN = 0x24000000, LENGTH = 512K
RAM_D2  (xrw)   : ORIGIN = 0x30000000, LENGTH = 288K
RAM_D3  (xrw)   : ORIGIN = 0x38000000, LENGTH = 64K
ITCMRAM (xrw)   : ORIGIN = 0x00000000, LENGTH = 64K
FLASH   (rx)    : ORIGIN = 0x08000000, LENGTH = 2048K
SDRAM   (rw)    : ORIGIN = 0xC0000000, LENGTH = 32M
}
```
然后添加有初值与无初值两个段
```
  /* 有初值的SDRAM段(data) */
  .init.SDRAM :
  {
    . = ALIGN(4);
    _init_sdram_start = .;        /* 段起始地址, 4字节对齐, 从FLASH拷贝初值时使用 */
    *(.init.SDRAM)
    . = ALIGN(4);
    _init_sdram_end = .;          /* 段结束地址, 4字节对齐, 从FLASH拷贝初值时使用 */
  } > SDRAM AT> FLASH
  _init_sdram_lma = LOADADDR(.init.SDRAM);  /* 段在FLASH中的地址, 从FLASH拷贝初值时使用 */

  /* 无初值的SDRAM段(bss) */
  .noinit.SDRAM :
  {
    . = ALIGN(4);
    _noinit_sdram_start = .;    /* 段起始地址, 4字节对齐, 赋0初值时使用 */
    *(.noinit.SDRAM)
    . = ALIGN(4);
    _noinit_sdram_end = .;      /* 段结束地址, 4字节对齐, 赋0初值时使用 */
  } > SDRAM
```
要点：
* 段名可以自定义，但需注意是否与已有的段名有冲突，如果有冲突需要放到冲突段的前面。举个例子：假设无初值的段命名为`.bss.SDRAM`，且在`.bss*`段的后面定义，那么定义变量时段名解析会先匹配到`.bss*`从而造成链接到错误的位置，因此这种情况下需要将`.bss.SDRAM`放到前面以优先匹配。
* 有初值的段初值需要放到FLASH中，因此需要`AT> FLASH`来指定LMA（加载内存地址）为FLASH
* 有初值段需要记下起始地址、结束地址与加载地址（FLASH地址），以便复位时根据这三个地址将初值从FLASH拷贝到RAM
* 无初值段需要记下起始地址与结束地址，以便复位时赋0值

赋初值

对于默认的有初值全局变量（`.data`段）、无初值/0初值全局变量（`.bss`段），复位后进入main函数前完成初值的赋值，赋值操作具体是在[启动文件](drivers/CMSIS/Device/ST/STM32H7xx/Source/Templates/gcc/startup_stm32h743xx.s)的Reset_Handler中实现的，可以看出先调用了SystemInit，然后是从FLASH中拷贝初值到SRAM，然后是bss段赋0初值，最后进入main函数
```
Reset_Handler:
  ldr   sp, =_estack      /* set stack pointer */

/* Call the clock system initialization function.*/
  bl  SystemInit

/* Copy the data segment initializers from flash to SRAM */
  ldr r0, =_sdata
  ldr r1, =_edata
  ldr r2, =_sidata
  movs r3, #0
  b LoopCopyDataInit

CopyDataInit:
  ldr r4, [r2, r3]
  str r4, [r0, r3]
  adds r3, r3, #4

LoopCopyDataInit:
  adds r4, r0, r3
  cmp r4, r1
  bcc CopyDataInit
/* Zero fill the bss segment. */
  ldr r2, =_sbss
  ldr r4, =_ebss
  movs r3, #0
  b LoopFillZerobss

FillZerobss:
  str  r3, [r2]
  adds r2, r2, #4

LoopFillZerobss:
  cmp r2, r4
  bcc FillZerobss

/* Call static constructors */
    bl __libc_init_array
/* Call the application's entry point.*/
  bl  main
  bx  lr
.size  Reset_Handler, .-Reset_Handler
```

对于SDRAM的两个段，也可以依葫芦画瓢在Reset_Handler中这样处理，但还有一个问题是拷贝初值之前要先完成SDRAM硬件的初始化，这个操作可以放在SystemInit中实现。但这种方案一是代码比较分散，二是需要写汇编，不是很友好。

观察上面的启动文件Reset_Handler代码，可以发现进入main函数前还调用了一个__libc_init_array，这是另一种方案的关键。

> `__libc_init_array`会依次调用一个函数列表中的函数，通过`constructor`属性可以将一个函数加入到上述函数列表，此外还可以设置优先级以决定调用顺序。

因此定义一个`constructor`属性的函数，然后SDRAM硬件的初始化，变量的初始化都可以放到此函数中，甚至所有硬件初始化都可以放到此函数中，这样main函数就直接处理业务了。

SDRAM段中记录的起始终止地址在启动文件汇编代码中通过`ldr`指令进行读取，在c代码中不能直接引用。由于其不是普通的变量只是ld文件中定义的符号，因此使用有些特别，如下所示，注意即使符号本身就是地址值，但仍旧需要使用`&`取地址符。
```
extern uint32_t _init_axi_ram_start;
uint32_t *ram = &_init_axi_ram_start;
```
SDRAM完整初始化[示例](user/hardware/common.c)如下
```
__attribute__((constructor)) void sys_init(void)
{
    // MCU初始化
    // 省略···

    // SDRAM初始化
    SDRAM_Init();

    // SDRAM数据初始化
    // 从FLASH拷贝
    extern uint32_t _init_sdram_start;
    extern uint32_t _init_sdram_end;
    extern uint32_t _init_sdram_lma;
    ram   = &_init_sdram_start;
    end   = &_init_sdram_end;
    flash = &_init_sdram_lma;
    while(ram < end)
    {
        *ram++ = *flash++;
    }
    // 赋0值
    extern uint32_t _noinit_sdram_start;
    extern uint32_t _noinit_sdram_end;
    ram = &_noinit_sdram_start;
    end = &_noinit_sdram_end;
    while(ram < end)
    {
        *ram++ = 0;
    }
}
```

## FileX文件系统
### 移植关键步骤
* 添加源码（[FileX](https://github.com/azure-rtos/filex/releases)）
    * `ports`只需要`cortex_m7/gnu`
    * 其他所有
* 拷贝`common/inc/fx_user_sample.h`重命名至`user/filex/fx_user.h`，在此文件中对FileX进行配置
* Makefile中需要添加的宏定义
    * `FX_INCLUDE_USER_DEFINE_FILE`：使能`fx_user.h`配置文件
* 底层驱动
    * 读写扇区等操作，详见`user/filex/fx_sdcard_driver.c`等

### 中文支持
FileX多语言支持使用双字节Unicode编码（UTF-16的双字节部分），下面对字符集与字符编码进行简单介绍

> 给每一个字符编一个编号（code point），这些字符的集合构成`字符集`（Charset）。对于一个字符，在计算机中实际存储的值为`字符编码`（Encoding）。字符编码可能直接等于字符集中字符的编号，也可能是与字符编号有一个映射关系，这与具体的字符集相关。例如GB2312即是字符集也是其字符编码，而UTF-8则是[Unicode](https://unicode-table.com/cn/)字符集的一种字符编码。

<table>
    <tr>
        <th>字符集</th>
        <th>字符编码</th>
        <th>字符宽度</th>
        <th>说明</th>
    </tr>
    <tr>
        <td rowspan="1">ASCII</td>
        <td>ASCII</td>
        <td>1字节</td>
        <td>128个字符</td>
    </tr>
    <tr>
        <td rowspan="3">Unicode</td>
        <td>UTF-8</td>
        <td>1~4字节</td>
        <td>互联网应用最广泛，可容纳200多万个字符</td>
    </tr>
    <tr>
        <td>UTF-16</td>
        <td>2/4字节</td>
        <td>Unicode第一平面使用2个字节，其他4个字节</td>
    </tr>
    <tr>
        <td>UTF-32</td>
        <td>4字节</td>
        <td>对Unicode字符直接编码</td>
    </tr>
    <tr>
        <td rowspan="1">GB2312</td>
        <td>GB2312</td>
        <td>2字节</td>
        <td>第一个汉字编码国标，收录汉字6763个</td>
    </tr>
    <tr>
        <td rowspan="1">GBK</td>
        <td>GBK</td>
        <td>2字节</td>
        <td>兼容GB2312并进行扩展</td>
    </tr>
    <tr>
        <td rowspan="1">GB18030</td>
        <td>GB18030</td>
        <td>1/2/4字节</td>
        <td>兼容GB2312与GBK并进行扩展</td>
    </tr>
</table>

中文文本显示时多采用UTF-8或GBK等编码，因此需要进行UTF-16到相应编码的转换。本项目使用UTF-8编码，下面介绍UTF-8的编码规则。
> 对于单字节编码的字符，字节最高位为0，其他7位为Unicode码
>
> 对于N字节编码的字符，第一个字节前N位都为1，第N+1位为0，后面的字节最高两位均为10，所有字节剩余的位拼到一起为Unicode码

1~4字节编码汇总如下表所示

字节数|编码格式|有效位数|Unicode范围
-|:-|:-:|:-
1个字节|0xxx_xxxx|7|00~7F (2^7-1)
2个字节|110x_xxxx 10xx_xxxx|11|80~7FF (2^11-1)
3个字节|1110_xxxx 10xx_xxxx 10xx_xxxx|16|800~FFFF (2^16-1)
4个字节|1111_0xxx 10xx_xxxx 10xx_xxxx 10xx_xxxx|21|10000~1FFFFF (2^21-1)

由上表可得UTF-16转UTF-8算法：根据字符UTF-16值的范围确定UTF-8编码字节数，再根据相应编码格式完成转换。详细算法见文件`user/app/thread_file.c`中的`utf16_to_utf8`函数。

> 以`"中"`字为例说明一下字节序的问题。
> 
> 在[查看字符编码(UTF-8)](http://www.mytju.com/classcode/tools/encode_utf8.asp)网站可以查到`"中"`的Unicode值为`4E2D`，UTF-8编码为`E4B8AD`。其中`4E2D`为一个16位数值，低字节为`2D`高字节为`4E`，实际在UTF-16字符串中`2D`在前`4E`在后（小端）。而`E4B8AD`不是一个数值而是三个字节的序列，UTF-8字符串中`E4`在最前。UTF-16转UTF-8时，从UTF-16的低字节低位开始填充UTF-8的最后一个字节(10xx_xxxx)，最后UTF-16高字节的剩余部分填充UTF-8的第一个字节(1110_xxxx)。

UTF-16转GBK的话没有固定的规律，需要一张包含每个字符的Unicode到GBK映射的转换表。转GB2312等其他中文编码也是一样的原理。

## FMC接口驱动NandFlash
### NandFlash介绍
本文使用的NandFlash型号为H27U4G8F2E，这是一块海力士的8bit×512M的NandFlash，具体参数特性如下
* 尺寸参数
    * 2个plane
    * 每个plane 2048个block
    * 每个block 64个page
    * 每个page 2048字节（data area） + 64字节（spare area）
* 典型时间特性
    * 串行读：25ns/字节
    * 页访问：25us
    * 页编程：200us
    * 块擦除：3.5ms
* 最小擦除单位是block，擦除后全1，写操作只能写0不能写1
* SLC单层单元
* 兼容Open NAND Flash Interface([ONFI](http://www.onfi.org/)) 1.0
* 支持内部数据拷贝功能(copy-back)
* 2个plane并行操作
* 10万次擦写编程循环

### 引脚定义与功能
引脚|说明
-|-
DQ0-DQ7|数据输入输出，命令、地址、数据复用                                                                                                                |
CLE|命令锁存使能，高电平有效表示DQ为命令
ALE|地址锁存使能，高电平有效表示DQ为地址
CE#|芯片使能，低电平有效
RE#|读使能，下降沿后tREA数据有效
WE#|写使能，上升沿时锁存命令(CLE有效)、地址(ALE有效)或数据(CLE与ALE均无效)
WP#|写保护，低电平有效禁止擦写
RB#|就绪/忙，开漏输出指示状态，可通过命令读取状态而不使用此信号


### 总线操作
共有6种标准总线操作如下表所示，对FLASH的读写擦除等操作由其中前4种的一种或几种操作构成。
操作|说明
-|-
命令输入|在CE#低、CLE高、ALE低、RE#高时，在WE#的上升沿锁存
地址输入|需要5个周期，在CE#低、ALE高、CLE低、RE#高时，在WE#的上升沿锁存
数据输入|WE#控制下串行输入，在CE#低、ALE低、CLE低、RE#高时，在WE#的上升沿锁存
数据输出|在CE#低、ALE低、CLE低、WE#高时，按照RE#的周期串行移出数据
写保护|WP#低电平时硬件写保护被激活，此时存储器的内容无法更改
休眠|standby状态，设备被取消选择，输出禁用，功耗降低

### 存储器地址
512MB空间需要29位地址，加上spare area则需要再多一位共30位地址，记为A0-A29，其中地址位定义如下表所示
地址位|定义|说明
-|-|-
A0-A11|列地址，即页内的字节地址|寻址2KB+64B，12位最大可寻址4KB，由此可知页末尾地址与相邻下一页起始地址并不连续
A12-A17|块内的页索引|6位地址寻址64，对应一块包含64页
|A18|plane地址|plane地址位于块地址的低位，由此可知block按地址排序则会在两个plane中逐个交替
|A19-A29|块地址|也可定义A18-A29为块地址，则12位地址寻址4096对应两个plane一共4096个block

与A0-A11为列地址（Column Address）相对应，A12-A29称为行地址（Row Address），这30位地址分5个周期传输，排列如下表所示
周期|DQ7|DQ6|DQ5|DQ4|DQ3|DQ2|DQ1|DQ0
-|-|-|-|-|-|-|-|-
第1周期|A7|A6|A5|A4|A3|A2|A1|A0|
第2周期|0|0|0|0|A11|A10|A9|A8|
第3周期|A19|A18|A17|A16|A15|A14|A13|A12|
第4周期|A27|A26|A25|A24|A23|A22|A21|A20|
第5周期|0|0|0|0|0|0|A29|A28|

### FMC控制器
我们写驱动代码操作NandFlash时并不要需要直接去实现手册里的那些读写时序，而是通过配置与操作单片机的NandFlash控制器来实现这些时序，这样能大大简化代码工作并节省CPU资源。对于H7来说即FMC控制器，即我们要写的NandFlash驱动其实就是FMC驱动。下面介绍怎样根据手册中的时序去配置FMC中的参数。

FMC中时序相关的参数有6个，包括下图中的4个（MEMSET、MEMWAIT、MEMHOLD、MEMHIZ）以及TCLR与TAR，注意下图中命名与FLASH手册命名不同，红字标出了FLASH手册中对应的名称

![FMC NAND Flash控制器时序](docs/fmc_nand_waveforms.png)

以命令锁存时序为例进行说明，下图为NANDFlash手册中的命令锁存时序

![命令锁存时序](docs/command_latch_cycle.png)

* **MEMSET**为命令使能前地址建立的时间，即CLE或ALE变高至WE#或RE#变低的时间。对于上图中的命令锁存即为tCLS-tWP，但手册中只给出了tCLS的最小值12ns与tWP的最小值12ns，两个最小值无法得到差的最小值。也有文章直接将tCLS及tALS当作此建立时间考虑。本文实测最小可配置为5ns，即MEMSET=0。


* **MEMWAIT**为命令使能的最小持续时间，即WE#或RE#的低电平时间（tWP与tRP）。对于上图中的命令锁存即为tWP，手册中最小值为12ns。tRP最小值同样为12ns，FMC时钟周期为5ns，因此理论最小可设3个周期15ns。但本文实测最小需配置25ns，即MEMWAIT=4。

* **MEMHOLD**为命令结束后，地址信号（写的话还有数据信号）的保持时间，即WE#或RE#变高至CLE或ALE变低的时间（tCLH与tALH）。对于上图中的命令锁存即为tCLH，手册中最小值为5ns。tALH最小值同样为5ns，FMC时钟周期为5ns，因此理论最小可设1个周期5ns，保险起见设置２个周期10ns。由于写访问保持时间为MEMHOLD而读访问保持时间为MEMHOLD+1，因此设置MEMHOLD=2，此时写访问10ns读访问15ns。

* **MEMHIZ**为开始写访问后数据总线高阻态时间，即CLE或ALE变低至IO上数据有效的时间。对于上图中的命令锁存即为tALS-tDS，但手册中只给出了tALS的最小值12ns与tDS的最小值12ns，两个最小值无法得到差的最小值。不过这个时间与MEMSET类似，也以尝试取与MEMSET一样的值。本文实测最小可配置为5ns，即MEMHIZ=0。

* **TCLR**配置CLE低电平到RE#低电平的时间，即FLASH手册中的tCLR（CLE to RE# Delay）。计算公式为tCLR=(TCLR+MEMSET+2)*T（T=5ns），手册中tCLR最小值为10ns，则MEMSET=0时，TCLR最小值为0，保险起见可设置TCLR=1。

* **TAR**配置ALE低电平到RE#低电平的时间，即FLASH手册中的tAR（ALE to RE# Delay）。计算公式为tAR=(TAR+MEMSET+2)*T（T=5ns），手册中tAR最小值为10ns，则MEMSET=0时，TAR最小值为0，保险起见可设置TAR=1。

### 读状态
本文以最简单的读状态来介绍操作时序，读状态时序如下图

![读状态时序](docs/status_read_cycle.png)

上述时序看起来复杂但实现却很简单，可主要分为写命令字和读状态值两个部分，写命令只需将命令字写入FMC命令区域任意地址即可，读状态结果则是读取FMC数据区域任意地址即可，具体的命令与数据时序由FMC根据初始化时的配置自动完成。

注意时序中发完命令至读取状态结果有一个tWHR的时间，这个时间FMC的配置中并没有，但手册中有最小60ns的要求。观察时序图可以看出tWHR=tCLH+tCLR，FMC中tCLH与tCLR配置均为15ns，则tWHR=30ns<60ns并不满足时序要求，需要在写命令与读状态之间增加额外至少30ns的延时。但实测发现不增加此延时也能正常读到状态值，使用示波器测量此时实际的tWHR值为51.2ns（下图是测量结果，使用的正点原子DS100示波器，由于带宽不足波形已经严重失真，这个时间只能大致参考，图中黄色为RE#、绿色为WE#）。这里实际与手册的差异暂没搞清楚怎么回事，保险起见代码里直接在写指令与读数据之间增加60ns的延时。

![黄色为RE# 绿色为WE#](docs/twhr.png)

经过以上分析最终读状态操作步骤如下：
* 写入命令0x70
* 延时tWHR
* 读状态值

> 注意进入读状态模式后，除非有新的指令，否则将一直处于该模式。因此在读页操作中，读取状态后需要发送一个0x00指令再开始读取页数据。

### 复位

![复位时序](docs/reset_operation_timing.png)

复位操作步骤：
* 写入命令0xFF
* 延时tWB
* 循环读ready状态（tRST超时）

### 读页

![读页时序](docs/read_operation_timing.png)

读页操作步骤：
* 写入命令0x00
* 写入地址
* 写入命令0x30
* 延时tWB
* 循环读ready状态（tR-tWB超时）
* 写入命令0x00
* 读数据

> 关于循环读ready状态的超时时间：
按照时序busy状态持续时间为tR-tWB，可近似为tR（tWB与tR相比很小），手册中tR最大值为25us，因此理论上超时时间设置为25us即可，但实际25us总是发生超时，示波器测量RB#电平时间发现大于25us，如下图所示（绿色为RB#信号），因此超时时间要设置长一些，代码中设置的为50us
![绿色为RB#信号](docs/tr-twb.png)

> 关于写入命令0x00后的延时
读数据之前的时然序有tRR与tCLR。由于ready状态（RB#升沿）之后有一个写0x00命令的操作，因此tRR（Min=20ns）必满足。而tCLR是由FMC自动控制的，因此理论上写入命令0x00之后代码无需延时即可读取数据。用示波器测量tCLR结果如下，87.5-72.6=15ns与配置值一致，满足要求。由于示波器只有两路，因此CLE与RE#分两次测量，图中黄色为便于测试控制GPIO输出的基准信号，左图绿色为CLE信号，右图绿色为CE#信号，两个信号的时间差即为tCLR。
![tCLR](docs/tclr.png)

### 写页

![写页时序](docs/page_program_operation.png)

写页操作步骤：
* 写入命令0x80
* 写入地址
* 延时tADL
* 写入数据
* 写入命令0x10
* 延时tWB
* 循环读ready状态（tPROG超时）

### 擦除块

![擦除块时序](docs/block_erase_operation.png)

擦除块操作步骤：
* 写入命令0x60
* 写入地址
* 写入命令0xD0
* 延时tWB
* 循环读ready状态（tBERS超时）

> 本文未介绍的如copy-back、CE don't-care、多plane并行等一些高级特性可参考[此文](https://www.cnblogs.com/yuanqiangfei/p/9400435.html)。

### ECC校验

**ECC简介**

与NorFlash相比，NandFlash具有成本低容量大的优点，但同时也具有数据读写更容易出错的缺点，所以一般都需要ECC数据校验措施，每页最后的spare area区域用途之一就是存放ECC校验值。

ECC（Error Code Correction或者Error Checking and Correcting）是一种差错检测和修正算法，可以检测特定长度（2的整次幂）数据中2个bit以内的错误和修正1个bit的错误。

**ECC校验值生成原理**

将数据按位排列（假设共Nbit），以不同的步长（1bit、2bit、4bit、8bit、16bit、…、N/2bit）交替将数据等分为两个部分，将每个部分所有位进行异或即得到ECC结果中的一位。由此可知ECC校验值位数为数据二等分方式数的2倍，即ECC有效位数=2log2(数据总位数），例如256字节数据的有效ECC位数为2log2(256*8)=22bit。下图展示了2字节数据的ECC计算原理。

![ECC校验](docs/ecc.png)

实现上述算法时一般会将数据按照一个字节或者两个字节一行排列成一个表，然后分别计算行校验与列校验，最后将行列校验组合起来形成ECC校验值（上面的ECC原理介绍相当于只有一行的表）。这样做的好处是方便加快计算速度，以一个字节一行为例，可以提前计算值0x00~0xFF的行列极性组成极性表，然后结合查表一次循环（按字节）即可计算出所有行列校验值，Linux内核中ECC计算便是使用此算法，[算法实现详细介绍见此文]( https://www.pianshen.com/article/2306657069/)。LevelX中的ECC算法使用的两个字节一行的算法，没有使用预计算的极性表（一个字节时极性表大小为256字节，两个字节的表则需要64KB，对存储占用太大）。

无论是哪种实现本质是一样的，因此计算得到的ECC结果必然也一样，但是注意不同的算法的ECC值内部bit可能有不同的排列方式，因此比较不同算法结果的话需要统一排列方式。

**硬件ECC计算**

H7的FMC控制器包含ECC硬件计算模块，支持每256、512、1K、2K、4K、8K个字节计算ECC。开启此功能后，FMC自动对读写的数据流进行ECC计算，结果存储在相应寄存器中。因此实际本项目代码中不用实现上述ECC生成算法，而是控制ECC硬件模块完成ECC计算。

本项目采用每256字节计算ECC，这样能应对最多的出错情况，另一方面ECC占用存储也最多。不过对于2K+64的页：2K/256*3字节（ECC有效22位）=24字节，还不到64字节的一半，因此存储空间完全没问题。由于每256字节计算ECC，对于2KB的页读写时需要分8次进行以获取每个256字节数据的ECC。

硬件ECC使用步骤如下，每页需要重复以下步骤8次
* 启动ECC
* 读/写256字节数据
* 阻塞读取ECC结果
* 关闭ECC

**ECC使用（纠错）**

写数据时计算ECC并写入spare area，读数据时计算ECC并和从spare area读出的ECC进行比较，根据比较结果完成纠错。和ECC计算一样，纠错算法也有不同的实现方式，本质上都是上面链接文章最后提到的二分判决过程。本项目使用下述方式（要保证ECC内部bit排列是上图示例中的排列方式）：
* 新旧ECC相等的话说明没有错误，否则二者进行异或，注意ECC有效位数
* 异或结果的bit0~1、bit2~3...以此类推全是01b或10b的话说明有1个bit翻转，异或结果奇数bit组成的数即为翻转bit的总索引（全部数据按位排列的位索引）
* 其他情况则表示无法纠正的错误

## LevelX闪存磨损均衡
最新版LevelX-6.2.1相比之前有很大的变化，但是相关文档并没有更新，本项目暂时仍旧使用旧版的LeveX-6.1.11

### 移植关键步骤
* 添加源码（[LevelX](https://github.com/azure-rtos/levelx/releases/tag/v6.1.11_rel)）
    * 注意删除或者在Makefile中排除源文件中的2个文件系统示例文件`common/src/fx_*.c`
* 拷贝`common/inc/lx_user_sample.h`重命名至`user/levelx/lx_user.h`，在此文件中对LevelX进行配置
* Makefile中需要添加的宏定义
    * `LX_INCLUDE_USER_DEFINE_FILE`：使能`lx_user.h`配置文件
* 底层驱动
    * NandFlash页读写及块擦除等操作，详见`user/levelx/lx_nand_driver.c`

### 配置
* 需要开启直接映射缓存选项（`lx_user.h`中定义`LX_NAND_FLASH_DIRECT_MAPPING_CACHE`宏）, 否则写未映射的sector会耗时数百ms(因为每次_lx_nand_flash_logical_sector_find会遍历全部block)
* 开启了直接映射缓存则需要设置Cache大小为Flash总页数（`lx_user.h`中配置`LX_NAND_SECTOR_MAPPING_CACHE_SIZE`宏）
* 以上配置会造成LX_NAND_FLASH实例很大（>1MB），因此需要定义到SDRAM中
* 为了读写页数据之后不用再发送地址而直接能够继续访问ECC，NandFlash驱动中ECC值放在了spare area的起始位置，因此LevelX中坏块标志等其他信息放置在了spare area的字节32之后







