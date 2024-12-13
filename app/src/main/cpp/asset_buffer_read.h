//
// Created by maks on 10.12.2024.
//

#ifndef NEWQC_ASSET_BUFFER_READ_H
#define NEWQC_ASSET_BUFFER_READ_H

#include <android/asset_manager.h>

typedef struct {
    AAssetManager *assetManager;
    const char* path;
} asset_info_t;

void* readAssetToBuffer(asset_info_t* uploadInfo, off64_t* assetSize);

#endif //NEWQC_ASSET_BUFFER_READ_H
