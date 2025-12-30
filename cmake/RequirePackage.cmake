function(require_package package_name)
  # Set default
  set(dependency_name "${package_name}")

  if(ARGC GREATER 1)
    list(GET ARGV 1 dependency_name)
  endif()

  message(STATUS "OpenKO: [${dependency_name}] Checking and fetching...")
  find_package(${package_name} REQUIRED)
  message(STATUS "OpenKO: [${dependency_name}] Up-to-date!")
endfunction()
