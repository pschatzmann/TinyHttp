# Build htmldump for linux
add_definitions(-DPLATFORM_LINUX)

## build htmldump.ino
add_executable(htmldump template.cpp)
target_compile_options(htmldump PUBLIC -include../examples/client/client.ino)
target_compile_features(htmldump PRIVATE cxx_std_17)
target_link_libraries(htmldump PRIVATE tinyhttp_library fmt::fmt)

## build server.ino
#add_executable(server template.cpp)
#target_compile_options(server PUBLIC -include../examples/server/server.ino)
#target_compile_features(server PRIVATE cxx_std_17)
#target_link_libraries(server PRIVATE tinyhttp_library fmt::fmt)