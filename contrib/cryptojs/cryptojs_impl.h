//
// Created by czp on 18-3-30.
//

#ifndef METAVERSE_CRYPTOJS_IMPL_H
#define METAVERSE_CRYPTOJS_IMPL_H

#include <string>

namespace cryptojs {

    std::string encrypt(const std::string &message, const std::string &passphrase_);

    std::string decrypt(const std::string &cipher_txt, const std::string &passphrase_);
}
#endif //METAVERSE_CRYPTJS_IMPL_H
