# Android specific build targets
PATH_DIST = $(srcdir)/dists/android

GRADLE_FILES = $(shell find $(PATH_DIST)/gradle -type f) $(PATH_DIST)/gradlew $(PATH_DIST)/build.gradle

PATH_BUILD = ./android_project
PATH_BUILD_GRADLE = $(PATH_BUILD)/build.gradle
PATH_BUILD_ASSETS = $(PATH_BUILD)/assets
PATH_BUILD_LIB = $(PATH_BUILD)/lib/$(ABI)
PATH_BUILD_LIBSCUMMVM = $(PATH_BUILD)/lib/$(ABI)/libscummvm.so

APK_MAIN = ScummVM-debug.apk
APK_MAIN_RELEASE = ScummVM-release-unsigned.apk
AAB_MAIN = ScummVM-debug.aab
AAB_MAIN_RELEASE = ScummVM-release-unsigned.aab

$(PATH_BUILD):
	$(MKDIR) $(PATH_BUILD)

$(PATH_BUILD_GRADLE): $(GRADLE_FILES) | $(PATH_BUILD)
	$(CP) -r $(PATH_DIST)/gradle/ $(PATH_BUILD)/gradle/
	$(INSTALL) -c -m 755 $(PATH_DIST)/gradlew $(PATH_BUILD)
	$(INSTALL) -c -m 644 $(PATH_DIST)/build.gradle $(PATH_BUILD)
	$(ECHO) "srcdir=$(realpath $(srcdir))\n" > $(PATH_BUILD)/gradle.properties
	$(ECHO) "org.gradle.jvmargs=-Xmx4096m\n" >> $(PATH_BUILD)/gradle.properties
	$(ECHO) "android.useAndroidX=true\n" >> $(PATH_BUILD)/gradle.properties
	$(ECHO) "android.enableJetifier=true\n" >> $(PATH_BUILD)/gradle.properties
	$(ECHO) "sdk.dir=$(realpath $(ANDROID_SDK_ROOT))\n" > $(PATH_BUILD)/local.properties

$(PATH_BUILD_ASSETS): $(DIST_FILES_THEMES) $(DIST_FILES_ENGINEDATA) $(DIST_FILES_NETWORKING) $(DIST_FILES_VKEYBD) $(DIST_FILES_DOCS) | $(PATH_BUILD)
	$(INSTALL) -d $(PATH_BUILD_ASSETS)
	$(INSTALL) -c -m 644 $(DIST_FILES_THEMES) $(DIST_FILES_ENGINEDATA) $(DIST_FILES_NETWORKING) $(DIST_FILES_VKEYBD) $(DIST_FILES_DOCS) $(PATH_BUILD_ASSETS)/
ifneq ($(DIST_FILES_SHADERS),)
	$(INSTALL) -d $(PATH_BUILD_ASSETS)/shaders
	$(INSTALL) -c -m 644 $(DIST_FILES_SHADERS) $(PATH_BUILD_ASSETS)/shaders
endif

$(PATH_BUILD_LIBSCUMMVM): libscummvm.so | $(PATH_BUILD)
	$(INSTALL) -d  $(PATH_BUILD_LIB)
	$(INSTALL) -c -m 644 libscummvm.so $(PATH_BUILD_LIBSCUMMVM)

$(APK_MAIN): $(PATH_BUILD_GRADLE) $(PATH_BUILD_ASSETS) $(PATH_BUILD_LIBSCUMMVM) | $(PATH_BUILD)
	(cd $(PATH_BUILD); ./gradlew assembleDebug)
	$(CP) $(PATH_BUILD)/build/outputs/apk/debug/$(APK_MAIN) $@

$(APK_MAIN_RELEASE): $(PATH_BUILD_GRADLE) $(PATH_BUILD_ASSETS) $(PATH_BUILD_LIBSCUMMVM) | $(PATH_BUILD)
	(cd $(PATH_BUILD); ./gradlew assembleRelease)
	$(CP) $(PATH_BUILD)/build/outputs/apk/release/$(APK_MAIN_RELEASE) $@

$(AAB_MAIN): $(PATH_BUILD_GRADLE) $(PATH_BUILD_ASSETS) $(PATH_BUILD_LIBSCUMMVM) | $(PATH_BUILD)
	(cd $(PATH_BUILD); ./gradlew bundleDebug)
	$(CP) $(PATH_BUILD)/build/outputs/bundle/debug/$(AAB_MAIN) $@
	
$(AAB_MAIN_RELEASE): $(PATH_BUILD_GRADLE) $(PATH_BUILD_ASSETS) $(PATH_BUILD_LIBSCUMMVM) | $(PATH_BUILD)
	(cd $(PATH_BUILD); ./gradlew bundleRelease)
	$(CP) $(PATH_BUILD)/build/outputs/bundle/release/$(AAB_MAIN_RELEASE) $@


all: $(AAB_MAIN)

clean: androidclean

androidclean:
	@$(RM) -rf $(PATH_BUILD) *.apk
	@$(RM) -rf $(PATH_BUILD) *.aab

androidrelease: $(AAB_MAIN_RELEASE)

androidtestmain: $(APK_MAIN)
	(cd $(PATH_BUILD); ./gradlew installDebug)
	# $(ADB) install -g -r $(APK_MAIN)
	# $(ADB) shell am start -a android.intent.action.MAIN -c android.intent.category.LAUNCHER -n org.scummvm.scummvm/.ScummVMActivity

androidtest: $(APK_MAIN)
	# @set -e; for apk in $^; do \
	# 	$(ADB) install -g -r $$apk; \
	# done
	# $(ADB) shell am start -a android.intent.action.MAIN -c android.intent.category.LAUNCHER -n org.scummvm.scummvm/.ScummVMActivity
	(cd $(PATH_BUILD); ./gradlew installDebug)

# used by buildbot!
androiddistdebug: $(AAB_MAIN)
	$(MKDIR) debug
	$(CP) $(AAB_MAIN) debug/
	for i in $(DIST_FILES_DOCS); do \
		sed 's/$$/\r/' < $$i > debug/`basename $$i`.txt; \
	done

androiddistrelease: $(AAB_MAIN_RELEASE)
	$(MKDIR) release
	$(CP) $(AAB_MAIN_RELEASE) release/
	for i in $(DIST_FILES_DOCS); do \
		sed 's/$$/\r/' < $$i > release/`basename $$i`.txt; \
	done

.PHONY: androidrelease androidtest $(PATH_BUILD_SRC)
