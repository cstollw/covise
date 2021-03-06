/* This file is part of COVISE.

   You can use it under the terms of the GNU Lesser General Public License
   version 2.1 or later, see lgpl-2.1.txt.

 * License: LGPL 2+ */

/**************************************************************************
** ODD: OpenDRIVE Designer
**   Frank Naegele (c) 2010
**   <mail@f-naegele.de>
**   06.04.2010
**
**************************************************************************/

#include "junctioneditortool.hpp"

#include "toolmanager.hpp"
#include "toolwidget.hpp"

// Data //
//
#include "src/data/prototypemanager.hpp"

// Qt //
//
#include <QGridLayout>
#include <QPushButton>
#include <QButtonGroup>
#include <QGroupBox>
//#include <QComboBox>
#include <QToolBar>
#include <QToolButton>
#include <QMenu>
#include <QLabel>
#include <QComboBox>
#include <QListWidget>
#include <QListWidgetItem>

#define ColumnCount 2

//################//
//                //
// JunctionEditorTool //
//                //
//################//

/*! \todo Ownership/destructor
*/
JunctionEditorTool::JunctionEditorTool(ToolManager *toolManager)
    : Tool(toolManager)
    , toolId_(ODD::TJE_SELECT)
{
    // Connect //
    //
    connect(this, SIGNAL(toolAction(ToolAction *)), toolManager, SLOT(toolActionSlot(ToolAction *)));

    // Tool Bar //
    //
    initToolBar();
    initToolWidget();
}

void
JunctionEditorTool::initToolWidget()
{
    //	prototypeListWidget->setMaximumWidth(156);

    QGridLayout *toolLayout = new QGridLayout;

    // ButtonGroup //
    //
    // A button group so only one button can be checked at a time
    QButtonGroup *toolGroup = new QButtonGroup;
    connect(toolGroup, SIGNAL(buttonClicked(int)), this, SLOT(handleToolClick(int)));

    // Tools //
    //
    QPushButton *toolButton;
    int row = -1; // button row

    //	toolButton = new QPushButton("Select");
    //	toolButton->setCheckable(true);
    //	toolLayout->addWidget(toolButton, ++row, 0, 1, ColumnCount);
    //	toolGroup->addButton(toolButton, ODD::TTE_SELECT); // button, id

    toolButton = new QPushButton("Select");
    toolButton->setCheckable(true);
    toolLayout->addWidget(toolButton, ++row, 0);
    toolGroup->addButton(toolButton, ODD::TJE_SELECT); // button, id

    QLabel *junctionLabel = new QLabel(tr("Prepare Roads"));
    toolLayout->addWidget(junctionLabel, ++row, 0, 1, ColumnCount);

    toolButton = new QPushButton("Split");
    toolButton->setCheckable(true);
    toolLayout->addWidget(toolButton, ++row, 0);
    toolGroup->addButton(toolButton, ODD::TJE_SPLIT); // button, id

    toolButton = new QPushButton("Adjust");
    toolButton->setCheckable(true);
    toolLayout->addWidget(toolButton, row, 1);
    toolGroup->addButton(toolButton, ODD::TJE_MOVE); // button, id

    toolButton = new QPushButton("Create Connecting Lane");
    toolButton->setCheckable(true);
    toolLayout->addWidget(toolButton, ++row, 0, 1, ColumnCount);
    toolGroup->addButton(toolButton, ODD::TJE_CREATE_LANE); // button, id

    toolButton = new QPushButton("Create Connecting Road");
    toolButton->setCheckable(true);
    toolLayout->addWidget(toolButton, ++row, 0, 1, ColumnCount);
    toolGroup->addButton(toolButton, ODD::TJE_CREATE_ROAD); // button, id

    QFrame *line = new QFrame();
    line->setFrameStyle(QFrame::HLine | QFrame::Sunken);
    line->setLineWidth(1);
    toolLayout->addWidget(line, ++row, 0, 1, ColumnCount);

    toolButton = new QPushButton("Create Junction from selected elements");
    toolButton->setCheckable(true);
    toolLayout->addWidget(toolButton, ++row, 0);
    toolGroup->addButton(toolButton, ODD::TJE_CREATE_JUNCTION); // button, id
    //	toolButton->setChecked(true);

    junctionLabel = new QLabel(tr("Junction"));
    toolLayout->addWidget(junctionLabel, ++row, 0, 1, ColumnCount);

    toolButton = new QPushButton("Select");
    toolButton->setCheckable(true);
    toolLayout->addWidget(toolButton, ++row, 0);
    toolGroup->addButton(toolButton, ODD::TJE_SELECT_JUNCTION); // button, id

    toolButton = new QPushButton("Add Selected Roads");
    toolButton->setCheckable(true);
    toolLayout->addWidget(toolButton, ++row, 0);
    toolGroup->addButton(toolButton, ODD::TJE_ADD_TO_JUNCTION); // button, id

    toolButton = new QPushButton("Remove Selected Roads");
    toolButton->setCheckable(true);
    toolLayout->addWidget(toolButton, row, 1);
    toolGroup->addButton(toolButton, ODD::TJE_REMOVE_FROM_JUNCTION); // button, id

    line = new QFrame();
    line->setFrameStyle(QFrame::HLine | QFrame::Sunken);
    line->setLineWidth(1);
    toolLayout->addWidget(line, ++row, 0, 1, ColumnCount);

    toolButton = new QPushButton("Link Selected Roads");
    toolButton->setCheckable(false);
    toolLayout->addWidget(toolButton, ++row, 0, 1, ColumnCount);
    toolGroup->addButton(toolButton, ODD::TJE_LINK_ROADS); // button, id

    toolButton = new QPushButton("Unlink Selected Roads");
    toolButton->setCheckable(false);
    toolLayout->addWidget(toolButton, ++row, 0, 1, ColumnCount);
    toolGroup->addButton(toolButton, ODD::TJE_UNLINK_ROADS); // button, id

    line = new QFrame();
    line->setFrameStyle(QFrame::HLine | QFrame::Sunken);
    line->setLineWidth(1);
    toolLayout->addWidget(line, ++row, 0, 1, ColumnCount);

    // Circular cutting tool //
    //
    cuttingCircleButton_ = new QPushButton(tr("Circular Cut"));
    cuttingCircleButton_->setCheckable(false);
    toolLayout->addWidget(cuttingCircleButton_, ++row, 0);
    toolGroup->addButton(cuttingCircleButton_, ODD::TJE_CIRCLE); // button, id

    connect(cuttingCircleButton_, SIGNAL(toggled(bool)), this, SLOT(cuttingCircle(bool)));

    // Threshold //
    //
    QLabel *thresholdLabel = new QLabel("Threshold:");
    thresholdEdit_ = new QDoubleSpinBox();
    thresholdEdit_->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    thresholdEdit_->setRange(0.0, 1000.0);
    thresholdEdit_->setValue(10.0);
    thresholdEdit_->setMinimumWidth(80.0);
    thresholdEdit_->setMaximumWidth(80.0);

    toolLayout->addWidget(thresholdLabel, row, 1);
    toolLayout->addWidget(thresholdEdit_, row, 1);

    connect(thresholdEdit_, SIGNAL(editingFinished()), this, SLOT(setThreshold()));

    // Finish Layout //
    //
    toolLayout->setRowStretch(++row, 1); // row x fills the rest of the availlable space
    toolLayout->setColumnStretch(ColumnCount, 1); // column 2 fills the rest of the availlable space

    // Widget/Layout //
    //
    ToolWidget *toolWidget = new ToolWidget();
    toolWidget->setLayout(toolLayout);
    toolManager_->addToolBoxWidget(toolWidget, tr("Junction Editor"));
    connect(toolWidget, SIGNAL(activated()), this, SLOT(activateEditor()));
}

void
JunctionEditorTool::initToolBar()
{
    // no tool bar for me
}

//################//
// SLOTS          //
//################//

/*! \brief Gets called when this widget (tab) has been activated.
*
*/
void
JunctionEditorTool::activateEditor()
{
    // Send //
    //
    JunctionEditorToolAction *action = new JunctionEditorToolAction(ODD::TJE_THRESHOLD, thresholdEdit_->value());
    emit toolAction(action);
    delete action;
}

/*! \brief Gets called when a tool button has been selected.
*
*/
void
JunctionEditorTool::handleToolClick(int id)
{
    toolId_ = (ODD::ToolId)id;

    // Set a tool //
    //
    JunctionEditorToolAction *action = new JunctionEditorToolAction(toolId_, thresholdEdit_->value());
    emit toolAction(action);
    delete action;
}

/*! \brief Gets called when a tool button has been selected.
*
*/
void
JunctionEditorTool::cuttingCircle(bool active)
{
    JunctionEditorToolAction *action = new JunctionEditorToolAction(ODD::TJE_CIRCLE, thresholdEdit_->value(), active);
    emit toolAction(action);
    delete action;
}

/*! \brief Gets called when a tool has been selected.
*/
void
JunctionEditorTool::setThreshold()
{

    // Set a tool //
    //
    JunctionEditorToolAction *action = new JunctionEditorToolAction(ODD::TJE_THRESHOLD, thresholdEdit_->value());
    emit toolAction(action);
    delete action;
}

//################//
//                //
// JunctionEditorToolAction //
//                //
//################//

JunctionEditorToolAction::JunctionEditorToolAction(ODD::ToolId toolId, double threshold, bool toggled)
    : ToolAction(ODD::EJE, toolId)
    , threshold_(threshold)
    , toggled_(toggled)
{
}

void JunctionEditorToolAction::setThreshold(double threshold)
{
    threshold_ = threshold;
}
