//
// Created by Basil Terkin on 10/31/16.
//

#include "ClapTrap.h"

using namespace TgBot;
using namespace ClapTp;

ClapTrap::ClapTrap(const std::string &token, boost::optional<std::string>& logPrefix, SqlWrapper& sqlWrapper)
        : Bot(token), _logPrefix(logPrefix), _dbInstance(sqlWrapper) {

    getEvents().onCommand("greeting", [this](Message::Ptr message) {
        getApi().sendMessage(
                message->chat->id,
                (_logPrefix ? *_logPrefix : std::string())
                        .append(
                                "Unce! Unce! Unce! Unce! Ooo, oh check me out. Unce! Unce! Unce! Unce! Oh, come on get down."
                        )
        );
    });

    getEvents().onCommand("querydb", [this](Message::Ptr message) {
        char buffer[256];
        _dbInstance.readSampleTable(buffer);
        getApi().sendMessage(
                message->chat->id,
                buffer
        );
    });

    getEvents().onCommand("enough", [this](Message::Ptr message) {
        getApi().sendMessage(
                message->chat->id,
                (_logPrefix ? *_logPrefix : std::string())
                        .append(
                                "Oh my God, I'm leaking! I think I'm leaking! Ahhhh, I'm leaking! There's oil everywhere!"
                        )
        );
        _shoudlShutdown = true;
    });

    getEvents().onCommand("echo", [this](Message::Ptr message) {
        std::string echoMsg = message->text.substr(strlen("/echo"));
        getApi().sendMessage(
                message->chat->id,
                (_logPrefix ? *_logPrefix : std::string())
                        .append("You hear an echo: ").append(echoMsg)
        );
    });

    getEvents().onAnyMessage([this](Message::Ptr message) {
        std::vector<MessageEntity::Ptr>::const_iterator
                itr = message->entities.begin(),
                end = message->entities.end();
        _dbInstance.saveUser(message->from);
        while (itr != end) {
            if ((*itr)->url.compare("url")) {
                MessageEntity::Ptr entity = *itr;
                std::string url = message->text.substr(entity->offset, entity->length);
                std::string description;
                if (entity->offset > 0) {
                    description.append(message->text.substr(0, entity->offset));
                }
                description.append(message->text.substr(entity->offset + entity->length));
                std::printf("Found a link [%s]. Saving...\n", url.c_str());
                _dbInstance.saveUrl(message->from->id, url, description);
            }
            itr++;
        }
    });
}

bool ClapTrap::shouldShutdown() {
    return _shoudlShutdown;
}
