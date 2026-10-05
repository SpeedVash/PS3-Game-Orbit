.SUFFIXES:

ifeq ($(strip $(PSL1GHT)),)
$(error "Please set PSL1GHT. Example: export PSL1GHT=$$PS3DEV")
endif

# Version 1.3.3 uses the renderer and presentation sequence approved in 1.2.
TARGET := ps3_game_orbit_fix36
TITLE := PS3 Game Orbit
APPID := PGORBT301
CONTENTID := UP0001-$(APPID)_00-0000000000000000
ICON0 := pkgfiles/ICON0.PNG
PKGFILES := pkgfiles
SFOXML := assets/PARAM_1_3_3.xml

include $(PSL1GHT)/ppu_rules

SOURCES := src
SHADERS := shaders
INCLUDES := include

CPPFILES := $(filter-out $(SOURCES)/webman_client.cpp,$(wildcard $(SOURCES)/*.cpp))
SHADER_VPO := $(SHADERS)/v14_case.vpo
SHADER_FPO := $(SHADERS)/v14_case.fpo
SHADER_OBJS := $(SHADER_VPO).o $(SHADER_FPO).o
OFILES := $(CPPFILES:.cpp=.o) $(SHADER_OBJS)

ELF_TARGET := $(TARGET).elf
SELF_TARGET := $(TARGET).self
PKG_TARGET := $(TARGET).pkg

CXXFLAGS += -std=gnu++17 -O2 -Wall -Wextra -Werror -MMD -MP -mcpu=cell -D__CELLOS_LV2__ -D__PSL1GHT__ -DPS3_SP_LOADER_FIX28=1 -DPS3_SP_LOADER_FIX29=1 -DPS3_GAME_ORBIT_FIX30=1 \
            -DPS3_GAME_ORBIT_FIX31=1 -DPS3_GAME_ORBIT_FIX32=1 -DPS3_GAME_ORBIT_FIX33=1 -DPS3_GAME_ORBIT_FIX34=1 -DPS3_GAME_ORBIT_FIX35=1 -DPS3_GAME_ORBIT_FIX36=1 $(LIBPSL1GHT_INC) -I$(INCLUDES)
LDFLAGS += $(MACHDEP) -Wl,-Map,$(TARGET).map
LIBPATHS += $(LIBPSL1GHT_LIB)
LIBS += -lrsx -lgcm_sys -lrt -lnet -lio -lsysutil -lsysmodule -lm -ljpgdec -lpngdec -llv2
CGCFLAGS += -Wcg,-strict

.PHONY: all self pkg clean info preflight api-audit verify-v14 verify-shaders host-test
all: $(SELF_TARGET)
self: $(SELF_TARGET)
pkg: $(PKG_TARGET)
preflight:
	@./scripts/preflight_v13.sh
api-audit:
	@./scripts/audit_psl1ght_api_v13.sh
verify-v14:
	@./scripts/verify_v14_frozen_v13.sh
verify-shaders:
	@sha256sum -c shaders/FROZEN_SHADER_SHA256.txt
host-test:
	@./tests/run_host_tests.sh

# Explicit C++ link rule. The generic PSL1GHT rule defaults to LD; this project always links C++.
$(ELF_TARGET): $(OFILES) | verify-v14 verify-shaders
	@echo linking ... $@
	$(CXX) $(OFILES) $(LDFLAGS) $(LIBPATHS) $(LIBS) -o $@

# Reuse the exact archived V13 shader blobs; FIX26 does not regenerate them.
$(SHADER_VPO) $(SHADER_FPO):
	@echo "ERROR: archived shader $@ is missing; restore it from this source package."
	@exit 3

-include $(CPPFILES:.cpp=.d)

clean:
	rm -f $(SOURCES)/*.o $(SOURCES)/*.d $(SHADERS)/*.vpo.o $(SHADERS)/*.fpo.o v14_case_vpo.h v14_case_fpo.h
	rm -f $(ELF_TARGET) $(SELF_TARGET) $(TARGET).fake.self $(PKG_TARGET) $(TARGET).gnpdrm.pkg $(TARGET).map
	rm -rf build

# Clean keeps the frozen shader blobs. clean-shaders is an explicit removal only.
.PHONY: clean-shaders
clean-shaders:
	rm -f $(SHADER_VPO) $(SHADER_FPO) $(SHADER_OBJS)

info:
	@echo PS3DEV=$(PS3DEV)
	@echo PSL1GHT=$(PSL1GHT)
	@echo CXX=$(CXX)
	@echo CGCOMP=$(CGCOMP)
	@echo TARGET=$(TARGET)
	@echo APPID=$(APPID)
	@echo CONTENTID=$(CONTENTID)
	@echo SHADER_VPO=$(SHADER_VPO)
	@echo SHADER_FPO=$(SHADER_FPO)
