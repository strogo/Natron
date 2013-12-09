//  Natron
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */
/*
 *Created by Alexandre GAUTHIER-FOICHAT on 6/1/2012.
 *contact: immarespond at gmail dot com
 *
 */

// TODO: split into KnobGui.cpp, KnobGuiFile.cpp and KnobGuiTypes.cpp

#include "Gui/KnobUndoCommand.h"
#include "Gui/KnobGui.h"

#include <cassert>
#include <climits>
#include <cfloat>
#include <stdexcept>

#include <QtCore/QString>
#include <QHBoxLayout>
#include <QPushButton>
#include <QFileDialog>
#include <QTextEdit>
#include <QtGui/QStyle>

#include <QKeyEvent>
#include <QColorDialog>
#include <QGroupBox>
#include <QtGui/QVector4D>
#include <QStyleFactory>
#include <QMenu>

#include "Global/AppManager.h"
#include "Global/LibraryBinary.h"
#include "Global/GlobalDefines.h"

#include "Engine/Node.h"
#include "Engine/VideoEngine.h"
#include "Engine/ViewerInstance.h"
#include "Engine/Settings.h"
#include "Engine/Knob.h"
#include "Engine/KnobFile.h"
#include "Engine/KnobTypes.h"
#include "Engine/TimeLine.h"

#include "Gui/Button.h"
#include "Gui/DockablePanel.h"
#include "Gui/ViewerTab.h"
#include "Gui/TimeLineGui.h"
#include "Gui/Gui.h"
#include "Gui/SequenceFileDialog.h"
#include "Gui/TabWidget.h"
#include "Gui/NodeGui.h"
#include "Gui/SpinBox.h"
#include "Gui/ComboBox.h"
#include "Gui/LineEdit.h"
#include "Gui/CurveEditor.h"
#include "Gui/ScaleSlider.h"
#include "Gui/KnobGuiTypes.h"
#include "Gui/CurveWidget.h"

#include "Readers/Reader.h"

using namespace Natron;

/////////////// KnobGui

KnobGui::KnobGui(Knob* knob,DockablePanel* container)
: _knob(knob)
, _triggerNewLine(true)
, _spacingBetweenItems(0)
, _widgetCreated(false)
, _container(container)
, _animationMenu(NULL)
, _setKeyAction(NULL)
, _animationButton(NULL)
{
    QObject::connect(knob,SIGNAL(valueChanged(int)),this,SLOT(onInternalValueChanged(int)));
    QObject::connect(this,SIGNAL(valueChanged(int,const Variant&)),knob,SLOT(onValueChanged(int,const Variant&)));
    QObject::connect(knob,SIGNAL(secretChanged()),this,SLOT(setSecret()));
    QObject::connect(knob,SIGNAL(enabledChanged()),this,SLOT(setEnabledSlot()));
    QObject::connect(knob,SIGNAL(deleted()),this,SLOT(deleteKnob()));
}

KnobGui::~KnobGui(){
    
    emit deleted(this);
    delete _animationButton;
    delete _animationMenu;
}

void KnobGui::pushUndoCommand(QUndoCommand* cmd){
    if(_knob->canBeUndone() && !_knob->isInsignificant()){
        _container->pushUndoCommand(cmd);
    }else{
        cmd->redo();
    }
}



void KnobGui::moveToLayout(QVBoxLayout* layout){
    QWidget* container = new QWidget(layout->parentWidget());
    QHBoxLayout* containerLayout = new QHBoxLayout(container);
    container->setLayout(containerLayout);
    containerLayout->setContentsMargins(0, 0, 0, 0);
    addToLayout(containerLayout);
    layout->addWidget(container);
}

void KnobGui::createGUI(QGridLayout* layout,int row){
    createWidget(layout, row);
    if(_knob->isAnimationEnabled() && !_knob->isSecret()){
        createAnimationButton(layout,row);
    }
    _widgetCreated = true;
    const std::map<int,Variant>& values = _knob->getValueForEachDimension();
    for(std::map<int,Variant>::const_iterator it = values.begin();it!=values.end();++it){
        updateGUI(it->first,it->second);
    }
    setEnabled();
    setSecret();
}

void KnobGui::createAnimationButton(QGridLayout* layout,int row){
    createAnimationMenu(layout->parentWidget());
    _animationButton = new Button("A",layout->parentWidget());
    _animationButton->setToolTip("Animation menu");
    QObject::connect(_animationButton,SIGNAL(clicked()),this,SLOT(showAnimationMenu()));
    layout->addWidget(_animationButton, row, 3,Qt::AlignLeft);
}

void KnobGui::createAnimationMenu(QWidget* parent){
    _animationMenu = new QMenu(parent);
    _setKeyAction = new QAction(tr("Set Key"),_animationMenu);
    QObject::connect(_setKeyAction,SIGNAL(triggered()),this,SLOT(onSetKeyActionTriggered()));
    _animationMenu->addAction(_setKeyAction);
    
    QAction* showInCurveEditorAction = new QAction(tr("Show in curve editor"),_animationMenu);
    QObject::connect(showInCurveEditorAction,SIGNAL(triggered()),this,SLOT(onShowInCurveEditorActionTriggered()));
    _animationMenu->addAction(showInCurveEditorAction);
    
    QAction* removeAnyAnimationAction = new QAction(tr("Remove animation"),_animationMenu);
    QObject::connect(removeAnyAnimationAction,SIGNAL(triggered()),this,SLOT(onRemoveAnyAnimationActionTriggered()));
    _animationMenu->addAction(removeAnyAnimationAction);
    
    
    
    QMenu* interpolationMenu = new QMenu(_animationMenu);
    interpolationMenu->setTitle("Interpolation");
    _animationMenu->addAction(interpolationMenu->menuAction());
    
    QAction* constantInterpAction = new QAction(tr("Constant"),interpolationMenu);
    QObject::connect(constantInterpAction,SIGNAL(triggered()),this,SLOT(onConstantInterpActionTriggered()));
    interpolationMenu->addAction(constantInterpAction);
    
    QAction* linearInterpAction = new QAction(tr("Linear"),interpolationMenu);
    QObject::connect(linearInterpAction,SIGNAL(triggered()),this,SLOT(onLinearInterpActionTriggered()));
    interpolationMenu->addAction(linearInterpAction);
    
    QAction* smoothInterpAction = new QAction(tr("Smooth"),interpolationMenu);
    QObject::connect(smoothInterpAction,SIGNAL(triggered()),this,SLOT(onSmoothInterpActionTriggered()));
    interpolationMenu->addAction(smoothInterpAction);
    
    QAction* catmullRomInterpAction = new QAction(tr("Catmull-Rom"),interpolationMenu);
    QObject::connect(catmullRomInterpAction,SIGNAL(triggered()),this,SLOT(onCatmullromInterpActionTriggered()));
    interpolationMenu->addAction(catmullRomInterpAction);
    
    QAction* cubicInterpAction = new QAction(tr("Cubic"),interpolationMenu);
    QObject::connect(cubicInterpAction,SIGNAL(triggered()),this,SLOT(onCubicInterpActionTriggered()));
    interpolationMenu->addAction(cubicInterpAction);
    
    QAction* horizInterpAction = new QAction(tr("Horizontal"),interpolationMenu);
    QObject::connect(horizInterpAction,SIGNAL(triggered()),this,SLOT(onHorizontalInterpActionTriggered()));
    interpolationMenu->addAction(horizInterpAction);
    
    
    QMenu* copyMenu = new QMenu(_animationMenu);
    copyMenu->setTitle("Copy");
    _animationMenu->addAction(copyMenu->menuAction());
    
    QAction* copyValuesAction = new QAction(tr("Copy values"),copyMenu);
    QObject::connect(copyValuesAction,SIGNAL(triggered()),this,SLOT(onCopyValuesActionTriggered()));
    copyMenu->addAction(copyValuesAction);
    
    QAction* copyAnimationAction = new QAction(tr("Copy animation"),copyMenu);
    QObject::connect(copyAnimationAction,SIGNAL(triggered()),this,SLOT(onCopyAnimationActionTriggered()));
    copyMenu->addAction(copyAnimationAction);
    
    QAction* pasteAction = new QAction(tr("Paste"),copyMenu);
    QObject::connect(pasteAction,SIGNAL(triggered()),this,SLOT(onPasteActionTriggered()));
    copyMenu->addAction(pasteAction);
 
    
    QAction* linkToAction = new QAction(tr("Link to"),_animationMenu);
    QObject::connect(linkToAction,SIGNAL(triggered()),this,SLOT(onLinkToActionTriggered()));
    _animationMenu->addAction(linkToAction);

}

void KnobGui::setSetKeyActionEnabled(bool e){
    if(_setKeyAction){
        _setKeyAction->setEnabled(e);
    }
}

void KnobGui::setSecret() {
    // If the Knob is within a group, only show it if the group is unfolded!
    // To test it:
    // try TuttlePinning: fold all groups, then switch from perspective to affine to perspective.
    //  VISIBILITY is different from SECRETNESS. The code considers that both things are equivalent, which is wrong.
    // Of course, this check has to be *recursive* (in case the group is within a folded group)
    bool showit = !_knob->isSecret();
    Knob* parentKnob = _knob->getParentKnob();
    while (showit && parentKnob && parentKnob->typeName() == "Group") {
        Group_KnobGui* parentGui = dynamic_cast<Group_KnobGui*>(_container->findKnobGuiOrCreate(parentKnob));
        assert(parentGui);
        // check for secretness and visibility of the group
        if (parentKnob->isSecret() || !parentGui->isChecked()) {
            showit = false; // one of the including groups is folder, so this item is hidden
        }
        // prepare for next loop iteration
        parentKnob = parentKnob->getParentKnob();
    }
    if (showit) {
        show(); //
    } else {
        hide();
    }
}

void KnobGui::showAnimationMenu(){
    _animationMenu->exec(_animationButton->mapToGlobal(QPoint(0,0)));
}

void KnobGui::onShowInCurveEditorActionTriggered(){
    _knob->getHolder()->getApp()->getGui()->setCurveEditorOnTop();
    std::vector<boost::shared_ptr<Curve> > curves;
    for(int i = 0; i < _knob->getDimension();++i){
        boost::shared_ptr<Curve> c = _knob->getCurve(i);
        if(c->isAnimated()){
            curves.push_back(c);
        }
    }
    if(!curves.empty()){
        _knob->getHolder()->getApp()->getGui()->_curveEditor->centerOn(curves);
    }
    
    
}

void KnobGui::onRemoveAnyAnimationActionTriggered(){
    std::vector<std::pair<CurveGui *, boost::shared_ptr<KeyFrame> > > toRemove;
    for(int i = 0; i < _knob->getDimension();++i){
        CurveGui* curve = _knob->getHolder()->getApp()->getGui()->_curveEditor->findCurve(this, i);
        const Curve::KeyFrames& keys = curve->getInternalCurve()->getKeyFrames();
        for(Curve::KeyFrames::const_iterator it = keys.begin();it!=keys.end();++it){
            toRemove.push_back(std::make_pair(curve,*it));
        }
    }
    _knob->getHolder()->getApp()->getGui()->_curveEditor->removeKeyFrames(toRemove);
    //refresh the gui so it doesn't indicate the parameter is animated anymore
    for(int i = 0; i < _knob->getDimension();++i){
        onInternalValueChanged(i);
    }
}

void KnobGui::setInterpolationForDimensions(const std::vector<int>& dimensions,Natron::KeyframeType interp){
    std::vector<boost::shared_ptr<KeyFrame> > keys;
    for(U32 i = 0; i < dimensions.size();++i){
        boost::shared_ptr<Curve> c = _knob->getCurve(dimensions[i]);
        const Curve::KeyFrames& keyframes = c->getKeyFrames();
        for(Curve::KeyFrames::const_iterator it = keyframes.begin();it!=keyframes.end();++it){
            keys.push_back(*it);
        }
    }
    _knob->getHolder()->getApp()->getGui()->_curveEditor->setKeysInterpolation(keys,interp);
}

void KnobGui::onConstantInterpActionTriggered(){
    std::vector<int> dims;
    for(int i = 0; i < _knob->getDimension();++i){
        dims.push_back(i);
    }
    setInterpolationForDimensions(dims,Natron::KEYFRAME_CONSTANT);
}

void KnobGui::onLinearInterpActionTriggered(){
    std::vector<int> dims;
    for(int i = 0; i < _knob->getDimension();++i){
        dims.push_back(i);
    }
    setInterpolationForDimensions(dims,Natron::KEYFRAME_LINEAR);
}

void KnobGui::onSmoothInterpActionTriggered(){
    std::vector<int> dims;
    for(int i = 0; i < _knob->getDimension();++i){
        dims.push_back(i);
    }
    setInterpolationForDimensions(dims,Natron::KEYFRAME_SMOOTH);
}

void KnobGui::onCatmullromInterpActionTriggered(){
    std::vector<int> dims;
    for(int i = 0; i < _knob->getDimension();++i){
        dims.push_back(i);
    }
    setInterpolationForDimensions(dims,Natron::KEYFRAME_CATMULL_ROM);
}

void KnobGui::onCubicInterpActionTriggered(){
    std::vector<int> dims;
    for(int i = 0; i < _knob->getDimension();++i){
        dims.push_back(i);
    }
    setInterpolationForDimensions(dims,Natron::KEYFRAME_CUBIC);
}

void KnobGui::onHorizontalInterpActionTriggered(){
    std::vector<int> dims;
    for(int i = 0; i < _knob->getDimension();++i){
        dims.push_back(i);
    }
    setInterpolationForDimensions(dims,Natron::KEYFRAME_HORIZONTAL);
}

void KnobGui::setKeyframe(SequenceTime time,int dimension){
     _knob->getHolder()->getApp()->getGui()->_curveEditor->addKeyFrame(this,time,dimension);
}

void KnobGui::onSetKeyActionTriggered(){
    
    //get the current time on the global timeline
    SequenceTime time = _knob->getHolder()->getApp()->getTimeLine()->currentFrame();
    for(int i = 0; i < _knob->getDimension();++i){
        setKeyframe(time,i);
    }
    
}

void KnobGui::hide(){
    _hide();
    if(_animationButton)
        _animationButton->hide();
    //also  hide the curve from the curve editor if there's any
    _knob->getHolder()->getApp()->getGui()->_curveEditor->hideCurves(this);
    
}

void KnobGui::show(){
    _show();
    if(_animationButton)
        _animationButton->show();
    //also show the curve from the curve editor if there's any
    _knob->getHolder()->getApp()->getGui()->_curveEditor->showCurves(this);
}

void KnobGui::setEnabledSlot(){
    setEnabled();
    if(!_knob->isEnabled()){
        _knob->getHolder()->getApp()->getGui()->_curveEditor->hideCurves(this);
    }else{
        _knob->getHolder()->getApp()->getGui()->_curveEditor->showCurves(this);
    }
}

void KnobGui::onInternalValueChanged(int dimension){
    if(_widgetCreated)
        updateGUI(dimension,_knob->getValue(dimension));
}

void KnobGui::onCopyValuesActionTriggered(){
    
}

void KnobGui::onCopyAnimationActionTriggered(){
    
}

void KnobGui::onPasteActionTriggered(){
    
}

void KnobGui::onLinkToActionTriggered(){
    
}