################################################################################
# Automatically-generated file. Do not edit!
################################################################################

SHELL = cmd.exe

# Each subdirectory must supply rules for building sources it contributes
%.obj: ../%.cpp $(GEN_OPTS) | $(GEN_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: MSP430 Compiler'
<<<<<<< HEAD
	"C:/ti/ccsv8/tools/compiler/ti-cgt-msp430_18.1.5.LTS/bin/cl430" -vmsp -O4 --opt_for_speed=5 --use_hw_mpy=none --include_path="C:/ti/ccsv8/ccs_base/msp430/include" --include_path="C:/Users/krad2/workspace_v8/sched" --include_path="C:/ti/ccsv8/tools/compiler/ti-cgt-msp430_18.1.5.LTS/include" --advice:power="all" --define=__MSP430G2553__ -g --cpp_default --printf_support=minimal --diag_warning=225 --diag_wrap=off --display_error_number --preproc_with_compile --preproc_dependency="$(basename $(<F)).d_raw" $(GEN_OPTS__FLAG) "$<"
=======
	"C:/ti/ccsv8/tools/compiler/ti-cgt-msp430_18.1.5.LTS/bin/cl430" -vmspx --data_model=restricted -O1 --use_hw_mpy=F5 --include_path="C:/ti/ccsv8/ccs_base/msp430/include" --include_path="C:/Users/krad2/workspace_v8/scheduler_refactor" --include_path="C:/ti/ccsv8/tools/compiler/ti-cgt-msp430_18.1.5.LTS/include" --advice:power=all --define=__MSP430F5529__ -g --cpp_default --printf_support=minimal --diag_warning=225 --diag_wrap=off --display_error_number --silicon_errata=CPU21 --silicon_errata=CPU22 --silicon_errata=CPU23 --silicon_errata=CPU40 --preproc_with_compile --preproc_dependency="$(basename $(<F)).d_raw" $(GEN_OPTS__FLAG) "$<"
>>>>>>> master
	@echo 'Finished building: "$<"'
	@echo ' '


