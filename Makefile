.SUFFIXES:

ifeq ($(strip $(PSL1GHT)),)
$(error "Please set PSL1GHT. Example: export PSL1GHT=$$PS3DEV")
endif

# Version 1.9.0 adds PS1 cases/multidisc; native PKG189 and resident16 remain frozen.
TARGET := ps3_game_orbit_fix48
TITLE := PS3 Game Orbit
APPID := PGORBT301
CONTENTID := UP0001-$(APPID)_00-0000000000000000
ICON0 := pkgfiles/ICON0.PNG
PKGFILES := pkgfiles
SFOXML := assets/PARAM_2_1_3.xml

include $(PSL1GHT)/ppu_rules

SOURCES := src
SHADERS := shaders
CXXFLAGS += -DORBIT_FEATURE_NEXT=1 -DORBIT_PS1_190=1
INCLUDES := include

CPPFILES := $(filter-out $(SOURCES)/webman_client.cpp $(SOURCES)/file_manager_fix40.cpp $(SOURCES)/file_manager_fix41.cpp $(SOURCES)/file_manager_fix42.cpp $(SOURCES)/plugin_probe04.cpp $(SOURCES)/service_probe12.cpp $(SOURCES)/service_probe13.cpp $(SOURCES)/service_probe15.cpp $(SOURCES)/ntfs_catalog_fix48.cpp,$(wildcard $(SOURCES)/*.cpp))
SHADER_VPO := $(SHADERS)/v14_case.vpo
SHADER_FPO := $(SHADERS)/v14_case.fpo
SHADER_OBJS := $(SHADER_VPO).o $(SHADER_FPO).o
CFILES := vendor/fatfs/ff.c vendor/fatfs/ffunicode.c
OFILES := $(CFILES:.c=.o) $(CPPFILES:.cpp=.o) $(SHADER_OBJS)

ELF_TARGET := $(TARGET).elf
SELF_TARGET := $(TARGET).self
PKG_TARGET := $(TARGET).pkg

CXXFLAGS += -DORBIT_DIAGNOSTIC_TEST03=1 -DORBIT_DIAGNOSTIC_TEST04=1 -DORBIT_DIAGNOSTIC_TEST13=1 -DORBIT_DIAGNOSTIC_TEST14=1 -DORBIT_DIAGNOSTIC_TEST15=1 -DORBIT_FEATURE_FAN16=1 -std=gnu++17 -O2 -Wall -Wextra -Werror -MMD -MP -mcpu=cell -D__CELLOS_LV2__ -D__PSL1GHT__ -DPS3_SP_LOADER_FIX28=1 -DPS3_SP_LOADER_FIX29=1 -DPS3_GAME_ORBIT_FIX30=1 \
            -DPS3_GAME_ORBIT_FIX31=1 -DPS3_GAME_ORBIT_FIX32=1 -DPS3_GAME_ORBIT_FIX33=1 -DPS3_GAME_ORBIT_FIX34=1 -DPS3_GAME_ORBIT_FIX35=1 -DPS3_GAME_ORBIT_FIX36=1 -DPS3_GAME_ORBIT_FIX37=1 -DPS3_GAME_ORBIT_FIX38=1 -DPS3_GAME_ORBIT_FIX39=1 -DPS3_GAME_ORBIT_FIX40=1 -DPS3_GAME_ORBIT_FIX41=1 -DPS3_GAME_ORBIT_FIX42=1 -DPS3_GAME_ORBIT_FIX43=1 -DPS3_GAME_ORBIT_FIX44=1 -DPS3_GAME_ORBIT_FIX45=1 -DPS3_GAME_ORBIT_FIX46=1 -DPS3_GAME_ORBIT_FIX47=1 -DPS3_GAME_ORBIT_FIX48=1 $(LIBPSL1GHT_INC) -I$(INCLUDES) -Ivendor/fatfs
CFLAGS += -std=gnu99 -O2 -Wall -Wextra -Werror -mcpu=cell -D__CELLOS_LV2__ -D__PSL1GHT__ $(LIBPSL1GHT_INC) -Ivendor/fatfs -Ivendor/file-manager17 -Iinclude
LDFLAGS += $(MACHDEP) -Wl,-Map,$(TARGET).map
LIBPATHS += $(LIBPSL1GHT_LIB)
LIBS += -lrsx -lgcm_sys -lrt -lnet -lio -lsysutil -lsysmodule -lm -ljpgdec -lpngdec -lsysfs -llv2
CGCFLAGS += -Wcg,-strict

.PHONY: all self pkg clean info preflight api-audit verify-v14 verify-shaders host-test
all: $(SELF_TARGET)
self: $(SELF_TARGET)
pkg: $(PKG_TARGET)
# The resident remains frozen; always rebuild package staging to drop obsolete files.
.PHONY: pkg_stage180
pkg_stage180:
	@python3 -c 'from pathlib import Path; import shutil; p=Path("build/pkg"); shutil.rmtree(p) if p.exists() else None'
$(PKG_TARGET): $(PKGFILES)/USRDIR/orbit_pkg189.sprx pkg_stage180 $(SFOXML) $(ICON0) $(PKGFILES)/USRDIR/orbit_service16.sprx $(wildcard $(PKGFILES)/* $(PKGFILES)/USRDIR/*)
$(PKGFILES)/USRDIR/orbit_service16.sprx: $(wildcard plugin/service16/*) plugin/raw_reader.c plugin/fan_policy.c include/fan_policy_fix48.h include/raw_storage15.h include/raw_reader_fix48.h plugin/file_ipc.h plugin/monotonic_clock.h plugin/thread_bootstrap.h plugin/thread_trace12.h include/service_probe16_protocol.h include/service_probe16_paths.h include/orbit_service_protocol_fix48.h scripts/build-service16.sh scripts/make-orbit-prx-fix48.py scripts/verify-service16.py scripts/verify-signed-probe07.py tests/check_service16_flags.py tests/check_service16_messages.py tests/test_service16_opd.py tests/audit_native_bridge15.py
	@bash scripts/build-service16.sh
$(PKGFILES)/USRDIR/orbit_pkg189.sprx: $(wildcard plugin/pkg189/*) include/pkg_handoff189_protocol.h scripts/build-pkg189.sh scripts/make-orbit-prx189.py
	@bash scripts/build-pkg189.sh
preflight:
	@bash scripts/preflight_v13.sh
api-audit:
	@bash scripts/audit_psl1ght_api_v13.sh
verify-v14:
	@bash scripts/verify_v14_frozen_v13.sh
verify-shaders:
	@sha256sum -c shaders/FROZEN_SHADER_SHA256.txt
host-test:
	@bash tests/run_host_tests.sh

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
	rm -f $(PKGFILES)/USRDIR/orbit_service.sprx

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

