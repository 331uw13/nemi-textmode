#ifndef BUFFER_ID_H
#define BUFFER_ID_H

#include "types.h"

typedef struct Buffer_t Buffer;

typedef int bufuid_t;


typedef struct BufferID_t {
    bufuid_t buffer_uid;
    size_t   buffer_index;
}
BufferID;

BufferID buffer_getid(Buffer* buf);

#endif
