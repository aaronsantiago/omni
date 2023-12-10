# Set name of the module
SET (MODULE ndi)

# Set all source files module uses
SET (SRC NDIInput.cpp
		 NDIInput.h
		 NDIOutput.cpp
		 NDIOutput.h
		 utils.cpp
		 utils.h
		 register.cpp)

add_library(${MODULE} MODULE ${SRC})
include_directories(${NDI_INCLUDE_DIRS}) 
target_link_libraries(${MODULE} ${LIBNAME})

YURI_INSTALL_MODULE(${MODULE})