################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Each subdirectory must supply rules for building sources it contributes
drivers/cfal96x64x16.obj: /home/victor_macedo/ti/tivaware_c_series_2_1_4_178/examples/boards/dk-tm4c123g/drivers/cfal96x64x16.c $(GEN_OPTS) | $(GEN_FILES) $(GEN_MISC_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: ARM Compiler'
	"/home/victor_macedo/ti/ccs1210/ccs/tools/compiler/ti-cgt-arm_20.2.7.LTS/bin/armcl" -mv7M4 --code_state=16 --float_support=FPv4SPD16 --abi=eabi -me -O2 --include_path="/home/victor_macedo/ti/ccs1210/ccs/tools/compiler/ti-cgt-arm_20.2.7.LTS/include" --include_path="/home/victor_macedo/ti/tivaware_c_series_2_1_4_178/examples/boards/dk-tm4c123g" --include_path="/home/victor_macedo/ti/tivaware_c_series_2_1_4_178" --advice:power=all -g --gcc --define=ccs="ccs" --define=PART_TM4C123GH6PGE --define=TARGET_IS_TM4C123_RB1 --diag_warning=225 --diag_wrap=off --display_error_number --gen_func_subsections=on --ual --preproc_with_compile --preproc_dependency="drivers/$(basename $(<F)).d_raw" --obj_directory="drivers" $(GEN_OPTS__FLAG) "$(shell echo $<)"
	@echo 'Finished building: "$<"'
	@echo ' '


