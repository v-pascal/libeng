#ifndef LIBENG_SCREEN_H_
#define LIBENG_SCREEN_H_

namespace eng {

typedef struct {

    float left;
    float right;
    float top;
    float bottom;

    float ratioW;
    float ratioH;
    // In matrix unit

    float xDpi; // Horizontal DPI
    float yDpi; // Vertical DPI

    short width;
    short height;
    // In pixel

} Screen;

} // namespace

#endif // LIBENG_SCREEN_H_
