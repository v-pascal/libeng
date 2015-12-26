#ifndef LIBENG_PANEL2D_H_
#define LIBENG_PANEL2D_H_
#if defined(__cplusplus)

#include <libeng/Graphic/Object/2D/Object2D.h>
#include <assert.h>

namespace eng {

static const unsigned char PANEL_INDICE_BUFFER[] = { 0, 1, 2, 2, 3, 0 };

//////
class Panel2D : public Object2D {

protected:
    inline void initScreenVertices() {

        mVertices[0] = mScreen->left;
        mVertices[1] = mScreen->top;
        mVertices[2] = mScreen->left;
        mVertices[3] = mScreen->bottom;
        mVertices[4] = mScreen->right;
        mVertices[5] = mScreen->bottom;
        mVertices[6] = mScreen->right;
        mVertices[7] = mScreen->top;
    };
    inline void centerVertices(float halfW, float halfH) {

        mVertices[0] -= halfW;
        mVertices[1] -= halfH;
        mVertices[2] -= halfW;
        mVertices[3] -= halfH;
        mVertices[4] -= halfW;
        mVertices[5] -= halfH;
        mVertices[6] -= halfW;
        mVertices[7] -= halfH;
    };

    void positionVertices(short x0, short y0, short x2, short y2);
    // Position the surface according a left/top & right/bottom screen coordinates (in pixel)
    // -> With [0;0]: Bottom/left coordinates; [g_ScreenWidth;g_ScreenHeight]: Top/right coordinates

    // WARNING: Low performance! Use only when loading

public:
    Panel2D(bool manage = true);
    virtual ~Panel2D();

    // Should not be used with 'Dynamic2D' inheritance (tranformation not applied)
    inline float getLeft() const { return mVertices[0]; }
    inline float getTop() const { return mVertices[1]; }
    inline float getRight() const { return mVertices[4]; }
    inline float getBottom() const { return mVertices[5]; }

    virtual void render(bool unused) const;
    // WARNING: To be redefined!
    // -> Without dynamic management add 'Object2D::transform' method call B4 (without 'Dynamic2D' class multiple inheritance)
    // -> With dynamic management add 'Dynamic2D::transform' method call B4 (with 'Dynamic2D' class multiple inheritance)

};

class Back2D : public Panel2D { // Static surface without any management for background (full screen & texture display)
                                // -> Nothing more to implement
public:
    Back2D(bool manage = true) : Panel2D(manage) { }

    //
    void setTexCoords(const float coords[8]);

    void start(unsigned char texture);
    void start(unsigned char red, unsigned char green, unsigned char blue);

    void render(bool resetUniform) const;

};

} // namespace

#endif // __cplusplus
#endif // LIBENG_PANEL2D_H_
