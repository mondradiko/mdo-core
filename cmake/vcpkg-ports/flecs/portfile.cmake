vcpkg_from_github(
  OUT_SOURCE_PATH SOURCE_PATH
  REPO SanderMertens/flecs
  REF 19b9d359b6e5eb1b319774e8e8b434be2fe9713d
  HEAD_REF master
  SHA512 1ff50ffef136a70cec09882daaaa8950f365c22667123d562dc1ec34d9005d162b36ba3f1d970e6650ed4dfbc2ee3691546b53427b8cd55403e25b4453df6308
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
