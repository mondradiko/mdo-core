set(VCPKG_LINKER_FLAGS "-Wl,-rpath,'$ORIGIN/../../lib'")

vcpkg_from_github(
  OUT_SOURCE_PATH SOURCE_PATH
  REPO WAVM/WAVM
  REF nightly/2021-05-10
  HEAD_REF master
  SHA512 9c3089b48ebb612eeef5c387ba6beea7f187a73cc711f5949123f9592043fc09797873453bad49105659df156a2e3366d9801712d4383b830ff492a75f1bc014
)

vcpkg_configure_cmake(
  SOURCE_PATH ${SOURCE_PATH}
  PREFER_NINJA
  OPTIONS
  -DWAVM_ENABLE_STATIC_LINKING=ON
  -DWAVM_ENABLE_FUZZ_TARGETS=OFF
)

vcpkg_install_cmake()

file(
  INSTALL ${SOURCE_PATH}/LICENSE.txt
  DESTINATION ${CURRENT_PACKAGES_DIR}/share/${PORT}
  RENAME copyright
)

file(MAKE_DIRECTORY ${CURRENT_PACKAGES_DIR}/tools)
file(RENAME "${CURRENT_PACKAGES_DIR}/bin" "${CURRENT_PACKAGES_DIR}/tools/${PORT}")

vcpkg_fixup_cmake_targets(
  CONFIG_PATH "lib/cmake/WAVM"
	TARGET_PATH "share/wavm"
)

vcpkg_copy_pdbs()
