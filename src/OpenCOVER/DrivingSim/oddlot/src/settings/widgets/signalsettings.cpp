/* This file is part of COVISE.

   You can use it under the terms of the GNU Lesser General Public License
   version 2.1 or later, see lgpl-2.1.txt.

 * License: LGPL 2+ */

/**************************************************************************
** ODD: OpenDRIVE Designer
**   Frank Naegele (c) 2010
**   <mail@f-naegele.de>
**   11/2/2010
**
**************************************************************************/

#include "signalsettings.hpp"
#include "ui_signalsettings.h"

#include "src/mainwindow.hpp"

// Data //
//
#include "src/data/roadsystem/sections/signalobject.hpp"
#include "src/data/roadsystem/rsystemelementjunction.hpp"
#include "src/data/roadsystem/roadsystem.hpp"
#include "src/data/roadsystem/roadlink.hpp"
#include "src/data/roadsystem/rsystemelementroad.hpp"
#include "src/data/commands/roadcommands.hpp"
#include "src/data/commands/roadsystemcommands.hpp"
#include "src/data/commands/signalcommands.hpp"
#include "src/data/commands/roadsectioncommands.hpp"
#include "src/data/signalmanager.hpp"
#include "src/data/roadsystem/sections/lanesection.hpp"
#include "src/data/roadsystem/sections/signalobject.hpp"

// GUI //
//
#include "src/gui/projectwidget.hpp"

// Qt //
//
#include <QInputDialog>
#include <QStringList>
#include <QLabel>
#include <QSpinBox>

// Utils //
//
#include "math.h"

//################//
// CONSTRUCTOR    //
//################//

SignalSettings::SignalSettings(ProjectSettings *projectSettings, SettingsElement *parentSettingsElement, Signal *signal)
    : SettingsElement(projectSettings, parentSettingsElement, signal)
    , ui(new Ui::SignalSettings)
    , signal_(signal)
    , init_(false)
{
    signalManager_ = getProjectSettings()->getProjectWidget()->getMainWindow()->getSignalManager();
    ui->setupUi(this);

    addSignals();

    // Initial Values //
    //
    updateProperties();

    connect(ui->nameBox, SIGNAL(editingFinished()), this, SLOT(onEditingFinished()));
    //	connect(ui->sSpinBox, SIGNAL(editingFinished()), this, SLOT(onEditingFinished()));
    connect(ui->tSpinBox, SIGNAL(editingFinished()), this, SLOT(onEditingFinished()));
    connect(ui->zOffsetSpinBox, SIGNAL(editingFinished()), this, SLOT(onEditingFinished()));
    connect(ui->countryBox, SIGNAL(editingFinished()), this, SLOT(onEditingFinished()));
    //	ui->typeComboBox->setCurrentIndex(signal_->getType()-100001);

    connect(ui->typeSpinBox, SIGNAL(editingFinished()), this, SLOT(onEditingFinished()));
    connect(ui->subclassLineEdit, SIGNAL(editingFinished()), this, SLOT(onEditingFinished()));
    connect(ui->subtypeSpinBox, SIGNAL(editingFinished()), this, SLOT(onEditingFinished()));
    connect(ui->valueSpinBox, SIGNAL(editingFinished()), this, SLOT(onEditingFinished()));
    connect(ui->dynamicCheckBox, SIGNAL(stateChanged(int)), this, SLOT(onEditingFinished(int)));
    connect(ui->orientationComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(onEditingFinished(int)));
    connect(ui->poleCheckBox, SIGNAL(stateChanged(int)), this, SLOT(onEditingFinished(int)));
    connect(ui->sizeComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(onEditingFinished(int)));

    connect(ui->fromLaneSpinBox, SIGNAL(editingFinished()), this, SLOT(onEditingFinished()));
    connect(ui->toLaneSpinBox, SIGNAL(editingFinished()), this, SLOT(onEditingFinished()));

    //Pedestrian Crossing has ancillary data
    //
    if (signal_->getType() == 293)
    {

        enableCrossingParams(true);

        connect(ui->crossingSpinBox, SIGNAL(editingFinished()), this, SLOT(onEditingFinished()));
        connect(ui->resetTimeSpinBox, SIGNAL(editingFinished()), this, SLOT(onEditingFinished()));
    }

    init_ = true;
}

SignalSettings::~SignalSettings()
{
    delete ui;
}

//################//
// FUNCTIONS      //
//################//

void
SignalSettings::updateProperties()
{
    if (signal_)
    {
        ui->nameBox->setText(signal_->getName());
        ui->idBox->setText(signal_->getId());
        ui->sSpinBox->setValue(signal_->getSStart());
        ui->tSpinBox->setValue(signal_->getT());
        ui->zOffsetSpinBox->setValue(signal_->getZOffset());
        ui->countryBox->setText(signal_->getCountry());
        //	ui->typeComboBox->setCurrentIndex(signal_->getType()-100001);

        ui->typeSpinBox->setValue(signal_->getType());
        ui->subclassLineEdit->setText(signal_->getTypeSubclass());
        ui->subtypeSpinBox->setValue(signal_->getSubtype());
        ui->valueSpinBox->setValue(signal_->getValue());
        ui->dynamicCheckBox->setChecked(signal_->getDynamic());
        ui->orientationComboBox->setCurrentIndex(signal_->getOrientation());
        ui->poleCheckBox->setChecked(signal_->getPole());

        ui->sizeComboBox->setCurrentIndex(signal_->getSize() - 1);

        ui->fromLaneSpinBox->setValue(signal_->getValidFromLane());
        ui->toLaneSpinBox->setValue(signal_->getValidToLane());
    }
}

double SignalSettings::
    signalT(double s, double t, double roadDistance)
{
    LaneSection *laneSection = signal_->getParentRoad()->getLaneSection(s);
    double dist = 0.0;
    double sSection = s - laneSection->getSStart();

    if (t >= 0)
    {
        dist = laneSection->getLaneSpanWidth(0, laneSection->getLeftmostLaneId(), sSection) + roadDistance;
    }
    else
    {
        dist = -laneSection->getLaneSpanWidth(0, laneSection->getRightmostLaneId(), sSection) - roadDistance;
    }

    return dist;
}

void
SignalSettings::enableCrossingParams(bool value)
{
    ui->crossingSpinBox->setEnabled(value);
    ui->crossingProbLabel->setEnabled(value);
    ui->resetTimeLabel->setEnabled(value);
    ui->resetTimeSpinBox->setEnabled(value);
    ui->poleCheckBox->setChecked(!value);
}

void
SignalSettings::updateProperties(QString country, SignalContainer *signalProperties)
{
    double t = signalT(ui->sSpinBox->value(), ui->tSpinBox->value(), signalProperties->getSignalDistance());

    ui->tSpinBox->setValue(t);
    ui->countryBox->setText(country);

    ui->typeSpinBox->setValue(signalProperties->getSignalType());
    ui->subclassLineEdit->setText(signalProperties->getSignalTypeSubclass());
    ui->subtypeSpinBox->setValue(signalProperties->getSignalSubType());
    ui->valueSpinBox->setValue(signalProperties->getSignalValue());

    //Pedestrian Crossing has ancillary data
    //
    if ((signalProperties->getSignalType() == 293) && !ui->crossingProbLabel->isEnabled())
    {

        enableCrossingParams(true);

        connect(ui->crossingSpinBox, SIGNAL(editingFinished()), this, SLOT(onEditingFinished()));
        connect(ui->resetTimeSpinBox, SIGNAL(editingFinished()), this, SLOT(onEditingFinished()));
    }
    else if (ui->crossingProbLabel->isEnabled() && (signalProperties->getSignalType() != 293))
    {
        enableCrossingParams(false);
    }
}

void
SignalSettings::addSignals()
{
    foreach (const SignalContainer *container, signalManager_->getSignals("China"))
    {
        ui->signalComboBox->addItem(container->getSignalIcon(), container->getSignalName());
    }
    foreach (const SignalContainer *container, signalManager_->getSignals("OpenDRIVE"))
    {
        ui->signalComboBox->addItem(container->getSignalIcon(), container->getSignalName());
    }
    foreach (const SignalContainer *container, signalManager_->getSignals("France"))
    {
        ui->signalComboBox->addItem(container->getSignalIcon(), container->getSignalName());
    }
    foreach (const SignalContainer *container, signalManager_->getSignals("Germany"))
    {
        ui->signalComboBox->addItem(container->getSignalIcon(), container->getSignalName());
    }
    foreach (const SignalContainer *container, signalManager_->getSignals("USA"))
    {
        ui->signalComboBox->addItem(container->getSignalIcon(), container->getSignalName());
    }
}

//################//
// SLOTS          //
//################//

void
SignalSettings::onEditingFinished(int i)
{
    if ((ui->dynamicCheckBox->isChecked() != signal_->getDynamic()) || (ui->poleCheckBox->isChecked() != signal_->getPole()) || ((Signal::OrientationType)ui->orientationComboBox->currentIndex() != signal_->getOrientation()) || (ui->sizeComboBox->currentIndex() + 1 != signal_->getSize()))
    {
        onEditingFinished();
    }
}

void
SignalSettings::onEditingFinished()
{
    QString filename = ui->nameBox->text();
    QString newId = signal_->getNewId(filename);
    signal_->setId(newId);

    double t = ui->tSpinBox->value();
    int fromLane = ui->fromLaneSpinBox->value();
    int toLane = ui->toLaneSpinBox->value();

    if (signal_->getType() != 293)
    {
        if (((t < 0) && ((fromLane > 0) || (fromLane < signal_->getParentRoad()->getLaneSection(signal_->getSStart())->getRightmostLaneId()))) || ((t > 0) && ((fromLane < 0) || (fromLane > signal_->getParentRoad()->getLaneSection(signal_->getSStart())->getLeftmostLaneId()))))
        {
            fromLane = signal_->getParentRoad()->getValidLane(signal_->getSStart(), t);
        }

        if (((t < 0) && ((toLane > 0) || (toLane < signal_->getParentRoad()->getLaneSection(signal_->getSStart())->getRightmostLaneId()))) || ((t > 0) && ((toLane < 0) || (toLane > signal_->getParentRoad()->getLaneSection(signal_->getSStart())->getLeftmostLaneId()))))
        {
            toLane = signal_->getParentRoad()->getValidLane(signal_->getSStart(), t);
        }
    }
    else
    {
        if (fromLane < signal_->getParentRoad()->getLaneSection(signal_->getSStart())->getRightmostLaneId())
        {
            fromLane = signal_->getParentRoad()->getLaneSection(signal_->getSStart())->getRightmostLaneId();
        }
        else if (fromLane > signal_->getParentRoad()->getLaneSection(signal_->getSStart())->getLeftmostLaneId())
        {
            fromLane = signal_->getParentRoad()->getLaneSection(signal_->getSStart())->getLeftmostLaneId();
        }

        if (toLane < signal_->getParentRoad()->getLaneSection(signal_->getSStart())->getRightmostLaneId())
        {
            toLane = signal_->getParentRoad()->getLaneSection(signal_->getSStart())->getRightmostLaneId();
        }
        else if (toLane > signal_->getParentRoad()->getLaneSection(signal_->getSStart())->getLeftmostLaneId())
        {
            toLane = signal_->getParentRoad()->getLaneSection(signal_->getSStart())->getLeftmostLaneId();
        }
    }

    if (((t < 0) && (toLane > fromLane)) || ((t > 0) && (toLane < fromLane)))
    {
        toLane = fromLane;
    }

    SetSignalPropertiesCommand *command = new SetSignalPropertiesCommand(signal_, signal_->getId(), filename, ui->tSpinBox->value(), ui->dynamicCheckBox->isChecked(), (Signal::OrientationType)ui->orientationComboBox->currentIndex(), ui->zOffsetSpinBox->value(), ui->countryBox->text(), ui->typeSpinBox->value(), ui->subclassLineEdit->text(), ui->subtypeSpinBox->value(), ui->valueSpinBox->value(), ui->poleCheckBox->isChecked(), ui->sizeComboBox->currentIndex() + 1, fromLane, toLane, ui->crossingSpinBox->value(), ui->resetTimeSpinBox->value(), NULL);
    getProjectSettings()->executeCommand(command);
}

void
SignalSettings::on_signalComboBox_activated(int id)
{
    QString country = "";
    int count = signalManager_->getSignals("OpenDRIVE").count();

    if (count > id)
    {
        country = "OpenDRIVE";
    }
    else if ((count += signalManager_->getSignals("China").count()) > id)
    {
        country = "China";
        id -= count - signalManager_->getSignals("China").count();
    }
    else if ((count += signalManager_->getSignals("France").count()) > id)
    {
        country = "France";
        id -= count - signalManager_->getSignals("France").count();
    }
    else if ((count += signalManager_->getSignals("Germany").count()) > id)
    {
        country = "Germany";
        id -= count - signalManager_->getSignals("Germany").count();
    }
    else if ((count += signalManager_->getSignals("USA").count()) > id)
    {
        country = "USA";
        id -= count - signalManager_->getSignals("USA").count();
    }
    else
    {
        qDebug() << "ID out of range";
        return;
    }

    SignalContainer *signalContainer = signalManager_->getSignals(country).at(id);
    updateProperties(country, signalContainer);

    QList<DataElement *> selectedElements = getProjectData()->getSelectedElements();
    int numberOfSelectedElements = selectedElements.size();

    if (numberOfSelectedElements > 1)
    {
        getProjectData()->getUndoStack()->beginMacro(QObject::tr("Change Signal Type"));
    }

    // Change types of selected items //
    //
    foreach (DataElement *element, selectedElements)
    {
        Signal *signal = dynamic_cast<Signal *>(element);
        if (signal)
        {
            double t = signalT(signal->getSStart(), signal->getT(), signalContainer->getSignalDistance());
            SetSignalPropertiesCommand *command;
            /*	if (country == "China")
			{
				QString newName = signalContainer->getSignalName().at(0);
				QString newId = signal->getNewId(newName);  // the first letter of the name defines the group (round, rectangular, ..)
				command = new SetSignalPropertiesCommand(signal, newId, newName, t, signal->getDynamic(), signal->getOrientation(), signal->getZOffset(), country, signalContainer->getSignalType(), signalContainer->getSignalTypeSubclass(), signalContainer->getSignalSubType(), signalContainer->getSignalValue(), signal->getPole(), signal->getSize(), signal->getValidFromLane(), signal->getValidToLane(), signal->getCrossingProbability(), signal->getResetTime());
			}
			else
			{ */

            command = new SetSignalPropertiesCommand(signal, signal->getId(), signal->getName(), t, signal->getDynamic(), signal->getOrientation(), signal->getZOffset(), country, signalContainer->getSignalType(), signalContainer->getSignalTypeSubclass(), signalContainer->getSignalSubType(), signalContainer->getSignalValue(), signal->getPole(), signal->getSize(), signal->getValidFromLane(), signal->getValidToLane(), signal->getCrossingProbability(), signal->getResetTime());
            //		}

            getProjectSettings()->executeCommand(command);
        }
    }

    // Macro Command //
    //
    if (numberOfSelectedElements > 1)
    {
        getProjectData()->getUndoStack()->endMacro();
    }
}

void
SignalSettings::on_sSpinBox_editingFinished()
{
    QList<DataElement *> selectedElements = getProjectData()->getSelectedElements();
    int numberOfSelectedElements = selectedElements.size();

    if (numberOfSelectedElements > 1)
    {
        getProjectData()->getUndoStack()->beginMacro(QObject::tr("Change Signal Type"));
    }

    foreach (DataElement *element, selectedElements)
    {

        Signal *signal = dynamic_cast<Signal *>(element);
        if (signal)
        {
            MoveRoadSectionCommand *command = new MoveRoadSectionCommand(signal_, ui->sSpinBox->value(), RSystemElementRoad::DRS_SignalSection);
            getProjectSettings()->executeCommand(command);
        }
    }

    // Macro Command //
    //
    if (numberOfSelectedElements > 1)
    {
        getProjectData()->getUndoStack()->endMacro();
    }

    onEditingFinished();
}

//##################//
// Observer Pattern //
//##################//

void
SignalSettings::updateObserver()
{

    // Parent //
    //
    SettingsElement::updateObserver();
    if (isInGarbage())
    {
        return; // no need to go on
    }

    // Signal //
    //
    int changes = signal_->getSignalChanges();

    if ((changes & Signal::CEL_ParameterChange))
    {
        updateProperties();
    }
}
