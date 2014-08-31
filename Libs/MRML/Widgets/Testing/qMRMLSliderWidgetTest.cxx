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

  This file was originally developed by Julien Finet, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

// Qt includes
#include <QApplication>
#include <QDoubleSpinBox>
#include <QTimer>

// CTK includes
#include <ctkDoubleSpinBox.h>
#include <ctkTest.h>

// MRML includes
#include "qMRMLSliderWidget.h"
#include <vtkMRMLScene.h>
#include <vtkMRMLUnitNode.h>
#include <vtkMRMLSelectionNode.h>

// VTK includes
#include <vtkNew.h>

Q_DECLARE_METATYPE(qMRMLSliderWidget::DisplayTypes);

// ----------------------------------------------------------------------------
class qMRMLSliderWidgetTester: public QObject
{
  Q_OBJECT
  vtkMRMLScene* MRMLScene;
  vtkMRMLUnitNode* MRMLUnitNode;

private:
  void setMicrometerUnit();

private slots:
  /// Run before each test
  void init();

  /// Run after each test
  void cleanup();

  /// Make sure all the default values can be set.
  void testSetDefaults();

  void testUnits();
  void testUnits_data();
};

// ----------------------------------------------------------------------------
void qMRMLSliderWidgetTester::init()
{
  this->MRMLScene = vtkMRMLScene::New();

  vtkNew<vtkMRMLSelectionNode> selectionNode;
  this->MRMLScene->AddNode(selectionNode.GetPointer());

  this->MRMLUnitNode = vtkMRMLUnitNode::New();
  this->MRMLUnitNode->SetQuantity("length");
  this->MRMLScene->AddNode(this->MRMLUnitNode);
  this->MRMLUnitNode->Delete();

  selectionNode->SetUnitNodeID("length", this->MRMLUnitNode->GetID());
}

// ----------------------------------------------------------------------------
void qMRMLSliderWidgetTester::cleanup()
{
  this->MRMLScene->Delete();
}

// ----------------------------------------------------------------------------
void qMRMLSliderWidgetTester::setMicrometerUnit()
{
  this->MRMLUnitNode->SetName("Micrometer");
  this->MRMLUnitNode->SetSuffix("µm");
  this->MRMLUnitNode->SetPrecision(3);
  this->MRMLUnitNode->SetMinimumValue(-10000.);
  this->MRMLUnitNode->SetMaximumValue(22.);
  this->MRMLUnitNode->SetDisplayCoefficient(1000.);
  this->MRMLUnitNode->SetDisplayOffset(0.);
}

// ----------------------------------------------------------------------------
void qMRMLSliderWidgetTester::testSetDefaults()
{
  qMRMLSliderWidget sliderWidget;
  sliderWidget.setDecimals(1);
  sliderWidget.setRange(-5., 1234.);
  sliderWidget.setValue(30.5);
  sliderWidget.setSingleStep(0.1);
  sliderWidget.setPageStep(10.);
  sliderWidget.setSuffix("v");

  QCOMPARE(sliderWidget.decimals(), 1);
  QCOMPARE(sliderWidget.spinBox()->spinBox()->decimals(), 1);
  QCOMPARE(sliderWidget.minimum(), -5.);
  QCOMPARE(sliderWidget.maximum(), 1234.);
  QCOMPARE(sliderWidget.value(), 30.5);
  QCOMPARE(sliderWidget.spinBox()->spinBox()->value(), 30.5);
  QCOMPARE(sliderWidget.singleStep(), 0.1);
  QCOMPARE(sliderWidget.spinBox()->spinBox()->singleStep(), 0.1);
  QCOMPARE(sliderWidget.pageStep(), 10.);
  QCOMPARE(sliderWidget.suffix(), QString("v"));
  QCOMPARE(sliderWidget.spinBox()->spinBox()->suffix(), QString("v"));

  // Make sure those are similar to the documentation.
  const qMRMLSliderWidget::DisplayTypes useUnitDefault = qMRMLSliderWidget::UseUnitDefault;
  const qMRMLSliderWidget::DisplayTypes useUnitScale = qMRMLSliderWidget::UseUnitScale;

  QCOMPARE(sliderWidget.displayDecimals(), useUnitDefault);
  QCOMPARE(sliderWidget.displayMinimum(), useUnitScale);
  QCOMPARE(sliderWidget.displayMaximum(), useUnitScale);
  QCOMPARE(sliderWidget.displayValue(), useUnitScale);
  QCOMPARE(sliderWidget.displayPrefix(), useUnitDefault);
  QCOMPARE(sliderWidget.displaySuffix(), useUnitDefault);
}

// ----------------------------------------------------------------------------
void qMRMLSliderWidgetTester::testUnits()
{
  qMRMLSliderWidget sliderWidget;
  sliderWidget.setDecimals(1);
  sliderWidget.setRange(-5., 1234.0);
  sliderWidget.setValue(30.5);
  sliderWidget.setSingleStep(0.1);
  sliderWidget.setPageStep(10.);
  sliderWidget.setSuffix("v");
  sliderWidget.setQuantity("length");

  QFETCH(qMRMLSliderWidget::DisplayTypes, displayDecimals);
  QFETCH(qMRMLSliderWidget::DisplayTypes, displayMinimum);
  QFETCH(qMRMLSliderWidget::DisplayTypes, displayMaximum);
  QFETCH(qMRMLSliderWidget::DisplayTypes, displayValue);
  QFETCH(qMRMLSliderWidget::DisplayTypes, displayPrefix);
  QFETCH(qMRMLSliderWidget::DisplayTypes, displaySuffix);

  sliderWidget.setDisplayDecimals(displayDecimals);
  sliderWidget.setDisplayMinimum(displayMinimum);
  sliderWidget.setDisplayMaximum(displayMaximum);
  sliderWidget.setDisplayValue(displayValue);
  sliderWidget.setDisplayPrefix(displayPrefix);
  sliderWidget.setDisplaySuffix(displaySuffix);

  this->setMicrometerUnit();
  sliderWidget.setMRMLScene(this->MRMLScene);
  //sliderWidget.show();
  //qApp->exec();

  QFETCH(int, expectedDecimals);
  QFETCH(double, expectedMinimum);
  QFETCH(double, expectedMaximum);
  QFETCH(double, expectedValue);
  QFETCH(double, expectedSingleStep);
  QFETCH(QString, expectedSuffix);

  QCOMPARE(sliderWidget.spinBox()->spinBox()->decimals(), expectedDecimals);
  QCOMPARE(sliderWidget.spinBox()->spinBox()->minimum(), expectedMinimum);
  QCOMPARE(sliderWidget.spinBox()->spinBox()->maximum(), expectedMaximum);
  QCOMPARE(sliderWidget.spinBox()->spinBox()->value(), expectedValue);
  QCOMPARE(sliderWidget.spinBox()->spinBox()->singleStep(), expectedSingleStep);
  QCOMPARE(sliderWidget.spinBox()->spinBox()->suffix(), expectedSuffix);
}

// ----------------------------------------------------------------------------
void qMRMLSliderWidgetTester::testUnits_data()
{
  QTest::addColumn<qMRMLSliderWidget::DisplayTypes>("displayDecimals");
  QTest::addColumn<qMRMLSliderWidget::DisplayTypes>("displayMinimum");
  QTest::addColumn<qMRMLSliderWidget::DisplayTypes>("displayMaximum");
  QTest::addColumn<qMRMLSliderWidget::DisplayTypes>("displayValue");
  QTest::addColumn<qMRMLSliderWidget::DisplayTypes>("displayPrefix");
  QTest::addColumn<qMRMLSliderWidget::DisplayTypes>("displaySuffix");

  QTest::addColumn<int>("expectedDecimals");
  QTest::addColumn<double>("expectedMinimum");
  QTest::addColumn<double>("expectedMaximum");
  QTest::addColumn<double>("expectedValue");
  QTest::addColumn<double>("expectedSingleStep");
  QTest::addColumn<QString>("expectedSuffix");

  const qMRMLSliderWidget::DisplayTypes ignoreUnit =
    qMRMLSliderWidget::IgnoreUnit;
  const qMRMLSliderWidget::DisplayTypes useUnitDefault =
    qMRMLSliderWidget::UseUnitDefault;
  const qMRMLSliderWidget::DisplayTypes useUnitScale =
    qMRMLSliderWidget::UseUnitScale;
  const qMRMLSliderWidget::DisplayTypes useValueOrderOfMagnitudePlus2 =
    qMRMLSliderWidget::UseDefaultValueOrderOfMagnitudePlus2;

  QTest::newRow("IgnoreUnit") << ignoreUnit << ignoreUnit
                              << ignoreUnit << ignoreUnit
                              << ignoreUnit << ignoreUnit
                              << 1 << -5. << 1234. << 30.5 << 0.1 << "v";
  QTest::newRow("slider defaults") << useUnitDefault << useUnitScale
                                   << useUnitScale << useUnitScale
                                   << useUnitDefault << useUnitDefault
                                   << 3 << -5000. << 1234000. << 30500. << 10000. << "µm";
  QTest::newRow("unit defaults") << useUnitDefault << useUnitDefault
                                 << useUnitDefault << useUnitDefault
                                 << useUnitDefault << useUnitDefault
                                 << 3 << -10000. << 22. << 22. << 100. << "µm";

  QTest::newRow("Max is O+2") << useUnitDefault << useUnitScale
                              << useValueOrderOfMagnitudePlus2 << useUnitScale
                              << useUnitDefault << useUnitDefault
                              << 3 << -5000. << 1000. << 1000. << 100. << "µm";
}


// ----------------------------------------------------------------------------
CTK_TEST_MAIN(qMRMLSliderWidgetTest)
#include "moc_qMRMLSliderWidgetTest.cxx"
