#include "LongPoll.h"

namespace ClapTp {

    LongPoll::LongPoll(const ClapTrap &bot) : _api(&bot.getApi()), _eventHandler(&bot.getEventHandler()), _bot(bot) {
    }

    void LongPoll::init() {
        std::vector<Update::Ptr> updates = _api->getUpdates(_lastUpdateId, 100, 60);
        if (updates.size() > 0) {
            _lastUpdateId = updates.back()->updateId + 1;
        }
        if (_bot.isDebug()) {
            _bot.sendGreeting(atoll(_bot.getDebugChatId().c_str()));
        }
    }

    void LongPoll::poll() {
        std::vector<Update::Ptr> updates = _api->getUpdates(_lastUpdateId, 100, 60);
        for (Update::Ptr &item : updates) {
            if (item->updateId >= _lastUpdateId) {
                _lastUpdateId = item->updateId + 1;
            }
            _eventHandler->handleUpdate(item);
        }
    }

}
