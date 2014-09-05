// Copyright (c) 2014, The Monero Project
// 
// All rights reserved.
// 
// Redistribution and use in source and binary forms, with or without modification, are
// permitted provided that the following conditions are met:
// 
// 1. Redistributions of source code must retain the above copyright notice, this list of
//    conditions and the following disclaimer.
// 
// 2. Redistributions in binary form must reproduce the above copyright notice, this list
//    of conditions and the following disclaimer in the documentation and/or other
//    materials provided with the distribution.
// 
// 3. Neither the name of the copyright holder nor the names of its contributors may be
//    used to endorse or promote products derived from this software without specific
//    prior written permission.
// 
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY
// EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
// MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL
// THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
// STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF
// THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
// 
// Parts of this file are originally copyright (c) 2012-2013 The Cryptonote developers

#pragma once

#include "checkpoints.h"
#include "misc_log_ex.h"

#define ADD_CHECKPOINT(h, hash)  CHECK_AND_ASSERT(checkpoints.add_checkpoint(h,  hash), false);

namespace cryptonote {
  inline bool create_checkpoints(cryptonote::checkpoints& checkpoints)
  {      
    ADD_CHECKPOINT(22231, "7cb10e29d67e1c069e6e11b17d30b809724255fee2f6868dc14cfc6ed44dfb25");
    ADD_CHECKPOINT(29556, "53c484a8ed91e4da621bb2fa88106dbde426fe90d7ef07b9c1e5127fb6f3a7f6");
    ADD_CHECKPOINT(50000, "0fe8758ab06a8b9cb35b7328fd4f757af530a5d37759f9d3e421023231f7b31c");
    ADD_CHECKPOINT(80000, "a62dcd7b536f22e003ebae8726e9e7276f63d594e264b6f0cd7aab27b66e75e3");
      
    // [linux] Smart way to generete this:  
    // Copy part of a table from http://monerochain.info/ to file.txt and run in bash: 
    // cat file.txt | awk '{print "    ADD_CHECKPOINT("  $1 ", \"" $7 "\");"}' 

    ADD_CHECKPOINT(202623, "5f3d0f13584621b51faf6c8460727ef3dce645720572424d9027f8f773b0e21e");
    ADD_CHECKPOINT(202622, "94e6edc9e4b54f9a38e2d9aa871f0bdb68d49bc838a0893d6f89063fcab326f5");
    ADD_CHECKPOINT(202621, "d0ea798fdadd22171359b5ec198332114d1a7cbb08bd5a84830a63269b16e89d");
    ADD_CHECKPOINT(202620, "c8c20ae68d0ba8101a0eed38daacfe082e39a5b603470fb0fff86ccbe4adc42f");
    ADD_CHECKPOINT(202619, "23f4934097e06c33152d68804184f6e1815bfd6a3da4c91dfe99991137235953");
    ADD_CHECKPOINT(202618, "6a04a849f728bca3ecb477f3e02a668c3ea16985374eac1736156c858c84e475");
    ADD_CHECKPOINT(202617, "c6c0944b5fc403f516ed0b5ad2f301373d0931d4b085ca9265d7b836b2e7ec60");
    ADD_CHECKPOINT(202616, "402acbadddb7782426e805800a4a2935b3181b4111894ed4f5042bc1402a9ebc");
    ADD_CHECKPOINT(202615, "b731af9ae1c25ab2a1975b70e8abe70709bb34350e27d1cb70a62eb1be1ea051");
    ADD_CHECKPOINT(202614, "ed4eea6109a1b662cf4a3bb372ed4bdee588160b0ac371c2ad78c5e603b8f2ac");
    ADD_CHECKPOINT(202613, "e2aa337e78df1f98f462b3b1e560c6b914dec47b610698b7b7d1e3e86b6197c2");

    
    ADD_CHECKPOINT(202612, "bbd604d2ba11ba27935e006ed39c9bfdd99b76bf4a50654bc1e1e61217962698"); // achtung! We know this hash is incorrect!
    ADD_CHECKPOINT(202611, "5da0a3d004c352a90cc86b00fab676695d76a4d1de16036c41ba4dd188c4d76f");
    ADD_CHECKPOINT(202610, "12a01d9cd3e95ba914411fb6392877508772f6815d04e52196debdefa726a5cb");
    ADD_CHECKPOINT(202609, "5ecb7e663bbe947c734c8059e7d7d52dc7d6644bb82d81a6ad4057d127ee8eda");
    ADD_CHECKPOINT(202608, "d1e0a6577e0dc26495be142d48700ef5646925750749fad2ac9aed8d03897c51");
    ADD_CHECKPOINT(202607, "9a1838a329adc829b030b6b54cbcc0ff3f1e443687e1ddc3966d2afa7764b1e9");
    ADD_CHECKPOINT(202606, "4a24b51d09fce6f084244f6cfb54fb3a9ea3ef72b5af0b4353c89ae9a716474e");
    ADD_CHECKPOINT(202605, "76464ff30a26b3af262933f833b0223dfdf00a086e00c61446feaf36da19444f");
    ADD_CHECKPOINT(202604, "c9957da0438c06dec4cb2952c457d8aca973b14cba0cb9f39c4c295160c2b79a");
    ADD_CHECKPOINT(202603, "1b93378212ac1aef92670bee09d8797661f1f6687712b09adcaee40d16bde3df");
    ADD_CHECKPOINT(202602, "1df4450d4d5b4604e43f21761690d2a35073d29706aeb574449e6505bb90f5a8");
    ADD_CHECKPOINT(202601, "99b0114929f3f3820f22f59e7d54ce7c6c47900e6c0a19ef8a8ed34534524b37");
 


    return true;
  }
}
