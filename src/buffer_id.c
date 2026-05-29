#include "buffer_id.h"
#include "buffer.h"


BufferID buffer_getid(Buffer* buf) {
    return (BufferID) {
        .buffer_uid = buf->uid,
        .buffer_index = buf->index
    };
}
