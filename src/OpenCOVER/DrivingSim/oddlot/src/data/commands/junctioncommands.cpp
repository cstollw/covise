/* This file is part of COVISE.

   You can use it under the terms of the GNU Lesser General Public License
   version 2.1 or later, see lgpl-2.1.txt.

 * License: LGPL 2+ */

/**************************************************************************
** ODD: OpenDRIVE Designer
**   Frank Naegele (c) 2010
**   <mail@f-naegele.de>
**   21.05.2010
**
**************************************************************************/

#include "junctioncommands.hpp"

#include "src/data/roadsystem/rsystemelementjunction.hpp"
#include "src/data/roadsystem/rsystemelementroad.hpp"
#include "src/data/roadsystem/junctionconnection.hpp"
#include "src/data/roadsystem/roadsystem.hpp"
#include "src/data/roadsystem/roadlink.hpp"

//#########################//
// RemoveJunctionCommand //
//#########################//

RemoveJunctionCommand::RemoveJunctionCommand(RSystemElementJunction *junction, DataCommand *parent)
    : DataCommand(parent)
    , junction_(junction)
{
    // Check for validity //
    //
    if (!junction_ || !junction->getRoadSystem())
    {
        setInvalid(); // Invalid
        setText(QObject::tr("RemoveJunctionCommand: Internal error! No junction specified."));
        return;
    }

    roadSystem_ = junction->getRoadSystem();
    connections_ = junction->getConnections();

    foreach (JunctionConnection *conn, connections_)
    {
        RSystemElementRoad *incomingRoad = roadSystem_->getRoad(conn->getIncomingRoad());
        RSystemElementRoad *connectingRoad = roadSystem_->getRoad(conn->getConnectingRoad());
        QString contactPoint = conn->getContactPoint();

        if (connectingRoad)
        {
            if ((contactPoint == "start") && (connectingRoad->getPredecessor()))
            {
                predecessors_.insert(connectingRoad, connectingRoad->getPredecessor());
            }
            else if ((contactPoint == "end") && (connectingRoad->getSuccessor()))
            {
                successors_.insert(connectingRoad, connectingRoad->getSuccessor());
            }
        }

        if (incomingRoad)
        {
            if (incomingRoad->getPredecessor() && (incomingRoad->getPredecessor()->getElementId() == junction->getID()))
            {
                predecessors_.insert(incomingRoad, incomingRoad->getPredecessor());
            }
            if (incomingRoad->getSuccessor() && (incomingRoad->getSuccessor()->getElementId() == junction->getID()))
            {
                successors_.insert(incomingRoad, incomingRoad->getSuccessor());
            }
        }
    }
    setValid();
    setText(QObject::tr("Remove Junction"));
}

/*! \brief .
*
*/
RemoveJunctionCommand::~RemoveJunctionCommand()
{
    // Clean up //
    //
    if (isUndone())
    {
        // nothing to be done (road is still owned by the roadsystem)
    }
    else
    {
        delete junction_;
    }
}

/*! \brief .
*
*/
void
RemoveJunctionCommand::redo()
{
    QMap<RSystemElementRoad *, RoadLink *>::const_iterator predIt = predecessors_.constBegin();
    while (predIt != predecessors_.constEnd())
    {
        predIt.key()->delPredecessor();
        predIt++;
    }

    QMap<RSystemElementRoad *, RoadLink *>::const_iterator succIt = successors_.constBegin();
    while (succIt != successors_.constEnd())
    {
        succIt.key()->delSuccessor();
        succIt++;
    }

    foreach (JunctionConnection *conn, connections_)
    {
        RSystemElementRoad *road = roadSystem_->getRoad(conn->getConnectingRoad());
        if (road)
        {
            road->setJunction("-1");
        }
    }

    roadSystem_->delJunction(junction_);

    setRedone();
}

/*! \brief
*
*/
void
RemoveJunctionCommand::undo()
{
    roadSystem_->addJunction(junction_);

    foreach (JunctionConnection *conn, connections_)
    {
        junction_->addConnection(conn);
        RSystemElementRoad *road = roadSystem_->getRoad(conn->getConnectingRoad());
        if (road)
        {
            road->setJunction(junction_->getID());
        }
    }

    QMap<RSystemElementRoad *, RoadLink *>::const_iterator predIt = predecessors_.constBegin();
    while (predIt != predecessors_.constEnd())
    {
        predIt.key()->setPredecessor(predIt.value());
        predIt++;
    }

    QMap<RSystemElementRoad *, RoadLink *>::const_iterator succIt = successors_.constBegin();
    while (succIt != successors_.constEnd())
    {
        succIt.key()->setSuccessor(succIt.value());
        succIt++;
    }

    setUndone();
}

//#########################//
// NewJunctionCommand //
//#########################//

NewJunctionCommand::NewJunctionCommand(RSystemElementJunction *newJunction, RoadSystem *roadSystem, DataCommand *parent)
    : DataCommand(parent)
    , newJunction_(newJunction)
    , roadSystem_(roadSystem)
{
    // Check for validity //
    //
    if (!newJunction || !roadSystem_)
    {
        setInvalid(); // Invalid
        setText(QObject::tr("NewJunctionCommand: Internal error! No new road specified."));
        return;
    }
    else
    {
        setValid();
        setText(QObject::tr("New Road"));
    }
}

/*! \brief .
*
*/
NewJunctionCommand::~NewJunctionCommand()
{
    // Clean up //
    //
    if (isUndone())
    {
        delete newJunction_;
    }
    else
    {
        // nothing to be done (road is now owned by the roadsystem)
    }
}

/*! \brief .
*
*/
void
NewJunctionCommand::redo()
{
    roadSystem_->addJunction(newJunction_);

    setRedone();
}

/*! \brief
*
*/
void
NewJunctionCommand::undo()
{
    roadSystem_->delJunction(newJunction_);

    setUndone();
}
