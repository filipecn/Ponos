link_libraries(${GLFW_LIB} ${VULKAN_LIBRARY})

link_libraries(poseidon)
include_directories("${POSEIDON_SOURCE_DIR}/src")
include_directories("${POSEIDON_SOURCE_DIR}/external")
link_libraries(ponos)
include_directories("${PONOS_SOURCE_DIR}/src")
link_libraries(aergia)
include_directories("${AERGIA_SOURCE_DIR}/src")

add_executable(main flip_example.cpp flip.h flip.cpp)
include_directories("/home/filipecn/gitclones/OpenVDB/include")
  include_directories("/usr/local/OpenVDB/4.0.0/include")
target_link_libraries(main ${aergia_LIBRARIES} -ltbb -lpthread -lHalf
	/usr/local/OpenVDB/4.0.0/lib/libopenvdb.so.4.0.1
	/usr/local/OpenVDB/4.0.0/lib/libopenvdb.so.4.0.0
	#	/home/filipecn/gitclones/OpenVDB/lib/libopenvdb.so
	#/home/filipecn/gitclones/OpenVDB/lib/libopenvdb_points.so
	)
