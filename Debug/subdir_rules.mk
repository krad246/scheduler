################################################################################
# Automatically-generated file. Do not edit!
################################################################################

SHELL = cmd.exe

# Each subdirectory must supply rules for building sources it contributes
%.obj: ../%.cpp $(GEN_OPTS) | $(GEN_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: MSP430 Compiler'
	"C:/ti/ccsv8/tools/compiler/ti-cgt-msp430_18.1.6.LTS/bin/cl430" -vmspx --code_model=large --data_model=restricted -O4 --opt_for_speed=2 --use_hw_mpy=F5 --include_path="C:/ti/ccsv8/ccs_base/msp430/include" --include_path="C:/boost_1_69_0" --include_path="C:/Users/krad2/workspace_v8/f5529_kernel" --include_path="C:/ti/ccsv8/tools/compiler/ti-cgt-msp430_18.1.6.LTS/include" --advice:power="all" --define=__MSP430F5529__ -g --cpp_default --printf_support=minimal --diag_warning=225 --diag_wrap=off --display_error_number --silicon_errata=CPU21 --silicon_errata=CPU22 --silicon_errata=CPU23 --silicon_errata=CPU40 --preproc_with_compile --preproc_dependency="$(basename $(<F)).d_raw" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: "$<"'
	@echo ' '

%.obj: ../%.asm $(GEN_OPTS) | $(GEN_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: MSP430 Compiler'
	"C:/ti/ccsv8/tools/compiler/ti-cgt-msp430_18.1.6.LTS/bin/cl430" -vmspx --code_model=large --data_model=restricted -O4 --opt_for_speed=2 --use_hw_mpy=F5 --include_path="C:/ti/ccsv8/ccs_base/msp430/include" --include_path="C:/boost_1_69_0" --include_path="C:/Users/krad2/workspace_v8/f5529_kernel" --include_path="C:/ti/ccsv8/tools/compiler/ti-cgt-msp430_18.1.6.LTS/include" --advice:power="all" --define=__MSP430F5529__ -g --cpp_default --printf_support=minimal --diag_warning=225 --diag_wrap=off --display_error_number --silicon_errata=CPU21 --silicon_errata=CPU22 --silicon_errata=CPU23 --silicon_errata=CPU40 --preproc_with_compile --preproc_dependency="$(basename $(<F)).d_raw" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: "$<"'
	@echo ' '

main.obj: ../main.cpp $(GEN_OPTS) | $(GEN_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: MSP430 Compiler'
	"C:/ti/ccsv8/tools/compiler/ti-cgt-msp430_18.1.6.LTS/bin/cl430" -vmspx --code_model=large --data_model=restricted -O4 --opt_for_speed=2 --use_hw_mpy=F5 --include_path="C:/ti/ccsv8/ccs_base/msp430/include" --include_path="C:/boost_1_69_0" --include_path="C:/Users/krad2/workspace_v8/f5529_kernel" --include_path="C:/ti/ccsv8/tools/compiler/ti-cgt-msp430_18.1.6.LTS/include" --advice:power="all" --define=__MSP430F5529__ -g --cpp_default --printf_support=minimal --diag_warning=225 --diag_wrap=off --display_error_number --silicon_errata=CPU21 --silicon_errata=CPU22 --silicon_errata=CPU23 --silicon_errata=CPU40 --preproc_with_compile --preproc_dependency="$(basename $(<F)).d_raw" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: "$<"'
	@echo ' '


