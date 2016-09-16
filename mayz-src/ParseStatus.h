#ifndef __ParseStatus_h__
#define __ParseStatus_h__

namespace up {
    enum ParseStatus {
        NOT_PARSED_YET,
        SUCCESS,
        WORD_ANALYSIS_ERROR,
        LEXENT_ANALYSIS_ERROR,
        TOO_LONG,
        SETUP_ERROR,
        EDGE_LIMIT,
        TIME_LIMIT,
        SEARCH_LIMIT,
        UNKNOWN
    };
}

#endif // __ParseStatus_h__
