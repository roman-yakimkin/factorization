//
// Created by Roman Yakimkin on 26.06.2026.
//

#pragma once
#include "defs.h"

namespace workers {
    class ResultPerformer {
        ResultQueue* m_queue = nullptr;

        std::string join(const std::vector<uint64_t>& vec) ;
    public:
        ResultPerformer(ResultQueue* queue);

        void operator()();
    };
} // workers