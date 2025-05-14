##
## Auto Generated makefile by CodeLite IDE
## any manual changes will be erased      
##
## Debug
ProjectName            :=rimer
ConfigurationName      :=Debug
WorkspaceConfiguration :=Debug
WorkspacePath          :=/Users/sergey/projloc/rimer/fw
ProjectPath            :=/Users/sergey/projloc/rimer/fw
IntermediateDirectory  :=$(ConfigurationName)
OutDir                 := $(IntermediateDirectory)
CurrentFileName        :=
CurrentFilePath        :=
CurrentFileFullPath    :=
User                   :=sergey
Date                   :=13/05/2025
CodeLitePath           :=/Users/sergey/.codelite
LinkerName             :=/Applications/arm/bin/arm-none-eabi-gcc
SharedObjectLinkerName :=/Applications/arm/bin/arm-none-eabi-g++ -shared -fPIC
ObjectSuffix           :=.o
DependSuffix           :=.o.d
PreprocessSuffix       :=.i
DebugSwitch            :=-g 
IncludeSwitch          :=-I
LibrarySwitch          :=-l
OutputSwitch           :=-o 
LibraryPathSwitch      :=-L
PreprocessorSwitch     :=-D
SourceSwitch           :=-c 
OutputDirectory        :=$(IntermediateDirectory)
OutputFile             :=$(IntermediateDirectory)/$(ProjectName)
Preprocessors          :=$(PreprocessorSwitch)__SAMD51J20A__ 
ObjectSwitch           :=-o 
ArchiveOutputSwitch    := 
PreprocessOnlySwitch   :=-E
ObjectsFileList        :="rimer.txt"
PCHCompileFlags        :=
MakeDirCommand         :=mkdir -p
LinkOptions            := -TucosR_1M_256k.ld -mthumb -Wl,-Map=$(ProjectName).map --specs=nosys.specs --specs=nano.specs -Wl,--gc-sections -mcpu=cortex-m4 -mfloat-abi=hard -mfpu=fpv4-sp-d16
IncludePath            :=  $(IncludeSwitch). $(IncludeSwitch)inc $(IncludeSwitch)inc/cmsis $(IncludeSwitch)inc/samd51 $(IncludeSwitch)inc/kernel $(IncludeSwitch)iface $(IncludeSwitch)basicd $(IncludeSwitch)zx80 $(IncludeSwitch)bios/samd51 
IncludePCH             := 
RcIncludePath          := 
Libs                   := $(LibrarySwitch)m $(LibrarySwitch)ucosR 
ArLibs                 :=  "m" "ucosR" 
LibPath                := $(LibraryPathSwitch)lib 

##
## Common variables
## AR, CXX, CC, AS, CXXFLAGS and CFLAGS can be overridden using an environment variable
##
AR       := /Applications/arm/bin/arm-none-eabi-ar rcu
CXX      := /Applications/arm/bin/arm-none-eabi-g++
CC       := /Applications/arm/bin/arm-none-eabi-gcc
CXXFLAGS :=  -g -O0 -Wall $(Preprocessors)
CFLAGS   := -mthumb -ffunction-sections -fdata-sections -mcpu=cortex-m4 -std=gnu99 -mfloat-abi=hard -mfpu=fpv4-sp-d16 -g -O0 -Wall $(Preprocessors)
ASFLAGS  := 
AS       := /Applications/arm/bin/arm-none-eabi-as


##
## User defined environment variables
##
CodeLiteDir:=/Applications/codelite.app/Contents/SharedSupport/
shell:=zsh
Objects0=$(IntermediateDirectory)/src_main.c$(ObjectSuffix) $(IntermediateDirectory)/iface_iface_eeprom.c$(ObjectSuffix) $(IntermediateDirectory)/iface_iface_mem.c$(ObjectSuffix) $(IntermediateDirectory)/basicd_bprime.c$(ObjectSuffix) $(IntermediateDirectory)/zx80_z80cpu.c$(ObjectSuffix) 

Objects1=$(IntermediateDirectory)/src_startup.c$(ObjectSuffix) $(IntermediateDirectory)/iface_rimer_iface.c$(ObjectSuffix) $(IntermediateDirectory)/src_syscalls.c$(ObjectSuffix) $(IntermediateDirectory)/basicd_bhighlight.c$(ObjectSuffix) $(IntermediateDirectory)/zx80_z80mnx.c$(ObjectSuffix) \
	$(IntermediateDirectory)/basicd_bedit.c$(ObjectSuffix) $(IntermediateDirectory)/basicd_rpn.c$(ObjectSuffix) $(IntermediateDirectory)/zx80_zx80sys.c$(ObjectSuffix) $(IntermediateDirectory)/basicd_bfunc.c$(ObjectSuffix) $(IntermediateDirectory)/basicd_bprog_rom.c$(ObjectSuffix) $(IntermediateDirectory)/basicd_bcore.c$(ObjectSuffix) $(IntermediateDirectory)/basicd_bstring.c$(ObjectSuffix) $(IntermediateDirectory)/iface_iface_sio.c$(ObjectSuffix) $(IntermediateDirectory)/basicd_berror.c$(ObjectSuffix) $(IntermediateDirectory)/zx80_snapshot.c$(ObjectSuffix) \
	$(IntermediateDirectory)/iface_iface_sd.c$(ObjectSuffix) $(IntermediateDirectory)/zx80_zxscreen.c$(ObjectSuffix) $(IntermediateDirectory)/zx80_z80dbg.c$(ObjectSuffix) $(IntermediateDirectory)/basicd_bscreen.c$(ObjectSuffix) $(IntermediateDirectory)/iface_iface_aio.c$(ObjectSuffix) 

Objects2=$(IntermediateDirectory)/iface_iface_zx80.c$(ObjectSuffix) $(IntermediateDirectory)/basicd_banalizer.c$(ObjectSuffix) $(IntermediateDirectory)/iface_iface_set.c$(ObjectSuffix) $(IntermediateDirectory)/iface_enums.c$(ObjectSuffix) $(IntermediateDirectory)/iface_iface_dio.c$(ObjectSuffix) \
	$(IntermediateDirectory)/basicd_bmath.c$(ObjectSuffix) $(IntermediateDirectory)/iface_iface_bas.c$(ObjectSuffix) 



Objects=$(Objects0) $(Objects1) $(Objects2) 

##
## Main Build Targets 
##
.PHONY: all clean PreBuild PrePreBuild PostBuild MakeIntermediateDirs
all: $(OutputFile)

$(OutputFile): $(IntermediateDirectory)/.d $(Objects) 
	@$(MakeDirCommand) $(@D)
	@echo "" > $(IntermediateDirectory)/.d
	@echo $(Objects0)  > $(ObjectsFileList)
	@echo $(Objects1) >> $(ObjectsFileList)
	@echo $(Objects2) >> $(ObjectsFileList)
	$(LinkerName) $(OutputSwitch)$(OutputFile) @$(ObjectsFileList) $(LibPath) $(Libs) $(LinkOptions)

PostBuild:
	@echo Executing Post Build commands ...
	/Applications/ARM/bin/arm-none-eabi-size Debug/rimer
	/Applications/ARM/bin/arm-none-eabi-objcopy -O ihex Debug/rimer Debug/rimer.hex
	@echo Done

MakeIntermediateDirs:
	@test -d $(ConfigurationName) || $(MakeDirCommand) $(ConfigurationName)


$(IntermediateDirectory)/.d:
	@test -d $(ConfigurationName) || $(MakeDirCommand) $(ConfigurationName)

PreBuild:


##
## Objects
##
$(IntermediateDirectory)/src_main.c$(ObjectSuffix): src/main.c
	@$(CC) $(CFLAGS) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/src_main.c$(ObjectSuffix) -MF$(IntermediateDirectory)/src_main.c$(DependSuffix) -MM src/main.c
	$(CC) $(SourceSwitch) "/Users/sergey/projloc/rimer/fw/src/main.c" $(CFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/src_main.c$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/src_main.c$(PreprocessSuffix): src/main.c
	$(CC) $(CFLAGS) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/src_main.c$(PreprocessSuffix) src/main.c

$(IntermediateDirectory)/iface_iface_eeprom.c$(ObjectSuffix): iface/iface_eeprom.c
	@$(CC) $(CFLAGS) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/iface_iface_eeprom.c$(ObjectSuffix) -MF$(IntermediateDirectory)/iface_iface_eeprom.c$(DependSuffix) -MM iface/iface_eeprom.c
	$(CC) $(SourceSwitch) "/Users/sergey/projloc/rimer/fw/iface/iface_eeprom.c" $(CFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/iface_iface_eeprom.c$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/iface_iface_eeprom.c$(PreprocessSuffix): iface/iface_eeprom.c
	$(CC) $(CFLAGS) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/iface_iface_eeprom.c$(PreprocessSuffix) iface/iface_eeprom.c

$(IntermediateDirectory)/iface_iface_mem.c$(ObjectSuffix): iface/iface_mem.c
	@$(CC) $(CFLAGS) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/iface_iface_mem.c$(ObjectSuffix) -MF$(IntermediateDirectory)/iface_iface_mem.c$(DependSuffix) -MM iface/iface_mem.c
	$(CC) $(SourceSwitch) "/Users/sergey/projloc/rimer/fw/iface/iface_mem.c" $(CFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/iface_iface_mem.c$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/iface_iface_mem.c$(PreprocessSuffix): iface/iface_mem.c
	$(CC) $(CFLAGS) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/iface_iface_mem.c$(PreprocessSuffix) iface/iface_mem.c

$(IntermediateDirectory)/basicd_bprime.c$(ObjectSuffix): basicd/bprime.c
	@$(CC) $(CFLAGS) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/basicd_bprime.c$(ObjectSuffix) -MF$(IntermediateDirectory)/basicd_bprime.c$(DependSuffix) -MM basicd/bprime.c
	$(CC) $(SourceSwitch) "/Users/sergey/projloc/rimer/fw/basicd/bprime.c" $(CFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/basicd_bprime.c$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/basicd_bprime.c$(PreprocessSuffix): basicd/bprime.c
	$(CC) $(CFLAGS) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/basicd_bprime.c$(PreprocessSuffix) basicd/bprime.c

$(IntermediateDirectory)/zx80_z80cpu.c$(ObjectSuffix): zx80/z80cpu.c
	@$(CC) $(CFLAGS) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/zx80_z80cpu.c$(ObjectSuffix) -MF$(IntermediateDirectory)/zx80_z80cpu.c$(DependSuffix) -MM zx80/z80cpu.c
	$(CC) $(SourceSwitch) "/Users/sergey/projloc/rimer/fw/zx80/z80cpu.c" $(CFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/zx80_z80cpu.c$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/zx80_z80cpu.c$(PreprocessSuffix): zx80/z80cpu.c
	$(CC) $(CFLAGS) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/zx80_z80cpu.c$(PreprocessSuffix) zx80/z80cpu.c

$(IntermediateDirectory)/src_startup.c$(ObjectSuffix): src/startup.c
	@$(CC) $(CFLAGS) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/src_startup.c$(ObjectSuffix) -MF$(IntermediateDirectory)/src_startup.c$(DependSuffix) -MM src/startup.c
	$(CC) $(SourceSwitch) "/Users/sergey/projloc/rimer/fw/src/startup.c" $(CFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/src_startup.c$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/src_startup.c$(PreprocessSuffix): src/startup.c
	$(CC) $(CFLAGS) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/src_startup.c$(PreprocessSuffix) src/startup.c

$(IntermediateDirectory)/iface_rimer_iface.c$(ObjectSuffix): iface/rimer_iface.c
	@$(CC) $(CFLAGS) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/iface_rimer_iface.c$(ObjectSuffix) -MF$(IntermediateDirectory)/iface_rimer_iface.c$(DependSuffix) -MM iface/rimer_iface.c
	$(CC) $(SourceSwitch) "/Users/sergey/projloc/rimer/fw/iface/rimer_iface.c" $(CFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/iface_rimer_iface.c$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/iface_rimer_iface.c$(PreprocessSuffix): iface/rimer_iface.c
	$(CC) $(CFLAGS) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/iface_rimer_iface.c$(PreprocessSuffix) iface/rimer_iface.c

$(IntermediateDirectory)/src_syscalls.c$(ObjectSuffix): src/syscalls.c
	@$(CC) $(CFLAGS) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/src_syscalls.c$(ObjectSuffix) -MF$(IntermediateDirectory)/src_syscalls.c$(DependSuffix) -MM src/syscalls.c
	$(CC) $(SourceSwitch) "/Users/sergey/projloc/rimer/fw/src/syscalls.c" $(CFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/src_syscalls.c$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/src_syscalls.c$(PreprocessSuffix): src/syscalls.c
	$(CC) $(CFLAGS) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/src_syscalls.c$(PreprocessSuffix) src/syscalls.c

$(IntermediateDirectory)/basicd_bhighlight.c$(ObjectSuffix): basicd/bhighlight.c
	@$(CC) $(CFLAGS) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/basicd_bhighlight.c$(ObjectSuffix) -MF$(IntermediateDirectory)/basicd_bhighlight.c$(DependSuffix) -MM basicd/bhighlight.c
	$(CC) $(SourceSwitch) "/Users/sergey/projloc/rimer/fw/basicd/bhighlight.c" $(CFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/basicd_bhighlight.c$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/basicd_bhighlight.c$(PreprocessSuffix): basicd/bhighlight.c
	$(CC) $(CFLAGS) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/basicd_bhighlight.c$(PreprocessSuffix) basicd/bhighlight.c

$(IntermediateDirectory)/zx80_z80mnx.c$(ObjectSuffix): zx80/z80mnx.c
	@$(CC) $(CFLAGS) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/zx80_z80mnx.c$(ObjectSuffix) -MF$(IntermediateDirectory)/zx80_z80mnx.c$(DependSuffix) -MM zx80/z80mnx.c
	$(CC) $(SourceSwitch) "/Users/sergey/projloc/rimer/fw/zx80/z80mnx.c" $(CFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/zx80_z80mnx.c$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/zx80_z80mnx.c$(PreprocessSuffix): zx80/z80mnx.c
	$(CC) $(CFLAGS) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/zx80_z80mnx.c$(PreprocessSuffix) zx80/z80mnx.c

$(IntermediateDirectory)/basicd_bedit.c$(ObjectSuffix): basicd/bedit.c
	@$(CC) $(CFLAGS) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/basicd_bedit.c$(ObjectSuffix) -MF$(IntermediateDirectory)/basicd_bedit.c$(DependSuffix) -MM basicd/bedit.c
	$(CC) $(SourceSwitch) "/Users/sergey/projloc/rimer/fw/basicd/bedit.c" $(CFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/basicd_bedit.c$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/basicd_bedit.c$(PreprocessSuffix): basicd/bedit.c
	$(CC) $(CFLAGS) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/basicd_bedit.c$(PreprocessSuffix) basicd/bedit.c

$(IntermediateDirectory)/basicd_rpn.c$(ObjectSuffix): basicd/rpn.c
	@$(CC) $(CFLAGS) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/basicd_rpn.c$(ObjectSuffix) -MF$(IntermediateDirectory)/basicd_rpn.c$(DependSuffix) -MM basicd/rpn.c
	$(CC) $(SourceSwitch) "/Users/sergey/projloc/rimer/fw/basicd/rpn.c" $(CFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/basicd_rpn.c$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/basicd_rpn.c$(PreprocessSuffix): basicd/rpn.c
	$(CC) $(CFLAGS) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/basicd_rpn.c$(PreprocessSuffix) basicd/rpn.c

$(IntermediateDirectory)/zx80_zx80sys.c$(ObjectSuffix): zx80/zx80sys.c
	@$(CC) $(CFLAGS) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/zx80_zx80sys.c$(ObjectSuffix) -MF$(IntermediateDirectory)/zx80_zx80sys.c$(DependSuffix) -MM zx80/zx80sys.c
	$(CC) $(SourceSwitch) "/Users/sergey/projloc/rimer/fw/zx80/zx80sys.c" $(CFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/zx80_zx80sys.c$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/zx80_zx80sys.c$(PreprocessSuffix): zx80/zx80sys.c
	$(CC) $(CFLAGS) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/zx80_zx80sys.c$(PreprocessSuffix) zx80/zx80sys.c

$(IntermediateDirectory)/basicd_bfunc.c$(ObjectSuffix): basicd/bfunc.c
	@$(CC) $(CFLAGS) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/basicd_bfunc.c$(ObjectSuffix) -MF$(IntermediateDirectory)/basicd_bfunc.c$(DependSuffix) -MM basicd/bfunc.c
	$(CC) $(SourceSwitch) "/Users/sergey/projloc/rimer/fw/basicd/bfunc.c" $(CFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/basicd_bfunc.c$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/basicd_bfunc.c$(PreprocessSuffix): basicd/bfunc.c
	$(CC) $(CFLAGS) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/basicd_bfunc.c$(PreprocessSuffix) basicd/bfunc.c

$(IntermediateDirectory)/basicd_bprog_rom.c$(ObjectSuffix): basicd/bprog_rom.c
	@$(CC) $(CFLAGS) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/basicd_bprog_rom.c$(ObjectSuffix) -MF$(IntermediateDirectory)/basicd_bprog_rom.c$(DependSuffix) -MM basicd/bprog_rom.c
	$(CC) $(SourceSwitch) "/Users/sergey/projloc/rimer/fw/basicd/bprog_rom.c" $(CFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/basicd_bprog_rom.c$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/basicd_bprog_rom.c$(PreprocessSuffix): basicd/bprog_rom.c
	$(CC) $(CFLAGS) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/basicd_bprog_rom.c$(PreprocessSuffix) basicd/bprog_rom.c

$(IntermediateDirectory)/basicd_bcore.c$(ObjectSuffix): basicd/bcore.c
	@$(CC) $(CFLAGS) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/basicd_bcore.c$(ObjectSuffix) -MF$(IntermediateDirectory)/basicd_bcore.c$(DependSuffix) -MM basicd/bcore.c
	$(CC) $(SourceSwitch) "/Users/sergey/projloc/rimer/fw/basicd/bcore.c" $(CFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/basicd_bcore.c$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/basicd_bcore.c$(PreprocessSuffix): basicd/bcore.c
	$(CC) $(CFLAGS) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/basicd_bcore.c$(PreprocessSuffix) basicd/bcore.c

$(IntermediateDirectory)/basicd_bstring.c$(ObjectSuffix): basicd/bstring.c
	@$(CC) $(CFLAGS) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/basicd_bstring.c$(ObjectSuffix) -MF$(IntermediateDirectory)/basicd_bstring.c$(DependSuffix) -MM basicd/bstring.c
	$(CC) $(SourceSwitch) "/Users/sergey/projloc/rimer/fw/basicd/bstring.c" $(CFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/basicd_bstring.c$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/basicd_bstring.c$(PreprocessSuffix): basicd/bstring.c
	$(CC) $(CFLAGS) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/basicd_bstring.c$(PreprocessSuffix) basicd/bstring.c

$(IntermediateDirectory)/iface_iface_sio.c$(ObjectSuffix): iface/iface_sio.c
	@$(CC) $(CFLAGS) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/iface_iface_sio.c$(ObjectSuffix) -MF$(IntermediateDirectory)/iface_iface_sio.c$(DependSuffix) -MM iface/iface_sio.c
	$(CC) $(SourceSwitch) "/Users/sergey/projloc/rimer/fw/iface/iface_sio.c" $(CFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/iface_iface_sio.c$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/iface_iface_sio.c$(PreprocessSuffix): iface/iface_sio.c
	$(CC) $(CFLAGS) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/iface_iface_sio.c$(PreprocessSuffix) iface/iface_sio.c

$(IntermediateDirectory)/basicd_berror.c$(ObjectSuffix): basicd/berror.c
	@$(CC) $(CFLAGS) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/basicd_berror.c$(ObjectSuffix) -MF$(IntermediateDirectory)/basicd_berror.c$(DependSuffix) -MM basicd/berror.c
	$(CC) $(SourceSwitch) "/Users/sergey/projloc/rimer/fw/basicd/berror.c" $(CFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/basicd_berror.c$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/basicd_berror.c$(PreprocessSuffix): basicd/berror.c
	$(CC) $(CFLAGS) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/basicd_berror.c$(PreprocessSuffix) basicd/berror.c

$(IntermediateDirectory)/zx80_snapshot.c$(ObjectSuffix): zx80/snapshot.c
	@$(CC) $(CFLAGS) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/zx80_snapshot.c$(ObjectSuffix) -MF$(IntermediateDirectory)/zx80_snapshot.c$(DependSuffix) -MM zx80/snapshot.c
	$(CC) $(SourceSwitch) "/Users/sergey/projloc/rimer/fw/zx80/snapshot.c" $(CFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/zx80_snapshot.c$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/zx80_snapshot.c$(PreprocessSuffix): zx80/snapshot.c
	$(CC) $(CFLAGS) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/zx80_snapshot.c$(PreprocessSuffix) zx80/snapshot.c

$(IntermediateDirectory)/iface_iface_sd.c$(ObjectSuffix): iface/iface_sd.c
	@$(CC) $(CFLAGS) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/iface_iface_sd.c$(ObjectSuffix) -MF$(IntermediateDirectory)/iface_iface_sd.c$(DependSuffix) -MM iface/iface_sd.c
	$(CC) $(SourceSwitch) "/Users/sergey/projloc/rimer/fw/iface/iface_sd.c" $(CFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/iface_iface_sd.c$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/iface_iface_sd.c$(PreprocessSuffix): iface/iface_sd.c
	$(CC) $(CFLAGS) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/iface_iface_sd.c$(PreprocessSuffix) iface/iface_sd.c

$(IntermediateDirectory)/zx80_zxscreen.c$(ObjectSuffix): zx80/zxscreen.c
	@$(CC) $(CFLAGS) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/zx80_zxscreen.c$(ObjectSuffix) -MF$(IntermediateDirectory)/zx80_zxscreen.c$(DependSuffix) -MM zx80/zxscreen.c
	$(CC) $(SourceSwitch) "/Users/sergey/projloc/rimer/fw/zx80/zxscreen.c" $(CFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/zx80_zxscreen.c$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/zx80_zxscreen.c$(PreprocessSuffix): zx80/zxscreen.c
	$(CC) $(CFLAGS) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/zx80_zxscreen.c$(PreprocessSuffix) zx80/zxscreen.c

$(IntermediateDirectory)/zx80_z80dbg.c$(ObjectSuffix): zx80/z80dbg.c
	@$(CC) $(CFLAGS) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/zx80_z80dbg.c$(ObjectSuffix) -MF$(IntermediateDirectory)/zx80_z80dbg.c$(DependSuffix) -MM zx80/z80dbg.c
	$(CC) $(SourceSwitch) "/Users/sergey/projloc/rimer/fw/zx80/z80dbg.c" $(CFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/zx80_z80dbg.c$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/zx80_z80dbg.c$(PreprocessSuffix): zx80/z80dbg.c
	$(CC) $(CFLAGS) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/zx80_z80dbg.c$(PreprocessSuffix) zx80/z80dbg.c

$(IntermediateDirectory)/basicd_bscreen.c$(ObjectSuffix): basicd/bscreen.c
	@$(CC) $(CFLAGS) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/basicd_bscreen.c$(ObjectSuffix) -MF$(IntermediateDirectory)/basicd_bscreen.c$(DependSuffix) -MM basicd/bscreen.c
	$(CC) $(SourceSwitch) "/Users/sergey/projloc/rimer/fw/basicd/bscreen.c" $(CFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/basicd_bscreen.c$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/basicd_bscreen.c$(PreprocessSuffix): basicd/bscreen.c
	$(CC) $(CFLAGS) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/basicd_bscreen.c$(PreprocessSuffix) basicd/bscreen.c

$(IntermediateDirectory)/iface_iface_aio.c$(ObjectSuffix): iface/iface_aio.c
	@$(CC) $(CFLAGS) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/iface_iface_aio.c$(ObjectSuffix) -MF$(IntermediateDirectory)/iface_iface_aio.c$(DependSuffix) -MM iface/iface_aio.c
	$(CC) $(SourceSwitch) "/Users/sergey/projloc/rimer/fw/iface/iface_aio.c" $(CFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/iface_iface_aio.c$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/iface_iface_aio.c$(PreprocessSuffix): iface/iface_aio.c
	$(CC) $(CFLAGS) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/iface_iface_aio.c$(PreprocessSuffix) iface/iface_aio.c

$(IntermediateDirectory)/iface_iface_zx80.c$(ObjectSuffix): iface/iface_zx80.c
	@$(CC) $(CFLAGS) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/iface_iface_zx80.c$(ObjectSuffix) -MF$(IntermediateDirectory)/iface_iface_zx80.c$(DependSuffix) -MM iface/iface_zx80.c
	$(CC) $(SourceSwitch) "/Users/sergey/projloc/rimer/fw/iface/iface_zx80.c" $(CFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/iface_iface_zx80.c$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/iface_iface_zx80.c$(PreprocessSuffix): iface/iface_zx80.c
	$(CC) $(CFLAGS) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/iface_iface_zx80.c$(PreprocessSuffix) iface/iface_zx80.c

$(IntermediateDirectory)/basicd_banalizer.c$(ObjectSuffix): basicd/banalizer.c
	@$(CC) $(CFLAGS) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/basicd_banalizer.c$(ObjectSuffix) -MF$(IntermediateDirectory)/basicd_banalizer.c$(DependSuffix) -MM basicd/banalizer.c
	$(CC) $(SourceSwitch) "/Users/sergey/projloc/rimer/fw/basicd/banalizer.c" $(CFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/basicd_banalizer.c$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/basicd_banalizer.c$(PreprocessSuffix): basicd/banalizer.c
	$(CC) $(CFLAGS) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/basicd_banalizer.c$(PreprocessSuffix) basicd/banalizer.c

$(IntermediateDirectory)/iface_iface_set.c$(ObjectSuffix): iface/iface_set.c
	@$(CC) $(CFLAGS) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/iface_iface_set.c$(ObjectSuffix) -MF$(IntermediateDirectory)/iface_iface_set.c$(DependSuffix) -MM iface/iface_set.c
	$(CC) $(SourceSwitch) "/Users/sergey/projloc/rimer/fw/iface/iface_set.c" $(CFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/iface_iface_set.c$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/iface_iface_set.c$(PreprocessSuffix): iface/iface_set.c
	$(CC) $(CFLAGS) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/iface_iface_set.c$(PreprocessSuffix) iface/iface_set.c

$(IntermediateDirectory)/iface_enums.c$(ObjectSuffix): iface/enums.c
	@$(CC) $(CFLAGS) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/iface_enums.c$(ObjectSuffix) -MF$(IntermediateDirectory)/iface_enums.c$(DependSuffix) -MM iface/enums.c
	$(CC) $(SourceSwitch) "/Users/sergey/projloc/rimer/fw/iface/enums.c" $(CFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/iface_enums.c$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/iface_enums.c$(PreprocessSuffix): iface/enums.c
	$(CC) $(CFLAGS) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/iface_enums.c$(PreprocessSuffix) iface/enums.c

$(IntermediateDirectory)/iface_iface_dio.c$(ObjectSuffix): iface/iface_dio.c
	@$(CC) $(CFLAGS) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/iface_iface_dio.c$(ObjectSuffix) -MF$(IntermediateDirectory)/iface_iface_dio.c$(DependSuffix) -MM iface/iface_dio.c
	$(CC) $(SourceSwitch) "/Users/sergey/projloc/rimer/fw/iface/iface_dio.c" $(CFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/iface_iface_dio.c$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/iface_iface_dio.c$(PreprocessSuffix): iface/iface_dio.c
	$(CC) $(CFLAGS) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/iface_iface_dio.c$(PreprocessSuffix) iface/iface_dio.c

$(IntermediateDirectory)/basicd_bmath.c$(ObjectSuffix): basicd/bmath.c
	@$(CC) $(CFLAGS) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/basicd_bmath.c$(ObjectSuffix) -MF$(IntermediateDirectory)/basicd_bmath.c$(DependSuffix) -MM basicd/bmath.c
	$(CC) $(SourceSwitch) "/Users/sergey/projloc/rimer/fw/basicd/bmath.c" $(CFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/basicd_bmath.c$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/basicd_bmath.c$(PreprocessSuffix): basicd/bmath.c
	$(CC) $(CFLAGS) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/basicd_bmath.c$(PreprocessSuffix) basicd/bmath.c

$(IntermediateDirectory)/iface_iface_bas.c$(ObjectSuffix): iface/iface_bas.c
	@$(CC) $(CFLAGS) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/iface_iface_bas.c$(ObjectSuffix) -MF$(IntermediateDirectory)/iface_iface_bas.c$(DependSuffix) -MM iface/iface_bas.c
	$(CC) $(SourceSwitch) "/Users/sergey/projloc/rimer/fw/iface/iface_bas.c" $(CFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/iface_iface_bas.c$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/iface_iface_bas.c$(PreprocessSuffix): iface/iface_bas.c
	$(CC) $(CFLAGS) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/iface_iface_bas.c$(PreprocessSuffix) iface/iface_bas.c


-include $(IntermediateDirectory)/*$(DependSuffix)
##
## Clean
##
clean:
	$(RM) -r $(ConfigurationName)/


