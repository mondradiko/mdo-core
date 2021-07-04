vcpkg_from_github(
	OUT_SOURCE_PATH SOURCE_PATH
	REPO WAVM/WAVM
	REF nightly/2021-05-10
	HEAD_REF master
)

vcpkg_configure_cmake(
	SOURCE_PATH ${SOURCE_PATH}
	PREFER_NINJA
)

vcpkg_install_cmake()
