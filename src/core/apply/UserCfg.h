#pragma once

#include <QString>

namespace core {

enum class UserCfgResult {
    Created,
    Added,
    Updated,
    Unchanged,
    Failed,
};

UserCfgResult ensureUserCfgLanguage(const QString &channelDir, const QString &scLanguageId);

}
