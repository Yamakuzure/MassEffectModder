TEMPLATE = subdirs

CONFIG += ordered

WRAPPERS_SHARED = true
cache(WRAPPERS_SHARED, set)

ZSTD_ENABLE = false
cache(ZSTD_ENABLE, set)

SUBDIRS += \
    Libs/7z \
    Libs/dxtc \
    Libs/lzo2 \
    Libs/png \
    Libs/unlzx \
    Libs/xdelta3 \
    Libs/zlib

equals(ZSTD_ENABLE, true) {
SUBDIRS += \
    Libs/zstd
}

SUBDIRS += \
    Libs/unrar \
    Wrappers \
