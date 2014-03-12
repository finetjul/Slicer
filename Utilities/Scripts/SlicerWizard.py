#!/usr/bin/env python

import argparse
import fnmatch
import os
import re
import sys

sourcePatterns = [
  "*.h",
  "*.cxx",
  "*.cpp",
  "CMakeLists.txt",
  "*.cmake",
  "*.ui",
  "*.qrc",
  "*.py",
  "*.xml",
  "*.xml.in",
  "*.md5",
  "*.png",
  "*.dox",
]

argValueFormats = {
  "addModule": "TYPE:NAME",
  "createExtension": "[TYPE:]NAME",
  "templateKey": "TYPE=KEY",
  "templatePath": "[CATEGORY=]PATH",
}

templateCategories = [
  "extensions",
  "modules",
]

templatePaths = {}
templateKeys = {}

reModuleInsertPlaceholder = re.compile("(?<=\n)([ \t]*)## NEXT_MODULE")
reAddSubdirectory = \
  re.compile("(?<=\n)([ \t]*)add_subdirectory[(][^)]+[)][^\n]*\n")

#-----------------------------------------------------------------------------
def isSourceFile(name):
  for pat in sourcePatterns:
    if fnmatch.fnmatch(name, pat):
      return True

  return False

#-----------------------------------------------------------------------------
def isTemplateCategory(name, relPath):
  if not os.path.isdir(os.path.join(relPath, name)):
    return False

  name = name.lower()
  return name in templateCategories

#-----------------------------------------------------------------------------
def listSources(directory):
  for root, subFolders, files in os.walk(directory):
    for f in files:
      if isSourceFile(f):
        f = os.path.join(root, f)
        yield f[len(directory) + 1:] # strip common dir

#-----------------------------------------------------------------------------
def copyAndReplace(inFile, template, destination, key, name):
  outFile = os.path.join(destination, inFile.replace(key, name))
  print("creating '%s'" % outFile)
  path = os.path.dirname(outFile)
  if not os.path.exists(path):
    os.makedirs(path)

  with open(os.path.join(template, inFile)) as fp:
    contents = fp.read()
  contents = contents.replace(key, name)
  contents = contents.replace(key.upper(), name.upper())
  with open(outFile, "w") as fp:
    fp.write(contents)

#-----------------------------------------------------------------------------
def copyTemplate(args, category, kind, name):
  templates = templatePaths[category]
  if not kind.lower() in templates:
    print("'%s' is not a known extension template" % kind)
    exit()
  kind = kind.lower()

  destination = os.path.join(args.destination, name)
  if os.path.exists(destination):
    print("create %s: refusing to overwrite existing directory '%s'" \
          % (category, destination))
    exit()

  template = templates[kind]
  if kind in templateKeys:
    key = templateKeys[kind]
  else:
    key = "TemplateKey"

  print("copy template '%s' to '%s', replacing '%s' -> '%s'" %
        (template, destination, key, name))
  for f in listSources(template):
    copyAndReplace(f, template, destination, key, name)

  return destination

#-----------------------------------------------------------------------------
def addTemplateCategoryPaths(category, path):
  if not category in templatePaths:
    templatePaths[category] = {}

  for entry in os.listdir(path):
    entryPath = os.path.join(path, entry)
    if os.path.isdir(entryPath):
      templatePaths[category][entry.lower()] = entryPath

#-----------------------------------------------------------------------------
def addTemplatePaths(basePath):
  if not os.path.exists(basePath):
    return

  basePath = os.path.realpath(basePath)

  for entry in os.listdir(basePath):
    if isTemplateCategory(entry, basePath):
      addTemplateCategoryPaths(entry.lower(), os.path.join(basePath, entry))

#-----------------------------------------------------------------------------
def addModuleToScript(name, contents):
  contents = "\n" + contents
  pat = "%sadd_subdirectory(%s)\n"

  # Try to insert before placeholder
  m = reModuleInsertPlaceholder.search(contents)
  if m is not None:
    return contents[1:m.start()] + \
           pat % (m.group(1), name) + \
           contents[m.start():]

  # No? Try to insert after last add_subdirectory
  for m in reAddSubdirectory.finditer(contents):
    pass

  if m is not None:
    print m.groups(), m.start(), m.end()
    return contents[1:m.end()] + \
           pat % (m.group(1), name) + \
           contents[m.end():]

  # Still no? Oh, dear...
  print("failed to find insertion point for module"
        " in parent CMakeLists.txt")
  exit()

#-----------------------------------------------------------------------------
def addModuleToProject(path, name):
  cmakeFile = os.path.join(path, "CMakeLists.txt")
  if not os.path.exists(cmakeFile):
    print("failed to add module to project '%s': no CMakeLists.txt found" %
          path)
    exit()

  with open(cmakeFile) as fp:
    contents = fp.read()
  with open(cmakeFile, "w") as fp:
    fp.write(addModuleToScript(name, contents))

#-----------------------------------------------------------------------------
def createExtension(args, name, kind="default"):
  args.destination = copyTemplate(args, "extensions", kind, name)
  print("created extension '%s'" % name)

#-----------------------------------------------------------------------------
def addModule(args, kind, name):
  addModuleToProject(args.destination, name)
  copyTemplate(args, "modules", kind, name)
  print("created module '%s'" % name)

#=============================================================================
class TemporaryBool(object):
  #---------------------------------------------------------------------------
  def __init__(self, obj, attr, value):
    self._obj = obj
    self._attr = attr
    self._oldValue = bool(getattr(obj, attr))
    self._newValue = value
  #---------------------------------------------------------------------------
  def __enter__(self):
    setattr(self._obj, self._attr, self._newValue)
  #---------------------------------------------------------------------------
  def __exit__(self, exc_type, exc_value, traceback):
    setattr(self._obj, self._attr, self._oldValue)

#=============================================================================
class WizardHelpFormatter(argparse.HelpFormatter):
  #---------------------------------------------------------------------------
  def __init__(self, *args, **kwargs):
    super(WizardHelpFormatter, self).__init__(*args, **kwargs)
    self._splitWorkaround = False

  #---------------------------------------------------------------------------
  def _metavar_formatter(self, action, default_metavar):
    if action.dest in argValueFormats:
      default_metavar = argValueFormats[action.dest]
      if self._splitWorkaround:
        default_metavar = default_metavar.replace("[", "<").replace("]", ">")

    return super(WizardHelpFormatter, self)._metavar_formatter(action,
                                                               default_metavar)

  #---------------------------------------------------------------------------
  def _format_usage(self, usage, actions, groups, prefix):
    with TemporaryBool(self, "_splitWorkaround", True):
      text = super(WizardHelpFormatter, self)._format_usage(usage, actions,
                                                            groups, prefix)

    return text.replace("<", "[").replace(">", "]")

#-----------------------------------------------------------------------------
def main():

  # Set up arguments
  parser = argparse.ArgumentParser(description="Slicer Wizard",
                                   formatter_class=WizardHelpFormatter)
  parser.add_argument("--addModule", action="append",
                      help="add new TYPE module NAME to an existing project"
                           " in the destination directory;"
                           " may use more than once")
  parser.add_argument("--createExtension",
                      help="create extension NAME"
                           " under the destination directory;"
                           " any modules are added to the new extension"
                           " (default type: 'default')")
  parser.add_argument("--templatePath", action="append",
                      help="add additional template path for specified"
                           " template category; if no category, expect that"
                           " PATH contains subdirectories for one or more"
                           " possible categories")
  parser.add_argument("--templateKey", action="append",
                      help="set template substitution key for specified"
                           " template (default key: 'TemplateKey')")
  parser.add_argument("destination", default=os.getcwd(), nargs="?",
                      help="location of output files (default: '.')")
  args = parser.parse_args()

  # Add built-in templates
  scriptPath = os.path.dirname(os.path.realpath(__file__))
  addTemplatePaths(os.path.join(scriptPath, "..", "Templates"))

  # Add user-specified template paths
  if args.templatePath is not None:
    for tp in args.templatePath:
      tpParts = tp.split("=", 1)

      if len(tpParts) == 1:
        if not os.path.exists(tp):
          print("template path '%s' does not exist" % tp)
          exit()
        if not os.path.isdir(tp):
          print("template path '%s' is not a directory" % tp)
          exit()

        addTemplatePaths(tp)

      else:
        if tpParts[0].lower() not in templateCategories:
          print("'%s' is not a recognized template category" % tpParts[0])
          print("recognized categories: %s" % ", ".join(templateCategories))
          exit()

        if not os.path.exists(tpParts[1]):
          print("template path '%s' does not exist" % tpParts[1])
          exit()
        if not os.path.isdir(tpParts[1]):
          print("template path '%s' is not a directory" % tpParts[1])
          exit()

        addTemplateCategoryPaths(tpParts[0].lower(),
                                 os.path.realpath(tpParts[1]))

  # Set user-specified template keys
  if args.templateKey is not None:
    for tk in args.templateKey:
      tkParts = tk.split("=")
      if len(tkParts) != 2:
        print("template key '%s' malformatted: expected 'NAME=KEY'" % tk)
        exit()
      templateKeys[tkParts[0]] = tkParts[1]

  # Check that we have something to do
  if args.createExtension is None and args.addModule is None:
    print("no action was requested!")
    exit()

  # Create requested extensions
  if args.createExtension is not None:
    extArgs = args.createExtension.split(":")
    extArgs.reverse()
    createExtension(args, *extArgs)

  # Create requested modules
  if args.addModule is not None:
    for module in args.addModule:
      addModule(args, *module.split(":"))

#=============================================================================

if __name__ == "__main__":
  main()
