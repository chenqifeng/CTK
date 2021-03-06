/*=============================================================================

  Library: CTK

  Copyright (c) German Cancer Research Center,
    Division of Medical and Biological Informatics

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


#ifndef ctkDicomExchangeInterface_H
#define ctkDicomExchangeInterface_H

#include <QRect>
#include <QObject>
#include "ctkDicomAppHostingTypes.h"

struct QUuid;

struct ctkDicomExchangeInterface
{

  // Data exchange interface methods

  virtual bool notifyDataAvailable(const ctkDicomAppHosting::AvailableData& data, bool lastData) = 0;

  virtual QList<ctkDicomAppHosting::ObjectLocator> getData(
    const QList<QUuid>& objectUUIDs,
    const QList<QString>& acceptableTransferSyntaxUIDs,
    bool includeBulkData) = 0;

  virtual void releaseData(const QList<QUuid>& objectUUIDs) = 0;

//    8.3.3 getAsModels(objectUUIDs : ArrayOfUUID, classUID : UID, supportedInfosetTypes : ArrayOfMimeType) : ModelSetDescriptor	33
//    8.3.4 queryModel(models : ArrayOfUUID, xpaths : ArrayOfString) : ArrayOfQueryResult	34
//    8.3.5 queryInfoset(models : ArrayOfUUID, xpaths : ArrayOfString) : ArrayOfQueryResultInfoset	34
//    8.3.7 releaseModels(objectUUIDs : ArrayOfUUID): void

};

Q_DECLARE_INTERFACE(ctkDicomExchangeInterface, "org.commontk.dah.core.ExchangeInterface")

#endif // ctkDicomExchangeInterface_H
