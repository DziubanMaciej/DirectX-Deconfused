#pragma once

#include "Utility/MathHelper.h"

#include "DXD/Settings.h"

#include <bitset>
#include <cassert>
#include <functional>

/// Allows dynamically changing various settings from API. Setting can be of any type.
/// Their values can be queried at any time or you can set a callback function which
/// will be executed upon change.
class SettingsImpl : public DXD::Settings {
public:
    // Callback called upon setting change
    struct Data;
    using SettingsChangeHandler = std::function<void(const Data &)>;

    // Generic classes for defining a setting, id is needed to distinguish settings with same type and default value
    template <int id, typename _Type, _Type _defaultValue>
    struct Setting {
        using Type = _Type;
        constexpr static Type defaultValue = _defaultValue;
        Type value = defaultValue;
        SettingsChangeHandler handler = nullptr;
        virtual void setValue(Type value) { this->value = value; }
    };
    template <int id, typename _Type, _Type _defaultValue, _Type _minimumValue, _Type _maximumValue>
    struct NumericalSetting : Setting<id, _Type, _defaultValue> {
        constexpr static Type minimumValue = _minimumValue;
        constexpr static Type maximumValue = _maximumValue;
        void setValue(Type value) override { this->value = MathHelper::clamp(value, minimumValue, maximumValue); }
    };

    // All settings definition
    using VerticalSyncEnabled = Setting<0, bool, false>;
    using SsaoEnabled = Setting<1, bool, false>;
    using SsrEnabled = Setting<2, bool, false>;
    using FogEnabled = Setting<3, bool, false>;
    using DofEnabled = Setting<4, bool, false>;
    using ShadowsQuality = NumericalSetting<5, unsigned int, 8u, 0u, 10u>;
    struct Data : std::tuple<VerticalSyncEnabled, SsaoEnabled, SsrEnabled, FogEnabled, DofEnabled, ShadowsQuality> {};

    // Registering handlers
    template <typename _Setting>
    void registerHandler(SettingsChangeHandler handler) {
        _Setting &setting = std::get<_Setting>(data);
        assert(setting.handler == nullptr); // cannot overwrite handler
        setting.handler = handler;
    }
    template <typename _Setting>
    void unregisterHandler() {
        _Setting &setting = std::get<_Setting>(data);
        assert(setting.handler != nullptr); // unregister only if registered previously
        setting.handler = nullptr;
    }

    // Api accessors
    void setVerticalSyncEnabled(bool value) override { set<VerticalSyncEnabled>(value); }
    void setSsaoEnabled(bool value) override { set<SsaoEnabled>(value); }
    void setSsrEnabled(bool value) override { set<SsrEnabled>(value); }
    void setFogEnabled(bool value) override { set<FogEnabled>(value); }
    void setDofEnabled(bool value) override { set<DofEnabled>(value); }
    void setShadowsQuality(unsigned int value) override { set<ShadowsQuality>(value); }
    bool getVerticalSyncEnabled() const override { return get<VerticalSyncEnabled>(); }
    bool getSsaoEnabled() const override { return get<SsaoEnabled>(); }
    bool getSsrEnabled() const override { return get<SsrEnabled>(); }
    bool getFogEnabled() const override { return get<FogEnabled>(); }
    bool getDofEnabled() const override { return get<DofEnabled>(); }
    unsigned int getShadowsQuality() const override { return get<ShadowsQuality>(); }

    template <typename _Setting>
    void set(typename _Setting::Type value) {
        _Setting &setting = std::get<_Setting>(data);
        setting.setValue(value);
        if (setting.handler != nullptr) {
            setting.handler(data);
        }
    }
    template <typename _Setting>
    typename _Setting::Type get() const {
        const _Setting &setting = std::get<_Setting>(data);
        return setting.value;
    }

private:
    Data data = {};
};

using SettingsData = SettingsImpl::Data;
