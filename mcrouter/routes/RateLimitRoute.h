/*
 *  Copyright (c) 2015, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree. An additional grant
 *  of patent rights can be found in the PATENTS file in the same directory.
 *
 */
#pragma once

#include <memory>
#include <vector>

#include "mcrouter/lib/Reply.h"
#include "mcrouter/routes/McrouterRouteHandle.h"
#include "mcrouter/routes/RateLimiter.h"

namespace facebook { namespace memcache { namespace mcrouter {

/**
 * Requests sent through this route will be rate limited according
 * to settings in the RateLimiter passed to the constructor.
 *
 * See comments in TokenBucket.h for algorithm details.
 */
class RateLimitRoute {
 public:
  using ContextPtr = std::shared_ptr<ProxyRequestContext>;

  static std::string routeName() { return "rate-limit"; }

  template <class Operation, class Request>
  std::vector<McrouterRouteHandlePtr> couldRouteTo(
    const Request& req, Operation, const ContextPtr& ctx) const {

    return {target_};
  }

  RateLimitRoute(McrouterRouteHandlePtr target, RateLimiter rl)
      : target_(std::move(target)),
        rl_(std::move(rl)) {
  }

  template <class Operation, class Request>
  typename ReplyType<Operation, Request>::type
  route(const Request& req, Operation, const ContextPtr& ctx) {
    if (LIKELY(rl_.canPassThrough(Operation()))) {
      return target_->route(req, Operation(), ctx);
    }

    using Reply = typename ReplyType<Operation, Request>::type;
    return Reply(DefaultReply, Operation());
  }

 private:
  McrouterRouteHandlePtr target_;
  RateLimiter rl_;
};

}}}  // facebook::memcache::mcrouter
