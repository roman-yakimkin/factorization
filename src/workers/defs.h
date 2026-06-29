//
// Created by Roman Yakimkin on 26.06.2026.
//

#pragma once

#include "libs/concurrent_queue.h"
#include "core/models/factorized_number.h"

using NumberQueue = core::IQueue<uint64_t>;
using ResultQueue = core::IQueue<model::FactorizedNumber>;