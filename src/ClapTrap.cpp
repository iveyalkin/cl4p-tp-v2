//
// Created by Basil Terkin on 10/31/16.
//

#include "ClapTrap.h"
#include "stdio.h"

using namespace TgBot;
using namespace ClapTp;

ClapTrap::ClapTrap(const std::string &token, const std::string &logPrefix, SqlWrapper& sqlWrapper)
        : Bot(token), _logPrefix(logPrefix), _dbInstance(sqlWrapper) {

    getEvents().onCommand("greeting", [this](Message::Ptr message) {
        replyToChat(message->chat->id,
                    "Unce! Unce! Unce! Unce! Ooo, oh check me out. Unce! Unce! Unce! Unce! Oh, come on get down.");
    });

    getEvents().onCommand("querydb", [this](Message::Ptr message) {
        char buffer[1024 * 10];
        _dbInstance.fetchUrls(buffer);
        getApi().sendMessage(message->chat->id, buffer);
    });

    getEvents().onCommand("enough", [this](Message::Ptr message) {
        replyToChat(message->chat->id,
                    "Oh my God, I'm leaking! I think I'm leaking! Ahhhh, I'm leaking! There's oil everywhere!");

        _shoudlShutdown = true;
    });

    getEvents().onCommand("echo", [this](Message::Ptr message) {
        std::string echoMsg = message->text.substr(strlen("/echo"));
        replyToChat(message->chat->id,
                    std::string("You hear an echo: ").append(echoMsg));
    });

    getEvents().onCommand("stash", [this](Message::Ptr message) {
        std::string reply;
        if (message->replyToMessage)
        {
            if (message->replyToMessage->text.empty())
                reply = std::string("I can only stash a text messages!");
            else // TODO: store reply message text in DB for message->from->id key
                reply = std::string("Going to stash: \"")
                                    .append(message->replyToMessage->text)
                                    .append("\" for you in my personal Vault!");
        }
        else
        {
            reply = std::string("There is no quote in your message!");
        }
        replyToChat(message->chat->id, reply);
    });

    getEvents().onCommand("unstash", [this](Message::Ptr message) {
        // TODO: load all message stored for message->from->id user
        // remove them from DB and send to chat
        replyToChat(message->chat->id, "Did you really think I will store your junk!? Poor minion...");
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

void ClapTrap::replyToChat(int64_t chatId, const std::string &message)
{
    getApi().sendMessage(chatId,
                        _logPrefix.append(message));
}

bool ClapTrap::shouldShutdown() {
    return _shoudlShutdown;
}
