#include "authmanager.h"
#include "settingshelper.h"

#include <qtimer.h>

AuthManager& AuthManager::getInstance()
{
    static AuthManager instance(nullptr);
    return instance;
}

void AuthManager::ConnectWallet(CWallet* wallet)
{
    this->wallet = wallet;

    std::string storedPin = SettingsHelper::GetPinCode();
    if (!(storedPin.compare("noPin") == 0 || storedPin.length() == 0))
    {
        // Until this point, newly created PIN is being stored within Settings, in unencrypted form
        this->wallet->SetCurrentPinCode(storedPin);
        SettingsHelper::PutPinCode("noPin");
        // From this point on, PIN is being stored within wallet itself
    }
}

bool AuthManager::Check(const std::string& pin)
{
    std::string storedPin;
    if (wallet)
        wallet->GetCurrentPinCode(storedPin);
    else
        storedPin = SettingsHelper::GetPinCode();
    bool fMatch = storedPin.compare(pin) == 0;
    if (!fMatch)
    {
        if (SettingsHelper::IncrementAuthFailCount() >= 3)
        {
            // TODO: Perform back-off action (i.e. disable wallet temporary)
        }
    }
    else
    {
        SettingsHelper::ResetAuthFailCount();
        SettingsHelper::SetAuthRequested(false);
        QTimer::singleShot(5 * 60 * 1000, this, SLOT(RequestAuthenticate()));
    }
    return fMatch;
}

bool AuthManager::AuthRequested()
{
    return SettingsHelper::IsAuthRequested();
}

void AuthManager::SetPinCode(const std::string& pin)
{
    SettingsHelper::ResetAuthFailCount();
    if (wallet)
    {
        if (!wallet->SetCurrentPinCode(pin))
        {
            // PIN code storing failed
        }
    }
    else
    {
        SettingsHelper::PutPinCode(pin);
    }
    // TODO: Should store last pin used time as well
}

void AuthManager::Reset()
{
    SettingsHelper::ResetAuthFailCount();
    if (wallet)
        wallet->SetCurrentPinCode("noPin");
    else
        SettingsHelper::PutPinCode("noPin");
}

bool AuthManager::ShouldSet()
{
    std::string storedPin;
    if (wallet)
        wallet->GetCurrentPinCode(storedPin);
    else
        storedPin = SettingsHelper::GetPinCode();
    return storedPin.compare("noPin") == 0;
}

void AuthManager::RequestCheck(const std::string &pin)
{
    if (Check(pin))
        Q_EMIT Authenticated();
    else
        Q_EMIT Authenticate();
}

void AuthManager::RequestAuthenticate()
{
    Q_EMIT Authenticate();
    SettingsHelper::SetAuthRequested(true);
}
