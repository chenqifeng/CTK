/*=============================================================================

  Library: CTK

  Copyright (c) University College London

  Licensed under the Apache License, Version 2.0 (the "License");
  you may not use this file except in compliance with the License.
  You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

=============================================================================*/

#include "ctkCmdLineModuleDefaultPathBuilder.h"
#include <QDir>
#include <QDebug>
#include <QCoreApplication>
#include <cstdlib>

//----------------------------------------------------------------------------
struct ctkCmdLineModuleDefaultPathBuilderPrivate
{
public:
  ctkCmdLineModuleDefaultPathBuilderPrivate();
  ~ctkCmdLineModuleDefaultPathBuilderPrivate();
  QStringList build() const;

  void setLoadFromHomeDir(bool doLoad);
  void setLoadFromCurrentDir(bool doLoad);
  void setLoadFromApplicationDir(bool doLoad);
  void setLoadFromCtkModuleLoadPath(bool doLoad);

  bool LoadFromHomeDir;
  bool LoadFromCurrentDir;
  bool LoadFromApplicationDir;
  bool LoadFromCtkModuleLoadPath;

};

//-----------------------------------------------------------------------------
// ctkCmdLineModuleDefaultPathBuilderPrivate methods

//-----------------------------------------------------------------------------
ctkCmdLineModuleDefaultPathBuilderPrivate::ctkCmdLineModuleDefaultPathBuilderPrivate()
: LoadFromHomeDir(false)
, LoadFromCurrentDir(false)
, LoadFromApplicationDir(false)
, LoadFromCtkModuleLoadPath(false)
{

}

//-----------------------------------------------------------------------------
ctkCmdLineModuleDefaultPathBuilderPrivate::~ctkCmdLineModuleDefaultPathBuilderPrivate()
{

}

//-----------------------------------------------------------------------------
void ctkCmdLineModuleDefaultPathBuilderPrivate::setLoadFromHomeDir(bool doLoad)
{
  LoadFromHomeDir = doLoad;
}

//-----------------------------------------------------------------------------
void ctkCmdLineModuleDefaultPathBuilderPrivate::setLoadFromCurrentDir(bool doLoad)
{
  LoadFromCurrentDir = doLoad;
}

//-----------------------------------------------------------------------------
void ctkCmdLineModuleDefaultPathBuilderPrivate::setLoadFromApplicationDir(bool doLoad)
{
  LoadFromApplicationDir = doLoad;
}

//-----------------------------------------------------------------------------
void ctkCmdLineModuleDefaultPathBuilderPrivate::setLoadFromCtkModuleLoadPath(bool doLoad)
{
  LoadFromCtkModuleLoadPath = doLoad;
}


//-----------------------------------------------------------------------------
QStringList ctkCmdLineModuleDefaultPathBuilderPrivate::build() const
{
  QStringList result;

  QString suffix = "cli-modules";

  if (LoadFromCtkModuleLoadPath)
  {
    char *ctkModuleLoadPath = getenv("CTK_MODULE_LOAD_PATH");
    if (ctkModuleLoadPath != NULL)
    {
      QDir dir = QDir(QString(ctkModuleLoadPath));
      if (dir.exists())
      {
        result << dir.canonicalPath();
      }
    }
  }

  if (LoadFromHomeDir)
  {
    if (QDir::home().exists())
    {
      result << QDir::homePath();
      result << QDir::homePath() + QDir::separator() + suffix;
    }
  }

  if (LoadFromCurrentDir)
  {
    if (QDir::current().exists())
    {
      result << QDir::currentPath();
      result << QDir::currentPath() + QDir::separator() + suffix;
    }
  }

  if (LoadFromApplicationDir)
  {
    result << QCoreApplication::applicationDirPath();
    result << QCoreApplication::applicationDirPath() + QDir::separator() + suffix;
  }

  return result;
}

//-----------------------------------------------------------------------------
// ctkCmdLineModuleDefaultPathBuilder methods

//-----------------------------------------------------------------------------
ctkCmdLineModuleDefaultPathBuilder::ctkCmdLineModuleDefaultPathBuilder()
  : d(new ctkCmdLineModuleDefaultPathBuilderPrivate)
{
}

//-----------------------------------------------------------------------------
ctkCmdLineModuleDefaultPathBuilder::~ctkCmdLineModuleDefaultPathBuilder()
{
}

//-----------------------------------------------------------------------------
QStringList ctkCmdLineModuleDefaultPathBuilder::build() const
{
  return d->build();
}

//-----------------------------------------------------------------------------
void ctkCmdLineModuleDefaultPathBuilder::setLoadFromHomeDir(bool doLoad)
{
  d->setLoadFromHomeDir(doLoad);
}

//-----------------------------------------------------------------------------
void ctkCmdLineModuleDefaultPathBuilder::setLoadFromCurrentDir(bool doLoad)
{
  d->setLoadFromCurrentDir(doLoad);
}

//-----------------------------------------------------------------------------
void ctkCmdLineModuleDefaultPathBuilder::setLoadFromApplicationDir(bool doLoad)
{
  d->setLoadFromApplicationDir(doLoad);
}

//-----------------------------------------------------------------------------
void ctkCmdLineModuleDefaultPathBuilder::setLoadFromCtkModuleLoadPath(bool doLoad)
{
  d->setLoadFromCtkModuleLoadPath(doLoad);
}
