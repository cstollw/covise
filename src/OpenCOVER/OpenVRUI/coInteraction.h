/* This file is part of COVISE.

   You can use it under the terms of the GNU Lesser General Public License
   version 2.1 or later, see lgpl-2.1.txt.

 * License: LGPL 2+ */

#ifndef CO_INTERACTION
#define CO_INTERACTION

#include <util/coTypes.h>
#include <string>

namespace vrui
{

class OPENVRUIEXPORT coInteraction
{
    friend class coInteractionManager;

public:
    enum InteractionState
    {
        Idle = 0,
        PendingActive,
        Active,
        Paused,
        RemoteActive,
        ActiveNotify,
        Stopped
    };

    enum InteractionPriority
    {
        Low = 0,
        Navigation,
        NavigationHigh,
        Medium,
        High,
        Menu,
        Highest
    };

    enum InteractionType
    {
        ButtonA = 0,
        ButtonB,
        ButtonC,
        ButtonD,
        ButtonE,
        Wheel,
        Joystick,
        AllButtons,
        NumInteractorTypes
    };

    coInteraction(InteractionType type, const std::string &name, InteractionPriority priority = Medium);
    virtual ~coInteraction();

    InteractionState getState() const
    {
        return state;
    }
    InteractionType getType() const
    {
        return type;
    }
    InteractionPriority getPriority() const
    {
        return priority;
    }
    int getRemoteLockID() const
    {
        return remoteLockID;
    }
    void setRemoteLockID(int ID);
    void setRemoteLock(bool);

    void setRemoteActive(bool);

    void requestActivation();

    virtual void update();
    virtual void cancelInteraction();

    virtual void setName(const std::string &name); // set new name (and possibly load appropriate icon)
    virtual void removeIcon(); // remove the indicator for this interaction
    virtual void addIcon(); // add    the indicator for this interaction

    virtual bool hasPriority(); // true: if the interaction's button is clicked, it is activated

    // these are called by the interaction manager, don't use these as client
    virtual void cancelPendingActivation();
    virtual void doActivation();
    virtual void pause();
    virtual const std::string &getName() const
    {
        return name;
    }
    virtual void resetState();

    bool activate(); // only call this in update!!!!

    bool isRegistered()
    {
        return registered;
    }

    void setNotifyOnly(bool flag);
    bool isNotifyOnly()
    {
        return notifyOnly;
    }

protected:
    std::string name;

    InteractionState state;
    InteractionType type;
    InteractionPriority priority;

    bool notifyOnly;
    bool hasPriorityFlag;
    bool registered;
    bool remoteLock;

    int remoteLockID;
};
}
#endif
