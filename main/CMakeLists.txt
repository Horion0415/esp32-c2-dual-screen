idf_component_register(
    SRC_DIRS
        "."
        "1_28_UI"
    INCLUDE_DIRS
        "."
        )

target_compile_options(${COMPONENT_LIB} PRIVATE  -Wno-implicit-function-declaration -Wno-stringop-truncation)

set(Drive_A "./out1")
set(Drive_B "./out2")

spiffs_create_partition_assets(
    assets_B
    ${Drive_B}
    FLASH_IN_PROJECT
    MMAP_FILE_SUPPORT_FORMAT ".png,.jpg"
    MMAP_SUPPORT_QOI
    MMAP_SUPPORT_SQOI
    MMAP_SPLIT_HEIGHT 4)

spiffs_create_partition_assets(
    assets_A
    ${Drive_A}
    FLASH_IN_PROJECT
    MMAP_FILE_SUPPORT_FORMAT ".png,.jpg"
    MMAP_SUPPORT_QOI
    MMAP_SUPPORT_SQOI
    MMAP_SPLIT_HEIGHT 4)
