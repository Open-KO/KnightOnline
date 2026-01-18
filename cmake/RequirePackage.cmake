function(require_package package_name)
  message(STATUS "OpenKO: [Find${package_name}] Checking and fetching... this might take some time.")

  find_package(${package_name} REQUIRED)

  message(STATUS "OpenKO: [Find${package_name}] Up-to-date!")
endfunction()

function(require_config_package package_name)
  message(STATUS "OpenKO: [${package_name}-config] Checking and fetching... this might take some time.")

  set(${package_name}_DIR "${OPENKO_MODULE_PATH}/${package_name}")
  find_package(${package_name} CONFIG REQUIRED)

  message(STATUS "OpenKO: [${package_name}-config] Up-to-date!")
endfunction()
