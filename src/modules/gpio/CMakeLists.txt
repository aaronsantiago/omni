# Set name of the module
SET (MODULE gpio)

IF(YURI_PLATFORM_LINUX)

# Set all source files module uses
SET (SRC register.cpp
         GPIO.cpp
         GPIO.h
         )
 
add_library(${MODULE} MODULE ${SRC})
target_link_libraries(${MODULE} ${LIBNAME})

YURI_INSTALL_MODULE(${MODULE})

ENDIF()