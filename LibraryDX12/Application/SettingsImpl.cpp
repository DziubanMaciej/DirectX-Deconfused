#include "SettingsImpl.h"

#include <cassert>

// ----------------------------------------------------------- Core logic

SettingsImpl::~SettingsImpl() {
    assert(registeredHandlers.count() == 0); // all handlers should be unregistered
}

void SettingsImpl::callHandler(Setting setting) {
    if (registeredHandlers.test(setting)) {
        handlers[setting](data);
    }
}

void SettingsImpl::registerHandler(Setting setting, SettingsChangeHandler handler) {
    assert(!registeredHandlers.test(setting)); // cannot overwrite handler
    handlers[setting] = handler;
    registeredHandlers.set(setting);
}

void SettingsImpl::unregisterHandler(Setting setting) {
    assert(registeredHandlers.test(setting)); // unregister only if registered previously
    handlers[setting] = {};
    registeredHandlers.reset(setting);
}

// ----------------------------------------------------------- Accessors

void SettingsImpl::setVerticalSyncEnabled(bool value) {
    data.verticalSyncEnabled = value;
    callHandler(VERTICAL_SYNC_ENABLED);
}

void SettingsImpl::setSsaoEnabled(bool value) {
    data.ssaoEnabled = value;
    callHandler(SSAO_ENABLED);
}

void SettingsImpl::setSsrEnabled(bool value) {
    data.ssrEnabled = value;
    callHandler(SSR_ENABLED);
}

bool SettingsImpl::getVerticalSyncEnabled() const {
    return data.verticalSyncEnabled;
}

bool SettingsImpl::getSsaoEnabled() const {
    return data.ssaoEnabled;
}

bool SettingsImpl::getSsrEnabled() const {
    return data.ssrEnabled;
}
