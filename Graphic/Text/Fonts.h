#ifndef LIBENG_FONTS_H_
#define LIBENG_FONTS_H_
#if defined(__cplusplus)

#include <libeng/Global.h>
#ifdef LIBENG_ENABLE_TEXT

#include <libeng/Log/Log.h>
#include <libeng/Graphic/Object/Object.h>
#include <stddef.h>
#include <vector>
#include <cstring>
#include <assert.h>

namespace eng {

enum {

    // abcdefghijklmnopqrstuvwxyz&àâçéèêûô
    CHAR_POS_a = 0, CHAR_POS_b, CHAR_POS_c, CHAR_POS_d, CHAR_POS_e, CHAR_POS_f, CHAR_POS_g, CHAR_POS_h, CHAR_POS_i,
    CHAR_POS_j, CHAR_POS_k, CHAR_POS_l, CHAR_POS_m, CHAR_POS_n, CHAR_POS_o, CHAR_POS_p, CHAR_POS_q, CHAR_POS_r,
    CHAR_POS_s, CHAR_POS_t, CHAR_POS_u, CHAR_POS_v, CHAR_POS_w, CHAR_POS_x, CHAR_POS_y, CHAR_POS_z, CHAR_POS_AND,
    CHAR_POS_aGRAVE, CHAR_POS_aCIRC, CHAR_POS_cCEDIL, CHAR_POS_eACUTE, CHAR_POS_eGRAVE, CHAR_POS_eCIRC, CHAR_POS_uCIRC,
    CHAR_POS_oCIRC,

    // ABCDEFGHIJKLMNOPQRSTUVWXYZ€>}]*#ïîö
    CHAR_POS_A = 0, CHAR_POS_B, CHAR_POS_C, CHAR_POS_D, CHAR_POS_E, CHAR_POS_F, CHAR_POS_G, CHAR_POS_H, CHAR_POS_I,
    CHAR_POS_J, CHAR_POS_K, CHAR_POS_L, CHAR_POS_M, CHAR_POS_N, CHAR_POS_O, CHAR_POS_P, CHAR_POS_Q, CHAR_POS_R,
    CHAR_POS_S, CHAR_POS_T, CHAR_POS_U, CHAR_POS_V, CHAR_POS_W, CHAR_POS_X, CHAR_POS_Y, CHAR_POS_Z, CHAR_POS_EURO,
    CHAR_POS_UPPER, CHAR_POS_CLSBRACE, CHAR_POS_CLSHOOK, CHAR_POS_MULTIPLY, CHAR_POS_SHARP, CHAR_POS_iUML,
    CHAR_POS_iCIRC, CHAR_POS_oUML,

    // 0123456789 '"-,.:!?()/@=+$%<{[;ëùü
    CHAR_POS_0 = 0, CHAR_POS_1, CHAR_POS_2, CHAR_POS_3, CHAR_POS_4, CHAR_POS_5, CHAR_POS_6, CHAR_POS_7, CHAR_POS_8,
    CHAR_POS_9, CHAR_POS_SPACE, CHAR_POS_QUOTE, CHAR_POS_DBLQUOTE, CHAR_POS_MINUS, CHAR_POS_COMMA, CHAR_POS_DOT,
    CHAR_POS_DBLDOT, CHAR_POS_EXCLAMATION, CHAR_POS_QUESTION, CHAR_POS_OPNPARENTHESIS, CHAR_POS_CLSPARENTHESIS,
    CHAR_POS_DIVIDE, CHAR_POS_AT, CHAR_POS_EQUAL, CHAR_POS_PLUS, CHAR_POS_DOLLAR, CHAR_POS_PERCENTAGE, CHAR_POS_LOWER,
    CHAR_POS_OPNBRACE, CHAR_POS_OPNHOOK, CHAR_POS_SEMICOLON,  CHAR_POS_eUML, CHAR_POS_uGRAVE,  CHAR_POS_uUML

};

inline void getPosCoords(float* coords, short pos, float width, float height, float offset) {

    coords[0] = pos * width;
    coords[1] = offset;
    coords[2] = coords[0];
    coords[3] = coords[1] + height;
    coords[4] = coords[0] + width;
    coords[5] = coords[3];
    coords[6] = coords[4];
    coords[7] = coords[1];
}

typedef struct {

    float width;
    float height;

    float offset; // Top coordinate according previous font

    short widthPix;     // Font width in pixel
    short heightPix;    // Font height in pixel

} Font; // ...all datas are texture unit (excepted '*Pix' datas)

//////
class Fonts {

private:
    Fonts();
    virtual ~Fonts();

    std::vector<Font*> mFonts;
    static Fonts* mThis;

public:
    static Fonts* getInstance() {
        if (!mThis)
            mThis = new Fonts;
        return mThis;
    }
    static void freeInstance() {
        if (mThis) {
            delete mThis;
            mThis = NULL;
        }
    }

    //////
    void addFont(short index, short width, short height, short texWidth, short texHeight,
            float offset = static_cast<float>(LIBENG_NO_DATA));
    // WARNING: The 'offset' parameter allows you to set manually the 'Font->offset' variable but can be used only one
    //          time on the last font added, or should be always defined

    inline void getFontCoords(float* textcoords, short index, wchar_t code) const {

        LOGV(LIBENG_LOG_TEXT, 0, LOG_FORMAT(" - t:%p; i:%d; c:0x%X"), __PRETTY_FUNCTION__, __LINE__, textcoords, index, code);
        assert(index > LIBENG_NO_DATA);
        assert(index < static_cast<short>(mFonts.size()));

        switch (code) {

        // abcdefghijklmnopqrstuvwxyz&àâçéèêûô
        case L'a': getPosCoords(textcoords, CHAR_POS_a, mFonts[index]->width, mFonts[index]->height, mFonts[index]->offset); break;
        case L'b': getPosCoords(textcoords, CHAR_POS_b, mFonts[index]->width, mFonts[index]->height, mFonts[index]->offset); break;
        case L'c': getPosCoords(textcoords, CHAR_POS_c, mFonts[index]->width, mFonts[index]->height, mFonts[index]->offset); break;
        case L'd': getPosCoords(textcoords, CHAR_POS_d, mFonts[index]->width, mFonts[index]->height, mFonts[index]->offset); break;
        case L'e': getPosCoords(textcoords, CHAR_POS_e, mFonts[index]->width, mFonts[index]->height, mFonts[index]->offset); break;
        case L'f': getPosCoords(textcoords, CHAR_POS_f, mFonts[index]->width, mFonts[index]->height, mFonts[index]->offset); break;
        case L'g': getPosCoords(textcoords, CHAR_POS_g, mFonts[index]->width, mFonts[index]->height, mFonts[index]->offset); break;
        case L'h': getPosCoords(textcoords, CHAR_POS_h, mFonts[index]->width, mFonts[index]->height, mFonts[index]->offset); break;
        case L'i': getPosCoords(textcoords, CHAR_POS_i, mFonts[index]->width, mFonts[index]->height, mFonts[index]->offset); break;
        case L'j': getPosCoords(textcoords, CHAR_POS_j, mFonts[index]->width, mFonts[index]->height, mFonts[index]->offset); break;
        case L'k': getPosCoords(textcoords, CHAR_POS_k, mFonts[index]->width, mFonts[index]->height, mFonts[index]->offset); break;
        case L'l': getPosCoords(textcoords, CHAR_POS_l, mFonts[index]->width, mFonts[index]->height, mFonts[index]->offset); break;
        case L'm': getPosCoords(textcoords, CHAR_POS_m, mFonts[index]->width, mFonts[index]->height, mFonts[index]->offset); break;
        case L'n': getPosCoords(textcoords, CHAR_POS_n, mFonts[index]->width, mFonts[index]->height, mFonts[index]->offset); break;
        case L'o': getPosCoords(textcoords, CHAR_POS_o, mFonts[index]->width, mFonts[index]->height, mFonts[index]->offset); break;
        case L'p': getPosCoords(textcoords, CHAR_POS_p, mFonts[index]->width, mFonts[index]->height, mFonts[index]->offset); break;
        case L'q': getPosCoords(textcoords, CHAR_POS_q, mFonts[index]->width, mFonts[index]->height, mFonts[index]->offset); break;
        case L'r': getPosCoords(textcoords, CHAR_POS_r, mFonts[index]->width, mFonts[index]->height, mFonts[index]->offset); break;
        case L's': getPosCoords(textcoords, CHAR_POS_s, mFonts[index]->width, mFonts[index]->height, mFonts[index]->offset); break;
        case L't': getPosCoords(textcoords, CHAR_POS_t, mFonts[index]->width, mFonts[index]->height, mFonts[index]->offset); break;
        case L'u': getPosCoords(textcoords, CHAR_POS_u, mFonts[index]->width, mFonts[index]->height, mFonts[index]->offset); break;
        case L'v': getPosCoords(textcoords, CHAR_POS_v, mFonts[index]->width, mFonts[index]->height, mFonts[index]->offset); break;
        case L'w': getPosCoords(textcoords, CHAR_POS_w, mFonts[index]->width, mFonts[index]->height, mFonts[index]->offset); break;
        case L'x': getPosCoords(textcoords, CHAR_POS_x, mFonts[index]->width, mFonts[index]->height, mFonts[index]->offset); break;
        case L'y': getPosCoords(textcoords, CHAR_POS_y, mFonts[index]->width, mFonts[index]->height, mFonts[index]->offset); break;
        case L'z': getPosCoords(textcoords, CHAR_POS_z, mFonts[index]->width, mFonts[index]->height, mFonts[index]->offset); break;
        case L'&': getPosCoords(textcoords, CHAR_POS_AND, mFonts[index]->width, mFonts[index]->height, mFonts[index]->offset); break;
        case L'à': getPosCoords(textcoords, CHAR_POS_aGRAVE, mFonts[index]->width, mFonts[index]->height, mFonts[index]->offset); break;
        case L'â': getPosCoords(textcoords, CHAR_POS_aCIRC, mFonts[index]->width, mFonts[index]->height, mFonts[index]->offset); break;
        case L'ç': getPosCoords(textcoords, CHAR_POS_cCEDIL, mFonts[index]->width, mFonts[index]->height, mFonts[index]->offset); break;
        case L'é': getPosCoords(textcoords, CHAR_POS_eACUTE, mFonts[index]->width, mFonts[index]->height, mFonts[index]->offset); break;
        case L'è': getPosCoords(textcoords, CHAR_POS_eGRAVE, mFonts[index]->width, mFonts[index]->height, mFonts[index]->offset); break;
        case L'ê': getPosCoords(textcoords, CHAR_POS_eCIRC, mFonts[index]->width, mFonts[index]->height, mFonts[index]->offset); break;
        case L'û': getPosCoords(textcoords, CHAR_POS_uCIRC, mFonts[index]->width, mFonts[index]->height, mFonts[index]->offset); break;
        case L'ô': getPosCoords(textcoords, CHAR_POS_oCIRC, mFonts[index]->width, mFonts[index]->height, mFonts[index]->offset); break;

        // ABCDEFGHIJKLMNOPQRSTUVWXYZ€>}]*#ïîö
        case L'A': getPosCoords(textcoords, CHAR_POS_A, mFonts[index]->width, mFonts[index]->height, mFonts[index]->offset + mFonts[index]->height); break;
        case L'B': getPosCoords(textcoords, CHAR_POS_B, mFonts[index]->width, mFonts[index]->height, mFonts[index]->offset + mFonts[index]->height); break;
        case L'C': getPosCoords(textcoords, CHAR_POS_C, mFonts[index]->width, mFonts[index]->height, mFonts[index]->offset + mFonts[index]->height); break;
        case L'D': getPosCoords(textcoords, CHAR_POS_D, mFonts[index]->width, mFonts[index]->height, mFonts[index]->offset + mFonts[index]->height); break;
        case L'E': getPosCoords(textcoords, CHAR_POS_E, mFonts[index]->width, mFonts[index]->height, mFonts[index]->offset + mFonts[index]->height); break;
        case L'F': getPosCoords(textcoords, CHAR_POS_F, mFonts[index]->width, mFonts[index]->height, mFonts[index]->offset + mFonts[index]->height); break;
        case L'G': getPosCoords(textcoords, CHAR_POS_G, mFonts[index]->width, mFonts[index]->height, mFonts[index]->offset + mFonts[index]->height); break;
        case L'H': getPosCoords(textcoords, CHAR_POS_H, mFonts[index]->width, mFonts[index]->height, mFonts[index]->offset + mFonts[index]->height); break;
        case L'I': getPosCoords(textcoords, CHAR_POS_I, mFonts[index]->width, mFonts[index]->height, mFonts[index]->offset + mFonts[index]->height); break;
        case L'J': getPosCoords(textcoords, CHAR_POS_J, mFonts[index]->width, mFonts[index]->height, mFonts[index]->offset + mFonts[index]->height); break;
        case L'K': getPosCoords(textcoords, CHAR_POS_K, mFonts[index]->width, mFonts[index]->height, mFonts[index]->offset + mFonts[index]->height); break;
        case L'L': getPosCoords(textcoords, CHAR_POS_L, mFonts[index]->width, mFonts[index]->height, mFonts[index]->offset + mFonts[index]->height); break;
        case L'M': getPosCoords(textcoords, CHAR_POS_M, mFonts[index]->width, mFonts[index]->height, mFonts[index]->offset + mFonts[index]->height); break;
        case L'N': getPosCoords(textcoords, CHAR_POS_N, mFonts[index]->width, mFonts[index]->height, mFonts[index]->offset + mFonts[index]->height); break;
        case L'O': getPosCoords(textcoords, CHAR_POS_O, mFonts[index]->width, mFonts[index]->height, mFonts[index]->offset + mFonts[index]->height); break;
        case L'P': getPosCoords(textcoords, CHAR_POS_P, mFonts[index]->width, mFonts[index]->height, mFonts[index]->offset + mFonts[index]->height); break;
        case L'Q': getPosCoords(textcoords, CHAR_POS_Q, mFonts[index]->width, mFonts[index]->height, mFonts[index]->offset + mFonts[index]->height); break;
        case L'R': getPosCoords(textcoords, CHAR_POS_R, mFonts[index]->width, mFonts[index]->height, mFonts[index]->offset + mFonts[index]->height); break;
        case L'S': getPosCoords(textcoords, CHAR_POS_S, mFonts[index]->width, mFonts[index]->height, mFonts[index]->offset + mFonts[index]->height); break;
        case L'T': getPosCoords(textcoords, CHAR_POS_T, mFonts[index]->width, mFonts[index]->height, mFonts[index]->offset + mFonts[index]->height); break;
        case L'U': getPosCoords(textcoords, CHAR_POS_U, mFonts[index]->width, mFonts[index]->height, mFonts[index]->offset + mFonts[index]->height); break;
        case L'V': getPosCoords(textcoords, CHAR_POS_V, mFonts[index]->width, mFonts[index]->height, mFonts[index]->offset + mFonts[index]->height); break;
        case L'W': getPosCoords(textcoords, CHAR_POS_W, mFonts[index]->width, mFonts[index]->height, mFonts[index]->offset + mFonts[index]->height); break;
        case L'X': getPosCoords(textcoords, CHAR_POS_X, mFonts[index]->width, mFonts[index]->height, mFonts[index]->offset + mFonts[index]->height); break;
        case L'Y': getPosCoords(textcoords, CHAR_POS_Y, mFonts[index]->width, mFonts[index]->height, mFonts[index]->offset + mFonts[index]->height); break;
        case L'Z': getPosCoords(textcoords, CHAR_POS_Z, mFonts[index]->width, mFonts[index]->height, mFonts[index]->offset + mFonts[index]->height); break;
        case L'€': getPosCoords(textcoords, CHAR_POS_EURO, mFonts[index]->width, mFonts[index]->height, mFonts[index]->offset + mFonts[index]->height); break;
        case L'>': getPosCoords(textcoords, CHAR_POS_UPPER, mFonts[index]->width, mFonts[index]->height, mFonts[index]->offset + mFonts[index]->height); break;
        case L'}': getPosCoords(textcoords, CHAR_POS_CLSBRACE, mFonts[index]->width, mFonts[index]->height, mFonts[index]->offset + mFonts[index]->height); break;
        case L']': getPosCoords(textcoords, CHAR_POS_CLSHOOK, mFonts[index]->width, mFonts[index]->height, mFonts[index]->offset + mFonts[index]->height); break;
        case L'*': getPosCoords(textcoords, CHAR_POS_MULTIPLY, mFonts[index]->width, mFonts[index]->height, mFonts[index]->offset + mFonts[index]->height); break;
        case L'#': getPosCoords(textcoords, CHAR_POS_SHARP, mFonts[index]->width, mFonts[index]->height, mFonts[index]->offset + mFonts[index]->height); break;
        case L'ï': getPosCoords(textcoords, CHAR_POS_iUML, mFonts[index]->width, mFonts[index]->height, mFonts[index]->offset + mFonts[index]->height); break;
        case L'î': getPosCoords(textcoords, CHAR_POS_iCIRC, mFonts[index]->width, mFonts[index]->height, mFonts[index]->offset + mFonts[index]->height); break;
        case L'ö': getPosCoords(textcoords, CHAR_POS_oUML, mFonts[index]->width, mFonts[index]->height, mFonts[index]->offset + mFonts[index]->height); break;

        // 0123456789 '"-,.:!?()/@=+$%<{[;ëùü
        case L'0': getPosCoords(textcoords, CHAR_POS_0, mFonts[index]->width, mFonts[index]->height, mFonts[index]->offset + (mFonts[index]->height * 2)); break;
        case L'1': getPosCoords(textcoords, CHAR_POS_1, mFonts[index]->width, mFonts[index]->height, mFonts[index]->offset + (mFonts[index]->height * 2)); break;
        case L'2': getPosCoords(textcoords, CHAR_POS_2, mFonts[index]->width, mFonts[index]->height, mFonts[index]->offset + (mFonts[index]->height * 2)); break;
        case L'3': getPosCoords(textcoords, CHAR_POS_3, mFonts[index]->width, mFonts[index]->height, mFonts[index]->offset + (mFonts[index]->height * 2)); break;
        case L'4': getPosCoords(textcoords, CHAR_POS_4, mFonts[index]->width, mFonts[index]->height, mFonts[index]->offset + (mFonts[index]->height * 2)); break;
        case L'5': getPosCoords(textcoords, CHAR_POS_5, mFonts[index]->width, mFonts[index]->height, mFonts[index]->offset + (mFonts[index]->height * 2)); break;
        case L'6': getPosCoords(textcoords, CHAR_POS_6, mFonts[index]->width, mFonts[index]->height, mFonts[index]->offset + (mFonts[index]->height * 2)); break;
        case L'7': getPosCoords(textcoords, CHAR_POS_7, mFonts[index]->width, mFonts[index]->height, mFonts[index]->offset + (mFonts[index]->height * 2)); break;
        case L'8': getPosCoords(textcoords, CHAR_POS_8, mFonts[index]->width, mFonts[index]->height, mFonts[index]->offset + (mFonts[index]->height * 2)); break;
        case L'9': getPosCoords(textcoords, CHAR_POS_9, mFonts[index]->width, mFonts[index]->height, mFonts[index]->offset + (mFonts[index]->height * 2)); break;
        case L' ': getPosCoords(textcoords, CHAR_POS_SPACE, mFonts[index]->width, mFonts[index]->height, mFonts[index]->offset + (mFonts[index]->height * 2)); break;
        case L'\'': getPosCoords(textcoords, CHAR_POS_QUOTE, mFonts[index]->width, mFonts[index]->height, mFonts[index]->offset + (mFonts[index]->height * 2)); break;
        case L'"': getPosCoords(textcoords, CHAR_POS_DBLQUOTE, mFonts[index]->width, mFonts[index]->height, mFonts[index]->offset + (mFonts[index]->height * 2)); break;
        case L'-': getPosCoords(textcoords, CHAR_POS_MINUS, mFonts[index]->width, mFonts[index]->height, mFonts[index]->offset + (mFonts[index]->height * 2)); break;
        case L',': getPosCoords(textcoords, CHAR_POS_COMMA, mFonts[index]->width, mFonts[index]->height, mFonts[index]->offset + (mFonts[index]->height * 2)); break;
        case L'.': getPosCoords(textcoords, CHAR_POS_DOT, mFonts[index]->width, mFonts[index]->height, mFonts[index]->offset + (mFonts[index]->height * 2)); break;
        case L':': getPosCoords(textcoords, CHAR_POS_DBLDOT, mFonts[index]->width, mFonts[index]->height, mFonts[index]->offset + (mFonts[index]->height * 2)); break;
        case L'!': getPosCoords(textcoords, CHAR_POS_EXCLAMATION, mFonts[index]->width, mFonts[index]->height, mFonts[index]->offset + (mFonts[index]->height * 2)); break;
        case L'?': getPosCoords(textcoords, CHAR_POS_QUESTION, mFonts[index]->width, mFonts[index]->height, mFonts[index]->offset + (mFonts[index]->height * 2)); break;
        case L'(': getPosCoords(textcoords, CHAR_POS_OPNPARENTHESIS, mFonts[index]->width, mFonts[index]->height, mFonts[index]->offset + (mFonts[index]->height * 2)); break;
        case L')': getPosCoords(textcoords, CHAR_POS_CLSPARENTHESIS, mFonts[index]->width, mFonts[index]->height, mFonts[index]->offset + (mFonts[index]->height * 2)); break;
        case L'/': getPosCoords(textcoords, CHAR_POS_DIVIDE, mFonts[index]->width, mFonts[index]->height, mFonts[index]->offset + (mFonts[index]->height * 2)); break;
        case L'@': getPosCoords(textcoords, CHAR_POS_AT, mFonts[index]->width, mFonts[index]->height, mFonts[index]->offset + (mFonts[index]->height * 2)); break;
        case L'=': getPosCoords(textcoords, CHAR_POS_EQUAL, mFonts[index]->width, mFonts[index]->height, mFonts[index]->offset + (mFonts[index]->height * 2)); break;
        case L'+': getPosCoords(textcoords, CHAR_POS_PLUS, mFonts[index]->width, mFonts[index]->height, mFonts[index]->offset + (mFonts[index]->height * 2)); break;
        case L'$': getPosCoords(textcoords, CHAR_POS_DOLLAR, mFonts[index]->width, mFonts[index]->height, mFonts[index]->offset + (mFonts[index]->height * 2)); break;
        case L'%': getPosCoords(textcoords, CHAR_POS_PERCENTAGE, mFonts[index]->width, mFonts[index]->height, mFonts[index]->offset + (mFonts[index]->height * 2)); break;
        case L'<': getPosCoords(textcoords, CHAR_POS_LOWER, mFonts[index]->width, mFonts[index]->height, mFonts[index]->offset + (mFonts[index]->height * 2)); break;
        case L'{': getPosCoords(textcoords, CHAR_POS_OPNBRACE, mFonts[index]->width, mFonts[index]->height, mFonts[index]->offset + (mFonts[index]->height * 2)); break;
        case L'[': getPosCoords(textcoords, CHAR_POS_OPNHOOK, mFonts[index]->width, mFonts[index]->height, mFonts[index]->offset + (mFonts[index]->height * 2)); break;
        case L';': getPosCoords(textcoords, CHAR_POS_SEMICOLON, mFonts[index]->width, mFonts[index]->height, mFonts[index]->offset + (mFonts[index]->height * 2)); break;
        case L'ë': getPosCoords(textcoords, CHAR_POS_eUML, mFonts[index]->width, mFonts[index]->height, mFonts[index]->offset + (mFonts[index]->height * 2)); break;
        case L'ù': getPosCoords(textcoords, CHAR_POS_uGRAVE, mFonts[index]->width, mFonts[index]->height, mFonts[index]->offset + (mFonts[index]->height * 2)); break;
        case L'ü': getPosCoords(textcoords, CHAR_POS_uUML, mFonts[index]->width, mFonts[index]->height, mFonts[index]->offset + (mFonts[index]->height * 2)); break;

            default: {

                LOGW(LOG_FORMAT(" - Code character 0x%X is not defined"), __PRETTY_FUNCTION__, __LINE__, code);
                assert(NULL);
                std::memcpy(textcoords, FULL_TEXCOORD_BUFFER, sizeof(float) * 8);
                break;
            }
        }
    }

    inline const Font* operator[](short index) const {

        assert(index > LIBENG_NO_DATA);
        assert(index < static_cast<short>(mFonts.size()));
        return mFonts[index];
    }

};

} // namespace

#endif // LIBENG_ENABLE_TEXT
#endif // __cplusplus
#endif // LIBENG_FONTS_H_
