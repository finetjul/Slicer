/*==============================================================================

  Program: 3D Slicer

  Copyright (c) Kitware Inc.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Johan Andruejol, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

#include "qMRMLSliderWidget.h"

// CTK includes
#include <ctkLinearValueProxy.h>
#include <ctkUtils.h>

// MRML includes
#include <vtkMRMLNode.h>
#include <vtkMRMLScene.h>
#include <vtkMRMLSelectionNode.h>
#include <vtkMRMLUnitNode.h>

// STD includes
#include <cmath>

// VTK includes
#include <vtkCommand.h>

// --------------------------------------------------------------------------
class qMRMLSliderWidgetPrivate
{
  Q_DECLARE_PUBLIC(qMRMLSliderWidget);
protected:
  qMRMLSliderWidget* const q_ptr;
public:
  qMRMLSliderWidgetPrivate(qMRMLSliderWidget& object);
  ~qMRMLSliderWidgetPrivate();

  void setAndObserveSelectionNode();
  void updateValueProxy(vtkMRMLUnitNode* unitNode);
  void updateUnitAwareProperties();
  double initValue(qMRMLSliderWidget::DisplayTypes displayTypes,
                   double originalValue,
                   vtkMRMLUnitNode* unitNode)const;

  QString Quantity;
  vtkMRMLScene* MRMLScene;
  vtkMRMLSelectionNode* SelectionNode;
  //qMRMLSliderWidget::UnitAwareProperties Flags;
  qMRMLSliderWidget::DisplayTypes DisplayDecimals;
  qMRMLSliderWidget::DisplayTypes DisplayMinimum;
  qMRMLSliderWidget::DisplayTypes DisplayMaximum;
  qMRMLSliderWidget::DisplayTypes DisplayValue;
  qMRMLSliderWidget::DisplayTypes DisplayPrefix;
  qMRMLSliderWidget::DisplayTypes DisplaySuffix;
  ctkLinearValueProxy* Proxy;
};

// --------------------------------------------------------------------------
qMRMLSliderWidgetPrivate::qMRMLSliderWidgetPrivate(qMRMLSliderWidget& object)
  : q_ptr(&object)
{
  this->Quantity = "";
  this->MRMLScene = 0;
  this->SelectionNode = 0;
  //this->Flags = qMRMLSliderWidget::Prefix | qMRMLSliderWidget::Suffix
  //  | qMRMLSliderWidget::Precision | qMRMLSliderWidget::Scaling;

  this->DisplayDecimals = qMRMLSliderWidget::UseUnitDefault;
  this->DisplayMinimum = qMRMLSliderWidget::UseUnitScale;
  this->DisplayMaximum = qMRMLSliderWidget::UseUnitScale;
  this->DisplayValue = qMRMLSliderWidget::UseUnitScale;
  this->DisplayPrefix = qMRMLSliderWidget::UseUnitDefault;
  this->DisplaySuffix = qMRMLSliderWidget::UseUnitDefault;

  this->Proxy = new ctkLinearValueProxy;
}

// --------------------------------------------------------------------------
qMRMLSliderWidgetPrivate::~qMRMLSliderWidgetPrivate()
{
  delete this->Proxy;
}

// --------------------------------------------------------------------------
void qMRMLSliderWidgetPrivate::setAndObserveSelectionNode()
{
  Q_Q(qMRMLSliderWidget);

  vtkMRMLSelectionNode* selectionNode = 0;
  if (this->MRMLScene)
    {
    selectionNode = vtkMRMLSelectionNode::SafeDownCast(
      this->MRMLScene->GetNthNodeByClass(0, "vtkMRMLSelectionNode"));
    }

  q->qvtkReconnect(this->SelectionNode, selectionNode,
    vtkMRMLSelectionNode::UnitModifiedEvent,
    q, SLOT(updateWidgetFromUnitNode()));
  this->SelectionNode = selectionNode;
  q->initWidgetFromUnitNode();
  q->updateWidgetFromUnitNode();
}

// --------------------------------------------------------------------------
void qMRMLSliderWidgetPrivate::updateValueProxy(vtkMRMLUnitNode* unitNode)
{
  Q_Q(qMRMLSliderWidget);
  if (!unitNode)
    {
    q->setValueProxy(0);
    this->Proxy->setCoefficient(1.0);
    this->Proxy->setOffset(0.0);
    return;
    }

  q->setValueProxy(this->Proxy);
  this->Proxy->setOffset(unitNode->GetDisplayOffset());
  this->Proxy->setCoefficient(unitNode->GetDisplayCoefficient());
}

// --------------------------------------------------------------------------
double qMRMLSliderWidgetPrivate::initValue(qMRMLSliderWidget::DisplayTypes displayTypes,
                                         double originalValue,
                                         vtkMRMLUnitNode* unitNode)const
{
  Q_Q(const qMRMLSliderWidget);
  double res = originalValue;
  if (displayTypes.testFlag(qMRMLSliderWidget::Zero))
    {
    res = 0.;
    }
  else if (displayTypes.testFlag(qMRMLSliderWidget::One))
    {
    res = 1.;
    }
  else if (displayTypes.testFlag(qMRMLSliderWidget::UseDefaultValueOrderOfMagnitudePlus2))
    {
    res = std::pow(10., ctk::orderOfMagnitude(q->value()) + 2);
    }
  if (this->DisplayValue.testFlag(qMRMLSliderWidget::UseUnitScale) &&
      !displayTypes.testFlag(qMRMLSliderWidget::UseUnitScale))
    {
    res = unitNode->GetValueFromDisplayValue(res);
    }
  return res;
}
// --------------------------------------------------------------------------
// qMRMLSliderWidget

// --------------------------------------------------------------------------
qMRMLSliderWidget::qMRMLSliderWidget(QWidget* parentWidget)
  : Superclass(parentWidget)
  , d_ptr(new qMRMLSliderWidgetPrivate(*this))
{
}

// --------------------------------------------------------------------------
qMRMLSliderWidget::~qMRMLSliderWidget()
{
}

//-----------------------------------------------------------------------------
void qMRMLSliderWidget::setQuantity(const QString& quantity)
{
  Q_D(qMRMLSliderWidget);
  if (quantity == d->Quantity)
    {
    return;
    }

  d->Quantity = quantity;
  this->updateWidgetFromUnitNode();
}

//-----------------------------------------------------------------------------
QString qMRMLSliderWidget::quantity()const
{
  Q_D(const qMRMLSliderWidget);
  return d->Quantity;
}

// --------------------------------------------------------------------------
vtkMRMLScene* qMRMLSliderWidget::mrmlScene()const
{
  Q_D(const qMRMLSliderWidget);
  return d->MRMLScene;
}

// --------------------------------------------------------------------------
void qMRMLSliderWidget::setMRMLScene(vtkMRMLScene* scene)
{
  Q_D(qMRMLSliderWidget);

  if (this->mrmlScene() == scene)
    {
    return;
    }

  d->MRMLScene = scene;
  d->setAndObserveSelectionNode();

  this->setEnabled(scene != 0);
}

// --------------------------------------------------------------------------
qMRMLSliderWidget::DisplayTypes
qMRMLSliderWidget::displayDecimals()const
{
  Q_D(const qMRMLSliderWidget);
  return d->DisplayDecimals;
}

// --------------------------------------------------------------------------
void qMRMLSliderWidget::setDisplayDecimals(DisplayTypes newFlags)
{
  Q_D(qMRMLSliderWidget);
  if (newFlags == d->DisplayDecimals)
    {
    return;
    }

  d->DisplayDecimals = newFlags;
  emit unitAwarePropertiesChanged();
}

// --------------------------------------------------------------------------
qMRMLSliderWidget::DisplayTypes
qMRMLSliderWidget::displayMinimum()const
{
  Q_D(const qMRMLSliderWidget);
  return d->DisplayMinimum;
}

// --------------------------------------------------------------------------
void qMRMLSliderWidget::setDisplayMinimum(DisplayTypes newFlags)
{
  Q_D(qMRMLSliderWidget);
  if (newFlags == d->DisplayMinimum)
    {
    return;
    }

  d->DisplayMinimum = newFlags;
  emit unitAwarePropertiesChanged();
}

// --------------------------------------------------------------------------
qMRMLSliderWidget::DisplayTypes
qMRMLSliderWidget::displayMaximum()const
{
  Q_D(const qMRMLSliderWidget);
  return d->DisplayMaximum;
}

// --------------------------------------------------------------------------
void qMRMLSliderWidget::setDisplayMaximum(DisplayTypes newFlags)
{
  Q_D(qMRMLSliderWidget);
  if (newFlags == d->DisplayMaximum)
    {
    return;
    }

  d->DisplayMaximum = newFlags;
  emit unitAwarePropertiesChanged();
}

// --------------------------------------------------------------------------
qMRMLSliderWidget::DisplayTypes
qMRMLSliderWidget::displayValue()const
{
  Q_D(const qMRMLSliderWidget);
  return d->DisplayValue;
}

// --------------------------------------------------------------------------
void qMRMLSliderWidget::setDisplayValue(DisplayTypes newFlags)
{
  Q_D(qMRMLSliderWidget);
  if (newFlags == d->DisplayValue)
    {
    return;
    }

  d->DisplayValue= newFlags;
  emit unitAwarePropertiesChanged();
}

// --------------------------------------------------------------------------
qMRMLSliderWidget::DisplayTypes
qMRMLSliderWidget::displayPrefix()const
{
  Q_D(const qMRMLSliderWidget);
  return d->DisplayPrefix;
}

// --------------------------------------------------------------------------
void qMRMLSliderWidget::setDisplayPrefix(DisplayTypes newFlags)
{
  Q_D(qMRMLSliderWidget);
  if (newFlags == d->DisplayPrefix)
    {
    return;
    }

  d->DisplayPrefix = newFlags;
  emit unitAwarePropertiesChanged();
}

// --------------------------------------------------------------------------
qMRMLSliderWidget::DisplayTypes
qMRMLSliderWidget::displaySuffix()const
{
  Q_D(const qMRMLSliderWidget);
  return d->DisplaySuffix;
}

// --------------------------------------------------------------------------
void qMRMLSliderWidget::setDisplaySuffix(DisplayTypes newFlags)
{
  Q_D(qMRMLSliderWidget);
  if (newFlags == d->DisplaySuffix)
    {
    return;
    }

  d->DisplaySuffix = newFlags;
  emit unitAwarePropertiesChanged();
}


// --------------------------------------------------------------------------
qMRMLSliderWidget::UnitAwareProperties
qMRMLSliderWidget::unitAwareProperties()const
{
  Q_D(const qMRMLSliderWidget);
  qMRMLSliderWidget::UnitAwareProperties flags = qMRMLSliderWidget::None;
  if (d->DisplayDecimals)
    {
    flags |= qMRMLSliderWidget::Precision;
    }
  if (d->DisplayMinimum)
    {
    flags |= qMRMLSliderWidget::MinimumValue;
    }
  if (d->DisplayMaximum)
    {
    flags |= qMRMLSliderWidget::MaximumValue;
    }
  if (d->DisplayValue)
    {
    flags |= qMRMLSliderWidget::Scaling;
    }
  if (d->DisplayPrefix)
    {
    flags |= qMRMLSliderWidget::Prefix;
    }
  if (d->DisplaySuffix)
    {
    flags |= qMRMLSliderWidget::Suffix;
    }
  return flags;
}

// --------------------------------------------------------------------------
void qMRMLSliderWidget::setUnitAwareProperties(UnitAwareProperties newFlags)
{
  Q_D(qMRMLSliderWidget);
  if (newFlags == this->unitAwareProperties())
    {
    return;
    }
  if (newFlags.testFlag(qMRMLSliderWidget::Precision))
    {
    d->DisplayDecimals |= qMRMLSliderWidget::UseUnitDefault;
    }
  if (newFlags.testFlag(qMRMLSliderWidget::MinimumValue))
    {
    d->DisplayMinimum |= qMRMLSliderWidget::UseUnitDefault;
    }
  if (newFlags.testFlag(qMRMLSliderWidget::MaximumValue))
    {
    d->DisplayMaximum |= qMRMLSliderWidget::UseUnitDefault;
    }
  if (newFlags.testFlag(qMRMLSliderWidget::Scaling))
    {
    d->DisplayValue |= qMRMLSliderWidget::UseUnitScale;
    }
  if (newFlags.testFlag(qMRMLSliderWidget::Prefix))
    {
    d->DisplayPrefix |= qMRMLSliderWidget::UseUnitDefault;
    }
  if (newFlags.testFlag(qMRMLSliderWidget::Suffix))
    {
    d->DisplaySuffix |= qMRMLSliderWidget::UseUnitDefault;
    }
  emit unitAwarePropertiesChanged();
}


// --------------------------------------------------------------------------
void qMRMLSliderWidget::initWidgetFromUnitNode()
{
  Q_D(qMRMLSliderWidget);

  vtkMRMLUnitNode* unitNode = vtkMRMLUnitNode::SafeDownCast(
    d->MRMLScene ? d->MRMLScene->GetNodeByID(
      d->SelectionNode ? d->SelectionNode->GetUnitNodeID(d->Quantity.toLatin1()) :
                         "") :
      0);

  if (!unitNode)
    {
    return;
    }
  // Here unit scaling has not yet been applied.
  double newMinimum = d->initValue(d->DisplayMinimum, this->minimum(), unitNode);
  double newMaximum = d->initValue(d->DisplayMaximum, this->maximum(), unitNode);
  this->Superclass::setRange(newMinimum, newMaximum);

  this->updateWidgetFromUnitNode();
  // Here unit scaling has been applied.
  if (d->DisplayValue.testFlag(qMRMLSliderWidget::UseUnitDefault))
    {
    //this->setValue(unitNode->GetDefaultValue());
    }
}

// --------------------------------------------------------------------------
void qMRMLSliderWidget::updateWidgetFromUnitNode()
{
  Q_D(qMRMLSliderWidget);

  vtkMRMLUnitNode* unitNode = vtkMRMLUnitNode::SafeDownCast(
    d->MRMLScene ? d->MRMLScene->GetNodeByID(
      d->SelectionNode ? d->SelectionNode->GetUnitNodeID(d->Quantity.toLatin1()) :
                         "") :
      0);

  if (!unitNode)
    {
    return;
    }
  if (d->DisplayDecimals.testFlag(qMRMLSliderWidget::UseUnitDefault))
    {
    this->setDecimals(unitNode->GetPrecision());
    }
  if (d->DisplayMinimum.testFlag(qMRMLSliderWidget::UseUnitDefault))
    {
    this->Superclass::setMinimum(unitNode->GetMinimumValue());
    }
  if (d->DisplayMaximum.testFlag(qMRMLSliderWidget::UseUnitDefault))
    {
    this->Superclass::setMaximum(unitNode->GetMaximumValue());
    }
  if (d->DisplayValue.testFlag(qMRMLSliderWidget::UseUnitScale))
    {
    d->updateValueProxy(unitNode);
    }
  if (d->DisplayPrefix.testFlag(qMRMLSliderWidget::UseUnitDefault))
    {
    this->setPrefix(unitNode->GetPrefix());
    }
  if (d->DisplaySuffix.testFlag(qMRMLSliderWidget::UseUnitDefault))
    {
    this->setSuffix(unitNode->GetSuffix());
    }
  if (d->DisplayDecimals.testFlag(qMRMLSliderWidget::UseUnitDefault))
    {
    double range = this->maximum() - this->minimum();
    if (d->DisplayValue.testFlag(qMRMLSliderWidget::UseUnitScale))
      {
      range = unitNode->GetDisplayValueFromValue(this->maximum()) -
              unitNode->GetDisplayValueFromValue(this->minimum());
      }
    double powerOfTen = ctk::closestPowerOfTen(range);
    if (powerOfTen != 0.)
      {
      this->setSingleStep(powerOfTen / 100);
      }
    }
}

// --------------------------------------------------------------------------
void qMRMLSliderWidget::setMinimum(double newMinimumValue)
{
  this->Superclass::setMinimum(newMinimumValue);
  if (this->unitAwareProperties().testFlag(qMRMLSliderWidget::Precision))
    {
    this->updateWidgetFromUnitNode();
    }
}

// --------------------------------------------------------------------------
void qMRMLSliderWidget::setMaximum(double newMaximumValue)
{
  this->Superclass::setMaximum(newMaximumValue);
  if (this->unitAwareProperties().testFlag(qMRMLSliderWidget::Precision))
    {
    this->updateWidgetFromUnitNode();
    }
}

// --------------------------------------------------------------------------
void qMRMLSliderWidget::setRange(double newMinimumValue, double newMaximumValue)
{
  this->Superclass::setRange(newMinimumValue, newMaximumValue);
  if (this->unitAwareProperties().testFlag(qMRMLSliderWidget::Precision))
    {
    this->updateWidgetFromUnitNode();
    }
}
