vcpkg_from_github(
	OUT_SOURCE_PATH SOURCE_PATH
	REPO recp/cglm
	REF v0.8.3
	SHA512 71062814637e2523b2632980384fa58a639cdc1bbe8b647e7826fe334a53cd880bd5b29baac7d5eaff2bb009a97830c76b239e7273af46908ad7ab52cc8b8173
)

vcpkg_configure_cmake(
	SOURCE_PATH ${SOURCE_PATH}
	PREFER_NINJA
	OPTIONS
	-DCGLM_STATIC=ON
)

vcpkg_install_cmake()

vcpkg_fixup_cmake_targets(CONFIG_PATH lib/cmake/cglm)

file(
	INSTALL "${SOURCE_PATH}/LICENSE"
  DESTINATION "${CURRENT_PACKAGES_DIR}/share/cglm"
	RENAME copyright
)
