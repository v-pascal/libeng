#ifndef NETWORK_H_
#define NETWORK_H_
#if defined(__cplusplus)

#include <libeng/Global.h>
#ifdef LIBENG_ENABLE_SOCIAL

#include <assert.h>

namespace eng {

class Network {

public:
    typedef enum {

        NONE = 0,

        FACEBOOK,
        TWITTER,
        GOOGLE

    } ID;

#ifdef DEBUG
    static void check(ID id) {
        assert((static_cast<unsigned char>(id) == FACEBOOK) ||
            (static_cast<unsigned char>(id) == TWITTER) ||
            (static_cast<unsigned char>(id) == GOOGLE));
    }
#endif
};
struct ShareData { };

struct Request {
    enum {

        RESULT_NONE = 0,

        RESULT_WAITING,
        RESULT_SUCCEEDED, // == 2
        RESULT_CANCELED,
        RESULT_EXPIRED,
        RESULT_FAILED // Unknown error
    };
};

enum {

    GENDER_NONE = 0, // Error case

    GENDER_MALE,
    GENDER_FEMALE
};

} // namespace

#endif // LIBENG_ENABLE_SOCIAL
#endif // __cplusplus
#endif // NETWORK_H_
