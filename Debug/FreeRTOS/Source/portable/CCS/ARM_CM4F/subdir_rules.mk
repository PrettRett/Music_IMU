################################################################################
# Automatically-generated file. Do not edit!
################################################################################

SHELL = cmd.exe

# Each subdirectory must supply rules for building sources it contributes
FreeRTOS/Source/portable/CCS/ARM_CM4F/port.obj: ../FreeRTOS/Source/portable/CCS/ARM_CM4F/port.c $(GEN_OPTS) | $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: ARM Compiler'
	"C:/ti/ccsv7/tools/compiler/ti-cgt-arm_16.9.3.LTS/bin/armcl" -mv7M4 --code_state=16 --float_support=FPv4SPD16 -me --include_path="C:/Users/Daniel/workspace_v7/Music_IMU" --include_path="C:/Users/Daniel/workspace_v7/Music_IMU/FreeRTOS/Source/portable/CCS/ARM_CM4F" --include_path="C:/Users/Daniel/workspace_v7/Music_IMU/FreeRTOS/Source/include" --include_path="C:/Users/Daniel/workspace_v7/Music_IMU/FreeRTOS/Source/portable/MemMang" --include_path="C:/ti/ccsv7/tools/compiler/ti-cgt-arm_16.9.3.LTS/include" --define=ccs="ccs" --define=PART_TM4C123GH6PM -g --gcc --diag_warning=225 --diag_wrap=off --display_error_number --abi=eabi --preproc_with_compile --preproc_dependency="FreeRTOS/Source/portable/CCS/ARM_CM4F/port.d" --obj_directory="FreeRTOS/Source/portable/CCS/ARM_CM4F" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

FreeRTOS/Source/portable/CCS/ARM_CM4F/portasm.obj: ../FreeRTOS/Source/portable/CCS/ARM_CM4F/portasm.asm $(GEN_OPTS) | $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: ARM Compiler'
	"C:/ti/ccsv7/tools/compiler/ti-cgt-arm_16.9.3.LTS/bin/armcl" -mv7M4 --code_state=16 --float_support=FPv4SPD16 -me --include_path="C:/Users/Daniel/workspace_v7/Music_IMU" --include_path="C:/Users/Daniel/workspace_v7/Music_IMU/FreeRTOS/Source/portable/CCS/ARM_CM4F" --include_path="C:/Users/Daniel/workspace_v7/Music_IMU/FreeRTOS/Source/include" --include_path="C:/Users/Daniel/workspace_v7/Music_IMU/FreeRTOS/Source/portable/MemMang" --include_path="C:/ti/ccsv7/tools/compiler/ti-cgt-arm_16.9.3.LTS/include" --define=ccs="ccs" --define=PART_TM4C123GH6PM -g --gcc --diag_warning=225 --diag_wrap=off --display_error_number --abi=eabi --preproc_with_compile --preproc_dependency="FreeRTOS/Source/portable/CCS/ARM_CM4F/portasm.d" --obj_directory="FreeRTOS/Source/portable/CCS/ARM_CM4F" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '


