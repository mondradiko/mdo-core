vcpkg_from_github(
  OUT_SOURCE_PATH SOURCE_PATH
  REPO SanderMertens/flecs
  REF v2.3.2
  HEAD_REF master
  SHA512 0e01716dbf15ca9dc9a10552943d0fd09fbe2dad2feb4ee04c677e14a965f93e481bad8b8352867488432e941344cec9aae841f0012d73a3b37b5b9a1e368455
)

configure_file(
  "${SOURCE_PATH}/LICENSE"
  "${CURRENT_PACKAGES_DIR}/share/flecs/copyright"
  COPYONLY
)

vcpkg_configure_cmake(
  SOURCE_PATH ${SOURCE_PATH}
	PREFER_NINJA
)

vcpkg_install_cmake()

vcpkg_copy_pdbs()

vcpkg_fixup_cmake_targets(CONFIG_PATH lib/cmake/flecs)
