/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkAnnotationROIRepresentation2D.cxx,v $

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/

// Annotations includes
#include "vtkAnnotationROIRepresentation2D.h"

// VTK includes
#include "vtkActor2D.h"
#include "vtkSphereSource.h"
#include "vtkPolyDataMapper2D.h"
#include "vtkPolyData.h"
#include "vtkCallbackCommand.h"
#include "vtkBox.h"
#include "vtkPolyData.h"
#include "vtkProperty2D.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkRenderer.h"
#include "vtkInteractorObserver.h"
#include "vtkMath.h"
#include "vtkCellArray.h"
#include "vtkPropPicker.h"
#include "vtkDoubleArray.h"
#include "vtkFloatArray.h"
#include "vtkPlanes.h"
#include "vtkCamera.h"
#include "vtkAssemblyPath.h"
#include "vtkWindow.h"
#include "vtkProperty2D.h"
#include "vtkObjectFactory.h"

#include <vtkSmartPointer.h>
#include <vtkCutter.h>
#include <vtkDoubleArray.h>
#include <vtkFloatArray.h>
#include <vtkInteractorObserver.h>
#include <vtkMath.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>
#include <vtkPlane.h>
#include <vtkPlanes.h>
#include <vtkPolyData.h>
#include <vtkPolyDataMapper2D.h>
#include <vtkProperty2D.h>
#include <vtkPropPicker.h>
#include <vtkRenderer.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkSphereSource.h>
#include <vtkTransform.h>
#include <vtkTransformPolyDataFilter.h>
#include <vtkWindow.h>


vtkCxxRevisionMacro(vtkAnnotationROIRepresentation2D, "$Revision: 12141 $");
vtkStandardNewMacro(vtkAnnotationROIRepresentation2D);

//----------------------------------------------------------------------------
vtkAnnotationROIRepresentation2D::vtkAnnotationROIRepresentation2D()
{
  this->LastPicker2D = NULL;

  this->HandleSize = 4.0;

  // Set up the initial properties
  this->CreateDefaultProperties();

  // Create the handles
  this->Handle2D = new vtkActor2D* [7];
  this->HandleMapper2D = new vtkPolyDataMapper2D* [7];
  this->HandleToPlaneTransformFilters = new vtkTransformPolyDataFilter* [7];

  this->IntersectionPlane = vtkPlane::New();
  double normal[3]={1,0,0};
  double origin[3]={0,0,0};
  this->IntersectionPlane->SetNormal(normal);
  this->IntersectionPlane->SetOrigin(origin);

  this->WorldToDisplayTransform = vtkTransform::New();

  this->HandlePicker2D = vtkPropPicker::New();
  this->HandlePicker2D->PickFromListOn();

  this->CreateFaceIntersections();

  // 2D handles pipelines
  int i;
  for (i=0; i<7; i++)
    {
    this->HandleToPlaneTransformFilters[i] = vtkTransformPolyDataFilter::New();
    this->HandleToPlaneTransformFilters[i]->SetInput(this->HandleGeometry[i]->GetOutput());
    this->HandleToPlaneTransformFilters[i]->SetTransform(this->WorldToDisplayTransform);

    this->HandleMapper2D[i] = vtkPolyDataMapper2D::New();
    this->HandleMapper2D[i]->SetInput(this->HandleToPlaneTransformFilters[i]->GetOutput());
    this->Handle2D[i] = vtkActor2D::New();
    this->Handle2D[i]->SetMapper(this->HandleMapper2D[i]);

    this->HandlePicker2D->AddPickList(this->Handle2D[i]);
    this->Handle2D[i]->SetProperty(this->HandleProperties2D[i]);
    }

  this->LastPicker2D = NULL;
  this->CurrentHandle2D = NULL;

}

//----------------------------------------------------------------------------
vtkAnnotationROIRepresentation2D::~vtkAnnotationROIRepresentation2D()
{  
  this->HandlePicker2D->Delete();
  int i;
  for (i=0; i<7; i++)
    {
    this->HandleToPlaneTransformFilters[i]->Delete();
    this->HandleMapper2D[i]->Delete();
    this->Handle2D[i]->Delete();
    }
  delete [] this->Handle2D;
  delete [] this->HandleMapper2D;
  delete [] this->HandleToPlaneTransformFilters;
 
  this->IntersectionPlane->Delete();
  this->WorldToDisplayTransform->Delete();
  for (i=0; i<6; i++)
    {
    this->IntersectionFaces[i]->Delete();
    this->IntersectionCutters[i]->Delete();
    this->IntersectionPlaneTransformFilters[i]->Delete();
    this->IntersectionMappers[i]->Delete();
    this->IntersectionActors[i]->Delete();
    }
  for(int i=0;i<NUMBER_HANDLES;i++)
    {
    this->HandleProperties2D[i]->Delete();
    this->HandleProperties2D[i]=NULL;
    }
  this->SelectedHandleProperty2D->Delete();
  this->SelectedFaceProperty2D->Delete();
  if (this->DefaultFaceProperty2D)
    {
    this->DefaultFaceProperty2D->Delete();
    }
}

//----------------------------------------------------------------------
void vtkAnnotationROIRepresentation2D::CreateFaceIntersections()
{
  int i;

  // Create Plane/Face intersection pipelines
  int faceIndex[6][4] = {
    {3, 0, 4, 7},
    {1, 2, 6, 5},
    {0, 1, 5, 4},
    {2, 3, 7, 6},
    {0, 3, 2, 1},
    {4, 5, 6, 7}};

  for (i=0; i<6; i++)
    {
    this->IntersectionFaces[i] = vtkPolyData::New();
    this->IntersectionFaces[i]->SetPoints(this->Points);
    vtkIdType pts[4];
    for (int j=0; j<4; j++)
      {
      pts[j] = faceIndex[i][j];
      }
    vtkCellArray *cells = vtkCellArray::New();
    cells->Allocate(cells->EstimateSize(1,4));
    cells->InsertNextCell(4,pts);
    this->IntersectionFaces[i]->SetPolys(cells);
    this->IntersectionFaces[i]->BuildCells();
    cells->Delete();

    this->IntersectionCutters[i] = vtkCutter::New();
    this->IntersectionCutters[i]->SetInput(this->IntersectionFaces[i]);
    this->IntersectionCutters[i]->SetCutFunction(this->IntersectionPlane);

    this->IntersectionPlaneTransformFilters[i] = vtkTransformPolyDataFilter::New();
    this->IntersectionPlaneTransformFilters[i]->SetInput(this->IntersectionCutters[i]->GetOutput());
    this->IntersectionPlaneTransformFilters[i]->SetTransform(this->WorldToDisplayTransform);

    this->IntersectionMappers[i] = vtkPolyDataMapper2D::New();
    this->IntersectionMappers[i]->SetInput(this->IntersectionPlaneTransformFilters[i]->GetOutput());

    this->IntersectionActors[i] = vtkActor2D::New();
    this->IntersectionActors[i]->SetMapper(this->IntersectionMappers[i]);
    }
}


//----------------------------------------------------------------------------
void vtkAnnotationROIRepresentation2D::BuildRepresentation()
{
  // Rebuild only if necessary
  if ( this->GetMTime() > this->BuildTime ||
       (this->Renderer && this->Renderer->GetVTKWindow() &&
        this->Renderer->GetVTKWindow()->GetMTime() > this->BuildTime) )
    {
    this->PositionHandles();
    this->SizeHandles();

    // Handle visibility
    bool atLeast1HandleVisible = false;
    for (int i=0; i < 6; ++i)
      {
      bool visible = this->HandleVisibility
        && (this->IntersectionCutters[i]->GetOutput()->GetNumberOfLines() > 0);
      this->Handle2D[i]->SetVisibility(visible);
      atLeast1HandleVisible = atLeast1HandleVisible || visible;
      }
    this->Handle2D[6]->SetVisibility(atLeast1HandleVisible);

    this->BuildTime.Modified();
    }
}

//----------------------------------------------------------------------
void vtkAnnotationROIRepresentation2D::GetActors(vtkPropCollection *vtkNotUsed(actors))
{
  // Intentionally empty to disable/hide all the 3D actors that are not used.
}

//----------------------------------------------------------------------
void vtkAnnotationROIRepresentation2D::GetActors2D(vtkPropCollection *actors)
{
  this->GetIntersectionActors(actors);
  for (int i=0; i<7; i++)
    {
    actors->AddItem(this->Handle2D[i]);
    }
}

//----------------------------------------------------------------------
void vtkAnnotationROIRepresentation2D::GetIntersectionActors(vtkPropCollection *actors)
{
  for (int i=0; i<6; i++)
    {
    actors->AddItem(this->IntersectionActors[i]);
    }
}

//----------------------------------------------------------------------------
void vtkAnnotationROIRepresentation2D::CreateDefaultProperties()
{
  for(int i=0;i<NUMBER_HANDLES;i++)
    {
    this->HandleProperties2D[i]=vtkProperty2D::New();
    }

  // lavender
  this->HandleProperties2D[0]->SetColor(.781, .633, .867);
  // dark violet
  this->HandleProperties2D[1]->SetColor(.5585, .343, .91);
  // dark red
  //this->HandleProperties2D[2]->SetColor(.51562, .38281, .15234);
  this->HandleProperties2D[2]->SetColor(.75, .121, .26953);
  // orange
  //this->HandleProperties2D[3]->SetColor(.9101, .39453, 0.0);
  this->HandleProperties2D[3]->SetColor(.9765, .488, .1133);
  // dark blue
  //this->HandleProperties2D[4]->SetColor(.140625, .30468, .5);
  this->HandleProperties2D[4]->SetColor(.1328, .4531, .5351);
  // light blue
  //this->HandleProperties2D[5]->SetColor(.33984, .69140, .71875);
  this->HandleProperties2D[5]->SetColor(.582, .898, .871);
  // yellow
  //this->HandleProperties2D[6]->SetColor(0.953125, .738281, 0.0);
  this->HandleProperties2D[6]->SetColor(0.973125, .798281, 0.0);

  this->SelectedHandleProperty2D = vtkProperty2D::New();
  this->SelectedHandleProperty2D->SetColor(.2,1,.2);
  
  this->SelectedFaceProperty2D = vtkProperty2D::New();
  this->SelectedFaceProperty2D->SetColor(1,1,0);
  this->SelectedFaceProperty2D->SetOpacity(0.25);

  this->DefaultFaceProperty2D = vtkProperty2D::New();
}

//----------------------------------------------------------------------
void vtkAnnotationROIRepresentation2D::SetInteractionState(int state)
{
  // Clamp to allowable values
  state = ( state < vtkAnnotationROIRepresentation::Outside ? vtkAnnotationROIRepresentation::Outside :
            (state > vtkAnnotationROIRepresentation::Scaling ? vtkAnnotationROIRepresentation::Scaling : state) );
  
  // Depending on state, highlight appropriate parts of representation
  int handle;
  this->InteractionState = state;
  switch (state)
    {
    case vtkAnnotationROIRepresentation::MoveF0:
    case vtkAnnotationROIRepresentation::MoveF1:
    case vtkAnnotationROIRepresentation::MoveF2:
    case vtkAnnotationROIRepresentation::MoveF3:
    case vtkAnnotationROIRepresentation::MoveF4:
    case vtkAnnotationROIRepresentation::MoveF5:
      handle = this->HighlightHandle(this->CurrentHandle2D);
      this->HighlightFace(handle);
      break;
    case vtkAnnotationROIRepresentation::Rotating:
      //this->HighlightOutline(0);
      //this->HighlightHandle(NULL);
      //this->HighlightFace(this->HexPicker->GetCellId());
      break;
    case vtkAnnotationROIRepresentation::Translating:
    case vtkAnnotationROIRepresentation::Scaling:
      this->HighlightHandle(this->Handle2D[6]);
      this->HighlightFace(-1);
      break;
    default:
      this->HighlightHandle(NULL);
      this->HighlightFace(-1);
    }
}

//----------------------------------------------------------------------------
int vtkAnnotationROIRepresentation2D::HighlightHandle(vtkProp *prop)
{

  for (int i = 0; i < NUMBER_HANDLES; i++)
    {
      this->Handle2D[i]->SetProperty(this->HandleProperties2D[i]);
    }
  this->CurrentHandle2D = NULL;
  if (prop)
    {
    this->CurrentHandle2D = static_cast<vtkActor2D *>(prop);
    }

  if ( this->CurrentHandle2D )
    {
    this->CurrentHandle2D->SetProperty(this->SelectedHandleProperty2D);
    for (int i=0; i<6; i++) //find attached face
      {
      if ( this->CurrentHandle2D == this->Handle2D[i] )
        {
        return i;
        }
      }
    }
  
  if ( this->CurrentHandle2D == this->Handle2D[6] )
    {
    return 6;
    }
  
  return -1;
}

//----------------------------------------------------------------------------
void vtkAnnotationROIRepresentation2D::HighlightFace(int cellId)
{
  for (int i=0; i<6; i++)
    {
    this->IntersectionActors[i]->SetProperty(this->DefaultFaceProperty2D);
    }

  if ( cellId >= 0 )
    {
    this->IntersectionActors[cellId]->SetProperty(this->SelectedFaceProperty2D);
    }
}

//----------------------------------------------------------------------------
void vtkAnnotationROIRepresentation2D::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}

#define VTK_AVERAGE(a,b,c) \
  c[0] = (a[0] + b[0])/2.0; \
  c[1] = (a[1] + b[1])/2.0; \
  c[2] = (a[2] + b[2])/2.0;

//----------------------------------------------------------------------------
void vtkAnnotationROIRepresentation2D::PositionHandles()
{
  this->Points->GetData()->Modified();
  this->Points->Modified();
  for (int i=0; i<6; i++)
    {
    // Update edges
    this->IntersectionFaces[i]->Modified();
    this->IntersectionCutters[i]->Update();
    vtkPolyData* roiEdge = this->IntersectionCutters[i]->GetOutput();
    if (roiEdge->GetNumberOfLines() == 0)
      {
      continue;
      }
    // Update handles
    double pi0[3];
    double pi1[3];
    roiEdge->GetPoint(0, pi0);
    roiEdge->GetPoint(1, pi1);
    double x[3];
    VTK_AVERAGE(pi0,pi1,x);
    this->Points->SetPoint(8+i, x);
    this->HandleGeometry[i]->SetCenter(x);
    }

  // Update center handle
  double p0[3] = {0.,0.,0.};
  double p6[3] = {0.,0.,0.};
  this->Points->GetPoint(0, p0);
  this->Points->GetPoint(6, p6);
  double x[3];
  VTK_AVERAGE(p0,p6,x);
  this->Points->SetPoint(8+6, x);
  this->HandleGeometry[6]->SetCenter(x);

  this->Points->GetData()->Modified();
  this->Points->Modified();
  this->GenerateOutline();
}

#undef VTK_AVERAGE


//----------------------------------------------------------------------
void vtkAnnotationROIRepresentation2D::WidgetInteraction(double e[2])
{
  // Convert events to appropriate coordinate systems
  vtkCamera *camera = this->Renderer->IsActiveCameraCreated() ?
    this->Renderer->GetActiveCamera() : NULL;
  if ( !camera )
    {
    return;
    }

  // Get transform from 2D image to world
  vtkSmartPointer<vtkMatrix4x4> XYtoWorldMatrix = vtkSmartPointer<vtkMatrix4x4>::New();
  XYtoWorldMatrix->DeepCopy(this->GetWorldToDisplayTransform()->GetMatrix());
  XYtoWorldMatrix->Invert();



  // Compute the two points defining the motion vector
  double point2D[4] = {e[0],e[1], 0, 1};
  double pickPoint[4] = {0, 0, 0, 1};
  double prevPickPoint[4] = {0, 0, 0, 1};

  XYtoWorldMatrix->MultiplyPoint(point2D, pickPoint);
  XYtoWorldMatrix->MultiplyPoint(this->LastEventPosition, prevPickPoint);

  // Process the motion
  if ( this->InteractionState == vtkAnnotationROIRepresentation::MoveF0 )
    {
    this->MoveMinusXFace(prevPickPoint,pickPoint);
    }

  else if ( this->InteractionState == vtkAnnotationROIRepresentation::MoveF1 )
    {
    this->MovePlusXFace(prevPickPoint,pickPoint);
    }

  else if ( this->InteractionState == vtkAnnotationROIRepresentation::MoveF2 )
    {
    this->MoveMinusYFace(prevPickPoint,pickPoint);
    }

  else if ( this->InteractionState == vtkAnnotationROIRepresentation::MoveF3 )
    {
    this->MovePlusYFace(prevPickPoint,pickPoint);
    }

  else if ( this->InteractionState == vtkAnnotationROIRepresentation::MoveF4 )
    {
    this->MoveMinusZFace(prevPickPoint,pickPoint);
    }

  else if ( this->InteractionState == vtkAnnotationROIRepresentation::MoveF5 )
    {
    this->MovePlusZFace(prevPickPoint,pickPoint);
    }

  else if ( this->InteractionState == vtkAnnotationROIRepresentation::Translating )
    {
    this->Translate(prevPickPoint, pickPoint);
    }

  else if ( this->InteractionState == vtkAnnotationROIRepresentation::Scaling )
    {
    this->Scale(prevPickPoint, pickPoint, 
                static_cast<int>(e[0]), static_cast<int>(e[1]));
    }

  else if ( this->InteractionState == vtkAnnotationROIRepresentation::Rotating )
    {
    double vpn[3];
    camera->GetViewPlaneNormal(vpn);

    this->Rotate(static_cast<int>(e[0]), static_cast<int>(e[1]), prevPickPoint, pickPoint, vpn);
    }

  // Store the start position
  this->LastEventPosition[0] = e[0];
  this->LastEventPosition[1] = e[1];
  this->LastEventPosition[2] = 0.0;
  this->LastEventPosition[3] = 1.0;
}

//----------------------------------------------------------------------------
int vtkAnnotationROIRepresentation2D::ComputeInteractionState(int X, int Y, int vtkNotUsed(modify))
{
  // Okay, we can process this. Try to pick handles first;
  // if no handles picked, then pick the bounding box.
  if (!this->Renderer || !this->Renderer->IsInViewport(X, Y))
    {
    this->InteractionState = vtkAnnotationROIRepresentation::Outside;
    return this->InteractionState;
    }
  
  vtkAssemblyPath *path;
  // Try and pick a handle first
  this->LastPicker2D = NULL;
  this->CurrentHandle2D = NULL;
  this->HandlePicker2D->Pick(X,Y,0.0,this->Renderer);
  path = this->HandlePicker2D->GetPath();
  if ( path != NULL )
    {
    this->ValidPick = 1;
    this->LastPicker2D = this->HandlePicker2D;
    this->CurrentHandle2D =
           reinterpret_cast<vtkActor2D *>(path->GetFirstNode()->GetViewProp());
    if ( this->CurrentHandle2D == this->Handle2D[0] )
      {
      this->InteractionState = vtkAnnotationROIRepresentation::MoveF0;
      }
    else if ( this->CurrentHandle2D == this->Handle2D[1] )
      {
      this->InteractionState = vtkAnnotationROIRepresentation::MoveF1;
      }
    else if ( this->CurrentHandle2D == this->Handle2D[2] )
      {
      this->InteractionState = vtkAnnotationROIRepresentation::MoveF2;
      }
    else if ( this->CurrentHandle2D == this->Handle2D[3] )
      {
      this->InteractionState = vtkAnnotationROIRepresentation::MoveF3;
      }
    else if ( this->CurrentHandle2D == this->Handle2D[4] )
      {
      this->InteractionState = vtkAnnotationROIRepresentation::MoveF4;
      }
    else if ( this->CurrentHandle2D == this->Handle2D[5] )
      {
      this->InteractionState = vtkAnnotationROIRepresentation::MoveF5;
      }
    else if ( this->CurrentHandle2D == this->Handle2D[6] )
      {
      this->InteractionState = vtkAnnotationROIRepresentation::Translating;
      }
    }

  return this->InteractionState;
}

//----------------------------------------------------------------------------
double vtkAnnotationROIRepresentation2D
::ComputeHandleRadiusInWorldCoordinates(double radInPixels)
{
  // Get transform from 2D image to world
  vtkSmartPointer<vtkMatrix4x4> XYtoWorldMatrix = vtkSmartPointer<vtkMatrix4x4>::New();
  XYtoWorldMatrix->DeepCopy(this->GetWorldToDisplayTransform()->GetMatrix());
  XYtoWorldMatrix->Invert();
  double xyz0[4] = {0,0,0,1};
  double xyz1[4] = {radInPixels,radInPixels,0,1};
  double wxyz0[4] = {0,0,0,1};
  double wxyz1[4] = {0,0,0,1};

  XYtoWorldMatrix->MultiplyPoint(xyz0, wxyz0);
  XYtoWorldMatrix->MultiplyPoint(xyz1, wxyz1);

  double radius = 0;
  for (int i=0; i<3; i++)
    {
    radius += (wxyz1[i] - wxyz0[i])*(wxyz1[i] - wxyz0[i]);
    }
  return sqrt(radius/2);
}

//----------------------------------------------------------------------------
void vtkAnnotationROIRepresentation2D::PrintIntersections(ostream& os)
{
  os << "PrintIntersections:\n";
  for (int i=0; i<6; i++)
    {
    double *pts = static_cast<vtkDoubleArray *>(this->Points->GetData())->GetPointer(0);
    vtkIdType ncpts;
    vtkIdType *cpts;
    this->IntersectionFaces[i]->GetCellPoints(0, ncpts, cpts);
    os << "   Face[" << i << "]=(" << pts[3*cpts[0]] << ", " << pts[3*cpts[0]+1] << ", " << pts[3*cpts[0]+2] << 
                             "), (" << pts[3*cpts[1]] << ", " << pts[3*cpts[1]+1] << ", " << pts[3*cpts[1]+2] << 
                             "), (" << pts[3*cpts[2]] << ", " << pts[3*cpts[2]+1] << ", " << pts[3*cpts[2]+2] << 
                             "), (" << pts[3*cpts[3]] << ", " << pts[3*cpts[3]+1] << ", " << pts[3*cpts[3]+2] << ")\n"; 

    if (this->IntersectionCutters[i]->GetOutput()->GetNumberOfLines())
      {
      float *fpts = static_cast<vtkFloatArray *>(this->IntersectionCutters[i]->GetOutput()->GetPoints()->GetData())->GetPointer(0);
      this->IntersectionCutters[i]->GetOutput()->GetLines()->GetCell(0, ncpts, cpts);
      os << "   Cutter[" << i <<"]=(" << fpts[3*cpts[0]] << ", " << fpts[3*cpts[0]+1] << ", " << fpts[3*cpts[0]+2] << 
                               "), (" << fpts[3*cpts[1]] << ", " << fpts[3*cpts[1]+1] << ", " << fpts[3*cpts[1]+2] << ")\n"; 
      }
    else
      {
      os << "   Cutter[" << i <<"]=null\n";
      }
    }
}
