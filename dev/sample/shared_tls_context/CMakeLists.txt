set(SAMPLE sample.shared_tls_context)
include(${CMAKE_SOURCE_DIR}/cmake/sample.cmake)

TARGET_INCLUDE_DIRECTORIES(${SAMPLE} PRIVATE ${OPENSSL_INCLUDE_DIR})
TARGET_LINK_LIBRARIES(${SAMPLE} PRIVATE ${OPENSSL_LIBRARIES})

# NOTE: in the case of RESTINIO_INSTALL_SAMPLES *.pem files will
# be installed with hello_world_https example.
