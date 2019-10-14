;****************************************************************************
;*   ctx_save
;*
;*     C syntax  : int ctx_save(ctx env)
;*
;*     Function  : Save callers current environment for a subsequent
;*                 call to reg_load_state.
;*
;*     The context save area is organized as follows:
;*
;*       env -->  .int   R4
;*                .int   R5
;*                .int   R6
;*                .int   R7
;*                .int   R8
;*                .int   R9
;*                .int   R10
;*                .int   SP
;*                .int   PC
;*				  .int   SR
;*
;****************************************************************************
     	.global	ctx_save,_ctx_save

	.text
	.align 2

     .if $DEFINED(__LARGE_CODE_MODEL__)
        .asg  4, RETADDRSZ
     .else
        .asg  2, RETADDRSZ
     .endif

	.if $DEFINED(__LARGE_CODE_MODEL__) | $DEFINED(__LARGE_DATA_MODEL__)
ctx_save: .asmfunc stack_usage(RETADDRSZ)
_ctx_save:
          MOVA   R4, 0(R12)			; save R4
          MOVA   R5, 4(R12)			; save R5
          MOVA   R6, 8(R12)			; save R6
          MOVA   R7, 12(R12)		; save R7
          MOVA   R8, 16(R12)		; save R8
          MOVA   R9, 20(R12)		; save R9
          MOVA   R10, 24(R12)		; save R10
          MOVA   SP, 28(R12)		; save SP

	.if $DEFINED(__LARGE_CODE_MODEL__)
          MOVX.A @SP,32(R12)
          ADDX.A #4,28(R12)        	; Increment saved SP by four ("pop" PC)
          MOV.W  #0,R12
          RETA
	.else
	  MOV.W  @SP,32(R12)
	  ADDX.A #2,28(R12)
	  MOV.W	 #0,R12
	  RET
	.endif
	.else
ctx_save: .asmfunc stack_usage(RETADDRSZ)
_ctx_save:
          MOV.W   R4,0(R12)
          MOV.W   R5,2(R12)
          MOV.W   R6,4(R12)
          MOV.W   R7,6(R12)
          MOV.W   R8,8(R12)
          MOV.W   R9,10(R12)
          MOV.W   R10,12(R12)
          MOV.W   SP,14(R12)
          MOV.W   @SP,16(R12)
          ADD.W   #2,14(R12)        ; Increment saved SP by two ("pop" PC)
          MOV.W   #0,R12
          RET
	.endif

          .endasmfunc

;****************************************************************************
;*   ctx_load
;*
;*     C syntax  : void ctx_load(ctx env)
;*
;*     Function  : Restore the context contained in the jump buffer.
;*                 This causes an apparent "2nd return" from the
;*                 setjmp invocation which built the "env" buffer.
;*				   Finally, reload the status register before jumping.
;****************************************************************************
          .global	ctx_load

	.if $DEFINED(__LARGE_CODE_MODEL__) | $DEFINED(__LARGE_DATA_MODEL__)
ctx_load: .asmfunc stack_usage(RETADDRSZ)
          MOVA   0(R12), R4			; load R4
          MOVA   4(R12), R5			; load R5
          MOVA   8(R12), R6			; load R6
          MOVA   12(R12), R7		; load R7
          MOVA   16(R12), R8		; load R8
          MOVA   20(R12), R9		; load R9
          MOVA   24(R12), R10		; load R10
          MOVA   28(R12), SP		; load SP

	.if $DEFINED(__LARGE_CODE_MODEL__)	; fetch PC addr to jump to
          MOVA   32(R12), R14
	.else
	      MOV.W  32(R12), R14
	.endif

		  MOVA   36(R12), SR		; set SR (EINT + whatever else), next instruction is always executed before interrupt

	.if $DEFINED(__LARGE_CODE_MODEL__)
end:      BRA     R14				; jump to runnable
	.else
end:      BR      R14
	.endif

	.else
ctx_load: .asmfunc stack_usage(RETADDRSZ)
          MOV.W   0(R12), R4
          MOV.W   2(R12), R5
          MOV.W   4(R12), R6
          MOV.W   6(R12), R7
          MOV.W   8(R12), R8
          MOV.W   10(R12), R9
          MOV.W   12(R12), R10
          MOV.W   14(R12), SP
		  MOV.W   16(R12), R14
		  MOV.W   18(R12), SR
end:      BR      R14
	.endif

          .endasmfunc


;******************************************************************************
;* BUILD ATTRIBUTES                                                           *
;*    HW_MPY_INLINE_INFO=1:  file does not have any inlined hw mpy            *
;*    HW_MPY_ISR_INFO   =1:  file does not have ISR's with mpy or func calls  *
;******************************************************************************
	.battr "TI", Tag_File, 1, Tag_HW_MPY_INLINE_INFO(1)
	.battr "TI", Tag_File, 1, Tag_HW_MPY_ISR_INFO(1)
        .end
