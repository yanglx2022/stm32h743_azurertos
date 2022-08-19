# 基于STM32H743+AzureRTOS的综合示例项目

STM32H743与AzureRTOS学习记录总结……

[2022/05/05] 创建项目、移植HAL库与ThreadX

[2022/05/06] SDRAM驱动

[2022/05/08] 配置AXIRAM、SDRAM的data与bss段，并实现初始化

[2022/08/19] SD卡驱动，FileX文件系统移植(中文支持)

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
DTCMRAM (xrw)      : ORIGIN = 0x20000000, LENGTH = 128K
RAM_D1 (xrw)      : ORIGIN = 0x24000000, LENGTH = 512K
RAM_D2 (xrw)      : ORIGIN = 0x30000000, LENGTH = 288K
RAM_D3 (xrw)      : ORIGIN = 0x38000000, LENGTH = 64K
ITCMRAM (xrw)      : ORIGIN = 0x00000000, LENGTH = 64K
FLASH (rx)      : ORIGIN = 0x8000000, LENGTH = 2048K
SDRAM (rw)      : ORIGIN = 0xC0000000, LENGTH = 32M
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
    * 读写扇区等操作，详见`user/filex/fx_sdcard_driver.c`

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

由上表可得UTF-16转UTF-8算法：根据字符UTF-16值的范围确定UTF-8编码字节数，再根据相应编码格式完成转换。详细算法见文件`user/app/thread_file.c`中的`utf16_to_utf8`函数

UTF-16转GBK的话没有固定的规律，需要一张包含每个字符的Unicode到GBK映射的转换表。转GB2312等其他中文编码也是一样的原理。