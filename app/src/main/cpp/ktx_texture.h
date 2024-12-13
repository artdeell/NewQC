//
// Created by maks on 05.12.2024.
//

#ifndef NEWQC_KTX_TEXTURE_H
#define NEWQC_KTX_TEXTURE_H

#include "asset_buffer_read.h"

#include <android/asset_manager.h>
#include <GLES2/gl2.h>
#include <stdbool.h>


bool loadKtx(asset_info_t* uploadInfo, GLuint* texture, GLenum* target);

#endif //NEWQC_KTX_TEXTURE_H
