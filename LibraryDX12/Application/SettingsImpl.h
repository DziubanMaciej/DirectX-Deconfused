#pragma once

#include "DXD/Settings.h"

#include <bitset>
#include <functional>

class SettingsImpl : public DXD::Settings {
public:
    ~SettingsImpl() override;

    enum Setting {
        VERTICAL_SYNC_ENABLED,
        SSAO_ENABLED,
        SSR_ENABLED,

        COUNT // This should be the last entry
    };
    struct Data {
        bool verticalSyncEnabled = false;
        bool ssaoEnabled = true;
        bool ssrEnabled = false;
    };
    using SettingsChangeHandler = std::function<void(const Data &)>;

    // Registering handlers
    void registerHandler(Setting setting, SettingsChangeHandler handler);
    void unregisterHandler(Setting setting);

    // Setters
    void setVerticalSyncEnabled(bool value) override;
    void setSsaoEnabled(bool value) override;
    void setSsrEnabled(bool value) override;

    // Getters
    bool getVerticalSyncEnabled() const override;
    bool getSsaoEnabled() const override;
    bool getSsrEnabled() const override;

private:
    void callHandler(Setting setting);

    SettingsChangeHandler handlers[Setting::COUNT] = {};
    std::bitset<Setting::COUNT> registeredHandlers = {};
    Data data = {};
};
