# Generic Makefile (based on gcc)

# 目标名称
TARGET = stm32h743_azurertos

# 编译路径
BUILD_BASE_DIR = build
BUILD_DIR = $(BUILD_BASE_DIR)/debug

# 可执行文件输出路径
OUTPUT_DIR = $(BUILD_BASE_DIR)/bin

######################################
# 文件列表
######################################
# 用户
C_SOURCES =  \
$(wildcard user/app/*.c) \
$(wildcard user/hardware/*.c) 

C_INCLUDES =  \
-Iuser/app \
-Iuser/hardware \

# STM32H7xx
C_SOURCES +=  \
drivers/CMSIS/Device/ST/STM32H7xx/Source/Templates/system_stm32h7xx.c \
$(filter-out drivers/STM32H7xx_HAL_Driver/Src/%_template.c, $(wildcard drivers/STM32H7xx_HAL_Driver/Src/*.c)) # 排除_template.c

C_INCLUDES +=  \
-Idrivers/CMSIS/Include \
-Idrivers/CMSIS/Device/ST/STM32H7xx/Include \
-Idrivers/STM32H7xx_HAL_Driver/Inc \
-Idrivers/STM32H7xx_HAL_Driver/Inc/Legacy \

# ThreadX
C_SOURCES +=  \
$(wildcard drivers/threadx-6.1.11_rel/common/src/*.c) \
drivers/threadx-6.1.11_rel/utility/execution_profile_kit/tx_execution_profile.c

C_INCLUDES +=  \
-Idrivers/threadx-6.1.11_rel/common/inc \
-Idrivers/threadx-6.1.11_rel/ports/cortex_m7/gnu/inc \
-Idrivers/threadx-6.1.11_rel/utility/execution_profile_kit \
-Iuser/threadx \

# ASM源文件
ASM_SOURCES =  \
drivers/CMSIS/Device/ST/STM32H7xx/Source/Templates/gcc/startup_stm32h743xx.s
ASM_SOURCES_S = \
$(wildcard drivers/threadx-6.1.11_rel/ports/cortex_m7/gnu/src/*.S) \
user/threadx/tx_initialize_low_level.S

######################################
# 编译选项
######################################
# C全局宏定义
C_DEFS =  \
-DUSE_FULL_LL_DRIVER \
-DUSE_HAL_DRIVER \
-DSTM32H743xx \
-DTX_INCLUDE_USER_DEFINE_FILE \
-DTX_EXECUTION_PROFILE_ENABLE \
-DTX_CORTEX_M_EPK \

# 汇编全局宏定义
AS_DEFS = \
-DTX_EXECUTION_PROFILE_ENABLE

# 优化等级 
# -O0 不优化
# -O1/-O -O2 -O3 数字越大优化等级越高
# -Og	 在-O1的基础上去掉影响调试的优化
# -Os	 在-O2的基础上去掉会导致可执行程序增大的优化,可节省存储空间
# -Ofast 在-O3的基础上添加一些非常规优化,会打破一些国际标准,一般不推荐
OPT = -Og

# MCU(CPU FPU双精度 硬浮点ABI)
MCU = -mthumb -mcpu=cortex-m7 -mfpu=fpv5-d16 -mfloat-abi=hard

# 编译选项汇总
CFLAGS = $(MCU) $(C_DEFS) $(C_INCLUDES) $(OPT) -Wall -fdata-sections -ffunction-sections -std=c99
ASFLAGS = $(MCU) $(AS_DEFS) $(AS_INCLUDES) $(OPT) -Wall -fdata-sections -ffunction-sections

# 开启调试信息(不影响hex/bin文件,影响elf文件,不开启不能gdb调试)
CFLAGS += -g -gdwarf-2

# 生成文件依赖关系到.d文件
CFLAGS += -MMD -MP -MF"$(@:%.o=%.d)"

#######################################
# 链接选项
#######################################
# ld脚本
LDSCRIPT = \
drivers/CMSIS/Device/ST/STM32H7xx/Source/Templates/gcc/linker/STM32H743IITx.ld

# 库
LIBS = -lc -lm -lnosys
LIBDIR = 

# 链接选项汇总
LDFLAGS = $(MCU) -specs=nano.specs -T$(LDSCRIPT) $(LIBDIR) $(LIBS) -Wl,-Map=$(BUILD_DIR)/$(TARGET).map,--cref -Wl,--gc-sections

# printf支持浮点数(生成的可执行程序大概会增大8KB)
LDFLAGS += -lc -lrdimon -u _printf_float

######################################
# 编译器
######################################
# GCC编译器
PREFIX = C:/gcc-arm-none-eabi-10.3-2021.10/bin/arm-none-eabi-
CC = $(PREFIX)gcc
AS = $(PREFIX)gcc -x assembler-with-cpp
CP = $(PREFIX)objcopy
SZ = $(PREFIX)size

#######################################
# 编译
#######################################
# 最终目标三种格式文件 build all
all: $(BUILD_DIR)/$(TARGET).elf $(BUILD_DIR)/$(TARGET).hex $(BUILD_DIR)/$(TARGET).bin 

# C目标列表
OBJECTS = $(addprefix $(BUILD_DIR)/,$(notdir $(C_SOURCES:.c=.o)))
vpath %.c $(sort $(dir $(C_SOURCES)))

# 汇编目标列表
OBJECTS += $(addprefix $(BUILD_DIR)/,$(notdir $(ASM_SOURCES:.s=.o)))
vpath %.s $(sort $(dir $(ASM_SOURCES)))
OBJECTS += $(addprefix $(BUILD_DIR)/,$(notdir $(ASM_SOURCES_S:.S=.o)))
vpath %.S $(sort $(dir $(ASM_SOURCES_S)))

# 编译C文件
$(BUILD_DIR)/%.o: %.c Makefile | $(BUILD_DIR) 
	$(CC) -c $(CFLAGS) -Wa,-a,-ad,-alms=$(BUILD_DIR)/$(notdir $(<:.c=.lst)) $< -o $@

# 编译汇编文件(.s只汇编 .S先预处理再汇编)
$(BUILD_DIR)/%.o: %.s Makefile | $(BUILD_DIR)
	$(AS) -c $(ASFLAGS) $< -o $@
$(BUILD_DIR)/%.o: %.S Makefile | $(BUILD_DIR)
	$(AS) -c $(ASFLAGS) $< -o $@

# 链接生成可执行文件并拷贝至输出目录
$(BUILD_DIR)/$(TARGET).elf: $(OBJECTS) Makefile $(LDSCRIPT)
	$(CC) $(OBJECTS) $(LDFLAGS) -o $@
	$(SZ) $@

# 生成HEX文件并拷贝至输出目录
$(BUILD_DIR)/%.hex: $(BUILD_DIR)/%.elf | $(BUILD_DIR) $(OUTPUT_DIR)
	$(CP) -O ihex $< $@
	powershell cp $@ $(OUTPUT_DIR)/$(TARGET).hex

# 生成BIN文件并拷贝至输出目录
$(BUILD_DIR)/%.bin: $(BUILD_DIR)/%.elf | $(BUILD_DIR) $(OUTPUT_DIR)
	$(CP) -O binary -S $< $@
	powershell cp $@ $(OUTPUT_DIR)/$(TARGET).bin

# 如果没有则创建输出目录
$(OUTPUT_DIR):
	powershell mkdir $@

# 如果没有则创建BUILD目录
$(BUILD_DIR):
	powershell mkdir $@

# 包含依赖文件(保证有头文件修改时make能重新编译)
-include $(wildcard $(BUILD_DIR)/*.d)

#######################################
# clean
#######################################
# 删除build文件夹
clean:
	-rmdir /S/Q $(BUILD_BASE_DIR)


