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

// Volumes logic
#include "vtkSlicerVolumesLogic.h"

// MRML includes
#include <vtkMRMLScalarVolumeNode.h>
#include <vtkMRMLScene.h>

// VTK includes
#include <vtkAlgorithm.h>
#include <vtkAlgorithmOutput.h>
#include <vtkDataSetAttributes.h>
#include <vtkImageData.h>
#include <vtkImageAlgorithm.h>
#include <vtkInformation.h>
#include <vtkNew.h>
#include <vtkTrivialProducer.h>

// ITK includes
#include <itkConfigure.h>
#include <itkFactoryRegistration.h>

//-----------------------------------------------------------------------------
#if VTK_MAJOR_VERSION <= 5
bool isImageDataValid(vtkImageData* imageData)
{
  if (!imageData)
    {
    return false;
    }
  imageData->GetProducerPort();
  vtkInformation* info = imageData->GetPipelineInformation();
  info = imageData->GetPipelineInformation();
#else
bool isImageDataValid(vtkAlgorithmOutput* imageDataConnection)
{
  if (!imageDataConnection ||
      !imageDataConnection->GetProducer())
    {
    std::cout << "No image data port" << std::endl;
    return false;
    }
  imageDataConnection->GetProducer()->Update();
  vtkInformation* info =
    imageDataConnection->GetProducer()->GetOutputInformation(0);
#endif
  if (!info)
    {
    std::cout << "No output information" << std::endl;
    return false;
    }
  vtkInformation *scalarInfo = vtkDataObject::GetActiveFieldInformation(info,
    vtkDataObject::FIELD_ASSOCIATION_POINTS, vtkDataSetAttributes::SCALARS);
  if (!scalarInfo)
    {
    std::cout << "No scalar information" << std::endl;
    return false;
    }
  return true;
}

//-----------------------------------------------------------------------------
int vtkSlicerVolumesLogicTest1( int argc, char * argv[] )
{
  itk::itkFactoryRegistration();

  vtkNew<vtkMRMLScene> scene;
  vtkNew<vtkSlicerVolumesLogic> logic;

  if (argc < 2)
    {
    std::cerr << "Usage: vtkSlicerVolumesLogicTest1 volumeName [-I]" << std::endl;
    return EXIT_FAILURE;
    }

  logic->SetMRMLScene(scene.GetPointer());

  vtkMRMLVolumeNode* volume =
    logic->AddArchetypeVolume(argv[1], "volume", 0);

  if (!volume ||
#if VTK_MAJOR_VERSION <=5
      !isImageDataValid(volume->GetImageData()))
#else
      !isImageDataValid(volume->GetImageDataConnection()))
#endif
    {
    std::cerr << "Failed to load scalar image." << std::endl;
    return EXIT_FAILURE;
    }

  if (volume && volume->GetImageData())
    {
    volume->GetImageData()->Print(std::cerr);
    }

  return EXIT_SUCCESS;
}
