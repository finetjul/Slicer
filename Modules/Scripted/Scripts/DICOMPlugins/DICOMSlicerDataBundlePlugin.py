import os, zipfile, tempfile
from __main__ import vtk, qt, ctk, slicer
from DICOMLib import DICOMPlugin
from DICOMLib import DICOMLoadable

#
# This is the plugin to handle translation of encapsulated MRML
# scenes from DICOM back into slicer.
# It follows the DICOM module's plugin architecture.
#

class DICOMSlicerDataBundlePluginClass(DICOMPlugin):
  """ SlicerDataBundle specific interpretation code
  """

  def __init__(self):
    super(DICOMSlicerDataBundlePluginClass,self).__init__()
    self.loadType = "Slicer Data Bundle"
    self.tags['seriesDescription'] = "0008,103e"
    self.tags['candygram'] = "cadb,0010"
    self.tags['zipSize'] = "cadb,1008"
    self.tags['zipData'] = "cadb,1010"

  def examine(self,fileLists):
    """ Returns a list of DICOMLoadable instances
    corresponding to ways of interpreting the
    fileLists parameter.
    """
    loadables = []
    for files in fileLists:
      cachedLoadables = self.getCachedLoadables(files)
      if cachedLoadables:
        loadables += cachedLoadables
      else:
        loadablesForFiles = self.examineFiles(files)
        loadables += loadablesForFiles
        self.cacheLoadables(files,loadablesForFiles)
    return loadables

  def examineFiles(self,files):
    """ Returns a list of DICOMLoadable instances
    corresponding to ways of interpreting the
    files parameter.
    Look for the special private creator tags that indicate
    a slicer data bundle
    Note that each data bundle is in a unique series, so
    if 'files' is a list of more than one element, then this
    is not a data bundle.
    """

    loadables = []
    if len(files) == 1:
      f = files[0]
      # get the series description to use as base for volume name
      name = slicer.dicomDatabase.fileValue(f, self.tags['seriesDescription'])
      if name == "":
        name = "Unknown"
      candygramValue = slicer.dicomDatabase.fileValue(f, self.tags['candygram'])
      if candygramValue:
        # default loadable includes all files for series
        loadable = DICOMLib.DICOMLoadable()
        loadable.files = [f]
        loadable.name = name + ' - as Slicer Scene'
        loadable.selected = True
        loadable.tooltip = 'Appears to contain a slicer scene'
        loadables.append(loadable)
    return loadables

  def load(self,loadable):
    """Load the selection as a diffusion volume
    using the dicom to nrrd converter module
    """

    f = loadable.files[0]
    try:
      zipSize = int(slicer.dicomDatabase.fileValue(f, self.tags['zipSize']))
    except ValueError:
      return False

    print('importing', f)
    print('size', zipSize)

    sceneDir = tempfile.mkdtemp('', 'sceneImport', slicer.app.temporaryPath)
    fp = open(f, 'rb')
    fp.seek(-1 * (1+zipSize), os.SEEK_END)
    zipData = fp.read(zipSize)
    fp.close()

    zipPath = os.path.join(sceneDir,'scene.zip')
    fp = open(zipPath,'wb')
    fp.write(zipData)
    fp.close()

    appLogic = slicer.app.applicationLogic()
    sceneFile = appLogic.OpenSlicerDataBundle(zipPath, sceneDir)
    print ("loaded %s" % sceneFile)

    return sceneFile != ""

#
# DICOMSlicerDataBundlePlugin
#

class DICOMSlicerDataBundlePlugin:
  """
  This class is the 'hook' for slicer to detect and recognize the plugin
  as a loadable scripted module
  """
  def __init__(self, parent):
    parent.title = "DICOM Diffusion Volume Plugin"
    parent.categories = ["Developer Tools.DICOM Plugins"]
    parent.contributors = ["Steve Pieper (Isomics Inc.)"]
    parent.helpText = """
    Plugin to the DICOM Module to parse and load diffusion volumes
    from DICOM files.
    No module interface here, only in the DICOM module
    """
    parent.acknowledgementText = """
    This DICOM Plugin was developed by
    Steve Pieper, Isomics, Inc.
    and was partially funded by NIH grant 3P41RR013218.
    """

    # don't show this module - it only appears in the DICOM module
    parent.hidden = True

    # Add this extension to the DICOM module's list for discovery when the module
    # is created.  Since this module may be discovered before DICOM itself,
    # create the list if it doesn't already exist.
    try:
      slicer.modules.dicomPlugins
    except AttributeError:
      slicer.modules.dicomPlugins = {}
    slicer.modules.dicomPlugins['DICOMSlicerDataBundlePlugin'] = DICOMSlicerDataBundlePluginClass

#
# DICOMSlicerDataBundleWidget
#

class DICOMSlicerDataBundleWidget:
  def __init__(self, parent = None):
    self.parent = parent

  def setup(self):
    # don't display anything for this widget - it will be hidden anyway
    pass

  def enter(self):
    pass

  def exit(self):
    pass


