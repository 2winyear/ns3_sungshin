include("/mnt/hdd/ylim/ns-allinone-3.36.1/ns-3.36.1/vcpkg/scripts/buildsystems/vcpkg.cmake")

#find_package(jsoncpp CONFIG REQUIRED)
find_path(JSONCPP_INCLUDE_DIR json/json.h)
find_library(JSONCPP jsoncpp)

build_example(
  NAME wifi-mlms
  SOURCE_FILES mlms.cc WiFiEnvironment.cc Configuration.cc CustomTrafficGenerator.cc
  LIBRARIES_TO_LINK
          ${JSONCPP}
	  ${libwifi}
          ${libflow-monitor}
          ${libapplications}
)

target_include_directories(wifi-mlms PRIVATE ${JSONCPP_INCLUDE_DIR})
