# ST Visual Debugger Generated MAKE File, based on mw550.stp

ifeq ($(CFG), )
CFG=Debug
$(warning ***No configuration specified. Defaulting to $(CFG)***)
endif

ToolsetRoot=C:\STMicroelectronics\st_toolset\CXSTM8
ToolsetBin=C:\STMicroelectronics\st_toolset\CXSTM8
ToolsetInc=C:\STMicroelectronics\st_toolset\CXSTM8\Hstm8
ToolsetLib=C:\STMicroelectronics\st_toolset\CXSTM8\Lib
ToolsetIncOpts=-iC:\STMicroelectronics\st_toolset\CXSTM8\Hstm8 
ToolsetLibOpts=-lC:\STMicroelectronics\st_toolset\CXSTM8\Lib 
ObjectExt=o
OutputExt=elf
InputName=$(basename $(notdir $<))


# 
# Debug
# 
ifeq "$(CFG)" "Debug"


OutputPath=Debug
ProjectSFile=mw550
TargetSName=$(ProjectSFile)
TargetFName=$(ProjectSFile).elf
IntermPath=$(dir $@)
CFLAGS_PRJ=$(ToolsetBin)\cxstm8  +mods0 +debug -pxp -no -pp -l $(ToolsetIncOpts) -cl$(IntermPath:%\=%) -co$(IntermPath:%\=%) $<
ASMFLAGS_PRJ=$(ToolsetBin)\castm8  -xx -l $(ToolsetIncOpts) -o$(IntermPath)$(InputName).$(ObjectExt) $<

all : $(OutputPath) $(ProjectSFile).elf

$(OutputPath) : 
	if not exist $(OutputPath)/ mkdir $(OutputPath)

Debug\audio.$(ObjectExt) : audio.c ..\cxstm8\hstm8\mods0.h include.h stm8s.h stm8s_type.h stm8s_conf.h stm8s105_global.h stm8s105_awu.h stm8s105_exti.h stm8s105_timer.h stm8s105_adc.h stm8s105_flash.h main.h global.h key.h bluetooth.h audio.h idt_acs422a67.h 
	@if not exist $(dir $@)  mkdir $(dir $@)
	$(CFLAGS_PRJ)

Debug\bluetooth.$(ObjectExt) : bluetooth.c ..\cxstm8\hstm8\mods0.h include.h stm8s.h stm8s_type.h stm8s_conf.h stm8s105_global.h stm8s105_awu.h stm8s105_exti.h stm8s105_timer.h stm8s105_adc.h stm8s105_flash.h main.h global.h key.h bluetooth.h audio.h idt_acs422a67.h 
	@if not exist $(dir $@)  mkdir $(dir $@)
	$(CFLAGS_PRJ)

Debug\cdr11iic.$(ObjectExt) : ..\..\..\..\cdr11iic.c ..\cxstm8\hstm8\mods0.h include.h stm8s.h stm8s_type.h stm8s_conf.h stm8s105_global.h stm8s105_awu.h stm8s105_exti.h stm8s105_timer.h stm8s105_adc.h stm8s105_flash.h main.h global.h key.h bluetooth.h audio.h idt_acs422a67.h ..\..\..\..\cdr01iic.h 
	@if not exist $(dir $@)  mkdir $(dir $@)
	$(CFLAGS_PRJ)

Debug\global.$(ObjectExt) : global.c ..\cxstm8\hstm8\mods0.h include.h stm8s.h stm8s_type.h stm8s_conf.h stm8s105_global.h stm8s105_awu.h stm8s105_exti.h stm8s105_timer.h stm8s105_adc.h stm8s105_flash.h main.h global.h key.h bluetooth.h audio.h idt_acs422a67.h 
	@if not exist $(dir $@)  mkdir $(dir $@)
	$(CFLAGS_PRJ)

Debug\idt_acs422a67.$(ObjectExt) : idt_acs422a67.c ..\cxstm8\hstm8\mods0.h include.h stm8s.h stm8s_type.h stm8s_conf.h stm8s105_global.h stm8s105_awu.h stm8s105_exti.h stm8s105_timer.h stm8s105_adc.h stm8s105_flash.h main.h global.h key.h bluetooth.h audio.h idt_acs422a67.h ..\..\..\..\cdr01iic.h 
	@if not exist $(dir $@)  mkdir $(dir $@)
	$(CFLAGS_PRJ)

Debug\key.$(ObjectExt) : key.c ..\cxstm8\hstm8\mods0.h include.h stm8s.h stm8s_type.h stm8s_conf.h stm8s105_global.h stm8s105_awu.h stm8s105_exti.h stm8s105_timer.h stm8s105_adc.h stm8s105_flash.h main.h global.h key.h bluetooth.h audio.h idt_acs422a67.h 
	@if not exist $(dir $@)  mkdir $(dir $@)
	$(CFLAGS_PRJ)

Debug\main.$(ObjectExt) : main.c ..\cxstm8\hstm8\mods0.h include.h stm8s.h stm8s_type.h stm8s_conf.h stm8s105_global.h stm8s105_awu.h stm8s105_exti.h stm8s105_timer.h stm8s105_adc.h stm8s105_flash.h main.h global.h key.h bluetooth.h audio.h idt_acs422a67.h 
	@if not exist $(dir $@)  mkdir $(dir $@)
	$(CFLAGS_PRJ)

Debug\stm8_interrupt_vector.$(ObjectExt) : stm8_interrupt_vector.c ..\cxstm8\hstm8\mods0.h include.h stm8s.h stm8s_type.h stm8s_conf.h stm8s105_global.h stm8s105_awu.h stm8s105_exti.h stm8s105_timer.h stm8s105_adc.h stm8s105_flash.h main.h global.h key.h bluetooth.h audio.h idt_acs422a67.h 
	@if not exist $(dir $@)  mkdir $(dir $@)
	$(CFLAGS_PRJ)

Debug\stm8s105_adc.$(ObjectExt) : stm8s105_adc.c ..\cxstm8\hstm8\mods0.h include.h stm8s.h stm8s_type.h stm8s_conf.h stm8s105_global.h stm8s105_awu.h stm8s105_exti.h stm8s105_timer.h stm8s105_adc.h stm8s105_flash.h main.h global.h key.h bluetooth.h audio.h idt_acs422a67.h 
	@if not exist $(dir $@)  mkdir $(dir $@)
	$(CFLAGS_PRJ)

Debug\stm8s105_awu.$(ObjectExt) : stm8s105_awu.c ..\cxstm8\hstm8\mods0.h include.h stm8s.h stm8s_type.h stm8s_conf.h stm8s105_global.h stm8s105_awu.h stm8s105_exti.h stm8s105_timer.h stm8s105_adc.h stm8s105_flash.h main.h global.h key.h bluetooth.h audio.h idt_acs422a67.h 
	@if not exist $(dir $@)  mkdir $(dir $@)
	$(CFLAGS_PRJ)

Debug\stm8s105_exti.$(ObjectExt) : stm8s105_exti.c ..\cxstm8\hstm8\mods0.h include.h stm8s.h stm8s_type.h stm8s_conf.h stm8s105_global.h stm8s105_awu.h stm8s105_exti.h stm8s105_timer.h stm8s105_adc.h stm8s105_flash.h main.h global.h key.h bluetooth.h audio.h idt_acs422a67.h 
	@if not exist $(dir $@)  mkdir $(dir $@)
	$(CFLAGS_PRJ)

Debug\stm8s105_flash.$(ObjectExt) : stm8s105_flash.c ..\cxstm8\hstm8\mods0.h include.h stm8s.h stm8s_type.h stm8s_conf.h stm8s105_global.h stm8s105_awu.h stm8s105_exti.h stm8s105_timer.h stm8s105_adc.h stm8s105_flash.h main.h global.h key.h bluetooth.h audio.h idt_acs422a67.h 
	@if not exist $(dir $@)  mkdir $(dir $@)
	$(CFLAGS_PRJ)

Debug\stm8s105_global.$(ObjectExt) : stm8s105_global.c ..\cxstm8\hstm8\mods0.h include.h stm8s.h stm8s_type.h stm8s_conf.h stm8s105_global.h stm8s105_awu.h stm8s105_exti.h stm8s105_timer.h stm8s105_adc.h stm8s105_flash.h main.h global.h key.h bluetooth.h audio.h idt_acs422a67.h 
	@if not exist $(dir $@)  mkdir $(dir $@)
	$(CFLAGS_PRJ)

Debug\stm8s105_timer.$(ObjectExt) : stm8s105_timer.c ..\cxstm8\hstm8\mods0.h include.h stm8s.h stm8s_type.h stm8s_conf.h stm8s105_global.h stm8s105_awu.h stm8s105_exti.h stm8s105_timer.h stm8s105_adc.h stm8s105_flash.h main.h global.h key.h bluetooth.h audio.h idt_acs422a67.h 
	@if not exist $(dir $@)  mkdir $(dir $@)
	$(CFLAGS_PRJ)

$(ProjectSFile).elf :  $(OutputPath)\audio.o $(OutputPath)\bluetooth.o $(OutputPath)\cdr11iic.o $(OutputPath)\global.o $(OutputPath)\idt_acs422a67.o $(OutputPath)\key.o $(OutputPath)\main.o $(OutputPath)\stm8_interrupt_vector.o $(OutputPath)\stm8s105_adc.o $(OutputPath)\stm8s105_awu.o $(OutputPath)\stm8s105_exti.o $(OutputPath)\stm8s105_flash.o $(OutputPath)\stm8s105_global.o $(OutputPath)\stm8s105_timer.o $(OutputPath)\mw550.lkf
	$(ToolsetBin)\clnk  $(ToolsetLibOpts) -o $(OutputPath)\$(TargetSName).sm8 -m$(OutputPath)\$(TargetSName).map $(OutputPath)\$(TargetSName).lkf 
	$(ToolsetBin)\cvdwarf  $(OutputPath)\$(TargetSName).sm8 

	$(ToolsetBin)\chex  -o $(OutputPath)\$(TargetSName).s19 $(OutputPath)\$(TargetSName).sm8
clean : 
	-@erase $(OutputPath)\audio.o
	-@erase $(OutputPath)\bluetooth.o
	-@erase $(OutputPath)\cdr11iic.o
	-@erase $(OutputPath)\global.o
	-@erase $(OutputPath)\idt_acs422a67.o
	-@erase $(OutputPath)\key.o
	-@erase $(OutputPath)\main.o
	-@erase $(OutputPath)\stm8_interrupt_vector.o
	-@erase $(OutputPath)\stm8s105_adc.o
	-@erase $(OutputPath)\stm8s105_awu.o
	-@erase $(OutputPath)\stm8s105_exti.o
	-@erase $(OutputPath)\stm8s105_flash.o
	-@erase $(OutputPath)\stm8s105_global.o
	-@erase $(OutputPath)\stm8s105_timer.o
	-@erase $(OutputPath)\mw550.elf
	-@erase $(OutputPath)\mw550.elf
	-@erase $(OutputPath)\mw550.map
	-@erase $(OutputPath)\audio.ls
	-@erase $(OutputPath)\bluetooth.ls
	-@erase $(OutputPath)\cdr11iic.ls
	-@erase $(OutputPath)\global.ls
	-@erase $(OutputPath)\idt_acs422a67.ls
	-@erase $(OutputPath)\key.ls
	-@erase $(OutputPath)\main.ls
	-@erase $(OutputPath)\stm8_interrupt_vector.ls
	-@erase $(OutputPath)\stm8s105_adc.ls
	-@erase $(OutputPath)\stm8s105_awu.ls
	-@erase $(OutputPath)\stm8s105_exti.ls
	-@erase $(OutputPath)\stm8s105_flash.ls
	-@erase $(OutputPath)\stm8s105_global.ls
	-@erase $(OutputPath)\stm8s105_timer.ls
endif

# 
# Release
# 
ifeq "$(CFG)" "Release"


OutputPath=Release
ProjectSFile=mw550
TargetSName=mw550
TargetFName=mw550.elf
IntermPath=$(dir $@)
CFLAGS_PRJ=$(ToolsetBin)\cxstm8  +mods0 -pp $(ToolsetIncOpts) -cl$(IntermPath:%\=%) -co$(IntermPath:%\=%) $<
ASMFLAGS_PRJ=$(ToolsetBin)\castm8  $(ToolsetIncOpts) -o$(IntermPath)$(InputName).$(ObjectExt) $<

all : $(OutputPath) mw550.elf

$(OutputPath) : 
	if not exist $(OutputPath)/ mkdir $(OutputPath)

Release\audio.$(ObjectExt) : 
	@if not exist $(dir $@)  mkdir $(dir $@)
	$(CFLAGS_PRJ)

Release\bluetooth.$(ObjectExt) : 
	@if not exist $(dir $@)  mkdir $(dir $@)
	$(CFLAGS_PRJ)

Release\cdr11iic.$(ObjectExt) : 
	@if not exist $(dir $@)  mkdir $(dir $@)
	$(CFLAGS_PRJ)

Release\global.$(ObjectExt) : 
	@if not exist $(dir $@)  mkdir $(dir $@)
	$(CFLAGS_PRJ)

Release\idt_acs422a67.$(ObjectExt) : 
	@if not exist $(dir $@)  mkdir $(dir $@)
	$(CFLAGS_PRJ)

Release\key.$(ObjectExt) : 
	@if not exist $(dir $@)  mkdir $(dir $@)
	$(CFLAGS_PRJ)

Release\main.$(ObjectExt) : 
	@if not exist $(dir $@)  mkdir $(dir $@)
	$(CFLAGS_PRJ)

Release\stm8_interrupt_vector.$(ObjectExt) : 
	@if not exist $(dir $@)  mkdir $(dir $@)
	$(CFLAGS_PRJ)

Release\stm8s105_adc.$(ObjectExt) : 
	@if not exist $(dir $@)  mkdir $(dir $@)
	$(CFLAGS_PRJ)

Release\stm8s105_awu.$(ObjectExt) : 
	@if not exist $(dir $@)  mkdir $(dir $@)
	$(CFLAGS_PRJ)

Release\stm8s105_exti.$(ObjectExt) : 
	@if not exist $(dir $@)  mkdir $(dir $@)
	$(CFLAGS_PRJ)

Release\stm8s105_flash.$(ObjectExt) : 
	@if not exist $(dir $@)  mkdir $(dir $@)
	$(CFLAGS_PRJ)

Release\stm8s105_global.$(ObjectExt) : 
	@if not exist $(dir $@)  mkdir $(dir $@)
	$(CFLAGS_PRJ)

Release\stm8s105_timer.$(ObjectExt) : 
	@if not exist $(dir $@)  mkdir $(dir $@)
	$(CFLAGS_PRJ)

mw550.elf :  $(OutputPath)\audio.o $(OutputPath)\bluetooth.o $(OutputPath)\cdr11iic.o $(OutputPath)\global.o $(OutputPath)\idt_acs422a67.o $(OutputPath)\key.o $(OutputPath)\main.o $(OutputPath)\stm8_interrupt_vector.o $(OutputPath)\stm8s105_adc.o $(OutputPath)\stm8s105_awu.o $(OutputPath)\stm8s105_exti.o $(OutputPath)\stm8s105_flash.o $(OutputPath)\stm8s105_global.o $(OutputPath)\stm8s105_timer.o $(OutputPath)\mw550.lkf
	$(ToolsetBin)\clnk  $(ToolsetLibOpts) -o $(OutputPath)\$(TargetSName).sm8 $(OutputPath)\$(TargetSName).lkf 
	$(ToolsetBin)\cvdwarf  $(OutputPath)\$(TargetSName).sm8 

	$(ToolsetBin)\chex  -o $(OutputPath)\$(TargetSName).s19 $(OutputPath)\$(TargetSName).sm8
clean : 
	-@erase $(OutputPath)\audio.o
	-@erase $(OutputPath)\bluetooth.o
	-@erase $(OutputPath)\cdr11iic.o
	-@erase $(OutputPath)\global.o
	-@erase $(OutputPath)\idt_acs422a67.o
	-@erase $(OutputPath)\key.o
	-@erase $(OutputPath)\main.o
	-@erase $(OutputPath)\stm8_interrupt_vector.o
	-@erase $(OutputPath)\stm8s105_adc.o
	-@erase $(OutputPath)\stm8s105_awu.o
	-@erase $(OutputPath)\stm8s105_exti.o
	-@erase $(OutputPath)\stm8s105_flash.o
	-@erase $(OutputPath)\stm8s105_global.o
	-@erase $(OutputPath)\stm8s105_timer.o
	-@erase $(OutputPath)\mw550.elf
	-@erase $(OutputPath)\mw550.map
	-@erase $(OutputPath)\mw550.st7
	-@erase $(OutputPath)\mw550.s19
	-@erase $(OutputPath)\audio.ls
	-@erase $(OutputPath)\bluetooth.ls
	-@erase $(OutputPath)\cdr11iic.ls
	-@erase $(OutputPath)\global.ls
	-@erase $(OutputPath)\idt_acs422a67.ls
	-@erase $(OutputPath)\key.ls
	-@erase $(OutputPath)\main.ls
	-@erase $(OutputPath)\stm8_interrupt_vector.ls
	-@erase $(OutputPath)\stm8s105_adc.ls
	-@erase $(OutputPath)\stm8s105_awu.ls
	-@erase $(OutputPath)\stm8s105_exti.ls
	-@erase $(OutputPath)\stm8s105_flash.ls
	-@erase $(OutputPath)\stm8s105_global.ls
	-@erase $(OutputPath)\stm8s105_timer.ls
endif
