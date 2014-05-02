/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkMRMLVolumeDisplayNode.h,v $
  Date:      $Date: 2006/03/19 17:12:29 $
  Version:   $Revision: 1.3 $

=========================================================================auto=*/

#ifndef __vtkMRMLVolumeDisplayNode_h
#define __vtkMRMLVolumeDisplayNode_h

// MRML includes
#include "vtkMRMLDisplayNode.h"
class vtkMRMLScene;
class vtkMRMLVolumeNode;

// VTK includes
class vtkAlgorithmOutput;
class vtkImageData;

/// \brief MRML node for representing a volume display attributes.
///
/// vtkMRMLVolumeDisplayNode nodes describe how volume is displayed.
class VTK_MRML_EXPORT vtkMRMLVolumeDisplayNode : public vtkMRMLDisplayNode
{
public:
  vtkTypeMacro(vtkMRMLVolumeDisplayNode,vtkMRMLDisplayNode);
  void PrintSelf(ostream& os, vtkIndent indent);

  ///
  /// Read node attributes from XML file
  virtual void ReadXMLAttributes( const char** atts);

  ///
  /// Write this node's information to a MRML file in XML format.
  virtual void WriteXML(ostream& of, int indent);

  ///
  /// Copy the node's attributes to this object
  virtual void Copy(vtkMRMLNode *node);

  ///
  /// Get node XML tag name (like Volume, Model)
  virtual const char* GetNodeTagName() = 0;

  ///
  /// Updates this node if it depends on other nodes
  /// when the node is deleted in the scene
  virtual void UpdateReferences();

  ///
  /// Finds the storage node and read the data
  virtual void UpdateScene(vtkMRMLScene *scene);

  ///
  /// Sets ImageData for background mask
  /// Must be reimplemented in deriving class if they need it.
  /// GetBackgroundImageDataConnection() returns 0 if the background image data
  /// is not used.
#if (VTK_MAJOR_VERSION <= 5)
  virtual void SetBackgroundImageData(vtkImageData* imageData);
#else
  virtual void SetBackgroundImageDataConnection(vtkAlgorithmOutput * imageDataConnection);
  virtual vtkAlgorithmOutput* GetBackgroundImageDataConnection();
#endif
  virtual vtkImageData* GetBackgroundImageData();

  /// Returns the output of the pipeline if there is a not a null input.
  /// Gets ImageData converted from the real data in the node
  /// The image is the direct output of the pipeline, it might not be
  /// up-to-date. You can call Update() on the returned vtkImageData or use
  /// GetUpToDateImageData() instead.
  /// \sa GetUpToDateImageData()
#if (VTK_MAJOR_VERSION <= 5)
  virtual vtkImageData* GetImageData();
#else
  virtual vtkAlgorithmOutput* GetImageDataConnection();
#endif

  /// Gets ImageData and ensure it's up-to-date by calling Update() on the
  /// pipeline.
  /// Please note that it can be slow, depending on the filters in
  /// the pipeline and the dimension of the input data.
#if (VTK_MAJOR_VERSION <= 5)
  vtkImageData* GetUpToDateImageData();
#endif

  /// Set the pipeline input.
  /// Filters can be applied to the input image data. The output image data
  /// is the one used by the mappers.
  /// It internally calls SetInputImageDataPipeline that can be reimplemented.
#if (VTK_MAJOR_VERSION <= 5)
  virtual void SetInputImageData(vtkImageData *imageData);
#else
  virtual void SetInputImageDataConnection(vtkAlgorithmOutput *imageDataConnection);
  virtual vtkAlgorithmOutput* GetInputImageDataConnection();
#endif

  /// Gets the pipeline input. To be reimplemented in subclasses.
  virtual vtkImageData* GetInputImageData();

  /// Gets the pipeline output. To be reimplemented in subclasses.
  virtual vtkImageData* GetOutputImageData();
#if (VTK_MAJOR_VERSION > 5)
  virtual vtkAlgorithmOutput* GetOutputImageDataConnection();
#endif

  ///
  /// Update the pipeline based on this node attributes
  virtual void UpdateImageDataPipeline();

  ///
  /// alternative method to propagate events generated in Display nodes
  virtual void ProcessMRMLEvents ( vtkObject * /*caller*/,
                                   unsigned long /*event*/,
                                   void * /*callData*/ );
  ///
  /// set gray colormap or override in subclass
  virtual void SetDefaultColorMap();

  /// Search in the scene the volume node vtkMRMLVolumeDisplayNode is associated
  /// to
  vtkMRMLVolumeNode* GetVolumeNode();

protected:
  vtkMRMLVolumeDisplayNode();
  ~vtkMRMLVolumeDisplayNode();
  vtkMRMLVolumeDisplayNode(const vtkMRMLVolumeDisplayNode&);
  void operator=(const vtkMRMLVolumeDisplayNode&);

#if (VTK_MAJOR_VERSION <= 5)
  virtual void SetInputToImageDataPipeline(vtkImageData *imageData);
#else
  virtual void SetInputToImageDataPipeline(vtkAlgorithmOutput *imageDataConnection);
#endif
};

#endif
