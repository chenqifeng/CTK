/*=========================================================================

  Library:   CTK

  Copyright (c) Kitware Inc.

  Licensed under the Apache License, Version 2.0 (the "License");
  you may not use this file except in compliance with the License.
  You may obtain a copy of the License at

      http://www.apache.org/licenses/LICENSE-2.0.txt

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

=========================================================================*/

// Qt includes
#include <QDebug>
#include <QMetaProperty>
#include <QSettings>
#include <QSignalMapper>

// CTK includes
#include "ctkSettingsPanel.h"
#include "ctkLogger.h"

static ctkLogger logger("org.commontk.libs.widgets.ctkSettingsPanel");

namespace
{

// --------------------------------------------------------------------------
struct PropertyType
{
  PropertyType();
  QObject* Object;
  QString  Property;
  QVariant PreviousValue;
  QVariant DefaultValue;
  QString  Label;
  ctkSettingsPanel::SettingOptions Options;

  QVariant value()const;
  bool setValue(const QVariant& value);

  QMetaProperty metaProperty();
};

// --------------------------------------------------------------------------
PropertyType::PropertyType()
  : Object(0)
  , Options(ctkSettingsPanel::OptionNone)
{
}

// --------------------------------------------------------------------------
QVariant PropertyType::value()const
{
  if (this->Object == 0 ||
      this->Property.isEmpty())
    {
    return QVariant();
    }
  return this->Object->property(this->Property.toLatin1());
}

// --------------------------------------------------------------------------
bool PropertyType::setValue(const QVariant& val)
{
  if (this->Object == 0 || this->Property.isEmpty())
    {
    Q_ASSERT(this->Object && !this->Property.isEmpty());
    return false;
    }
  QVariant value(val);
  // HACK - See http://bugreports.qt.nokia.com/browse/QTBUG-19823
  if (qstrcmp(this->metaProperty().typeName(), "QStringList") == 0 && !value.isValid())
    {
    value = QVariant(QStringList());
    }
  bool success = this->Object->setProperty(this->Property.toLatin1(), value);
  Q_ASSERT(success);
  return success;
}

// --------------------------------------------------------------------------
QMetaProperty PropertyType::metaProperty()
{
  Q_ASSERT(this->Object);
  for(int i=0; i < this->Object->metaObject()->propertyCount(); ++i)
    {
    this->Object->metaObject()->property(i);
    if (this->Object->metaObject()->property(i).name() == this->Property)
      {
      return this->Object->metaObject()->property(i);
      }
    }
  return QMetaProperty();
}

} // end of anonymous namespace

//-----------------------------------------------------------------------------
class ctkSettingsPanelPrivate
{
  Q_DECLARE_PUBLIC(ctkSettingsPanel);
protected:
  ctkSettingsPanel* const q_ptr;

public:
  ctkSettingsPanelPrivate(ctkSettingsPanel& object);
  void init();

  QSettings*                  Settings;
  QMap<QString, PropertyType> Properties;
  bool                        SaveToSettingsWhenRegister;
};

// --------------------------------------------------------------------------
ctkSettingsPanelPrivate::ctkSettingsPanelPrivate(ctkSettingsPanel& object)
  :q_ptr(&object)
{
  qRegisterMetaType<ctkSettingsPanel::SettingOption>("ctkSettingsPanel::SettingOption");
  qRegisterMetaType<ctkSettingsPanel::SettingOptions>("ctkSettingsPanel::SettingOptions");
  this->Settings = 0;
  this->SaveToSettingsWhenRegister = true;
}

// --------------------------------------------------------------------------
void ctkSettingsPanelPrivate::init()
{
}

// --------------------------------------------------------------------------
ctkSettingsPanel::ctkSettingsPanel(QWidget* _parent)
  : Superclass(_parent)
  , d_ptr(new ctkSettingsPanelPrivate(*this))
{
  Q_D(ctkSettingsPanel);
  d->init();
}

// --------------------------------------------------------------------------
ctkSettingsPanel::~ctkSettingsPanel()
{
  this->applySettings();
}

// --------------------------------------------------------------------------
QSettings* ctkSettingsPanel::settings()const
{
  Q_D(const ctkSettingsPanel);
  return d->Settings;
}

// --------------------------------------------------------------------------
void ctkSettingsPanel::setSettings(QSettings* settings)
{
  Q_D(ctkSettingsPanel);
  if (d->Settings == settings)
    {
    return;
    }
  d->Settings = settings;
  this->updateProperties();
}

// --------------------------------------------------------------------------
void ctkSettingsPanel::updateProperties()
{
  Q_D(ctkSettingsPanel);
  if (!d->Settings)
    {
    return;
    }
  foreach(const QString& key, d->Properties.keys())
    {
    if (d->Settings->contains(key))
      {
      QVariant value = d->Settings->value(key);
      PropertyType& prop = d->Properties[key];
      // Update object registered using registerProperty()
      prop.setValue(value);
      prop.PreviousValue = value;
      }
    else
      {
      this->updateSetting(key);
      }
    }
}

// --------------------------------------------------------------------------
void ctkSettingsPanel::updateSetting(const QString& key)
{
  Q_D(ctkSettingsPanel);
  if (!d->Settings)
    {
    return;
    }
  this->setSetting(key, d->Properties[key].value());
}

// --------------------------------------------------------------------------
void ctkSettingsPanel::setSetting(const QString& key, const QVariant& newVal)
{
  Q_D(ctkSettingsPanel);
  if (!d->Settings)
    {
    return;
    }
  QVariant oldVal = d->Settings->value(key);
  d->Settings->setValue(key, newVal);
  d->Properties[key].setValue(newVal);
  if (d->Settings->status() != QSettings::NoError)
    {
    logger.warn( QString("Error #%1 while writing setting \"%2\"")
      .arg(static_cast<int>(d->Settings->status()))
      .arg(key));
    }
  if (oldVal != newVal)
    {
    emit settingChanged(key, newVal);
    }
}

// --------------------------------------------------------------------------
void ctkSettingsPanel::registerProperty(const QString& key,
                                        QObject* object,
                                        const QString& property,
                                        const char* signal,
                                        const QString& label,
                                        ctkSettingsPanel::SettingOptions options)
{
  Q_D(ctkSettingsPanel);
  PropertyType prop;
  prop.Object = object;
  prop.Property = property;
  prop.DefaultValue = prop.PreviousValue = prop.value();
  prop.Label = label;
  prop.Options = options;

  if (d->Settings && d->Settings->contains(key))
    {
    QVariant val = d->Settings->value(key);
    prop.setValue(val);
    prop.PreviousValue = val;
    }
  d->Properties[key] = prop;

  // Create a signal mapper per property to be able to support
  // multiple signals from the same sender.
  QSignalMapper* signalMapper = new QSignalMapper(this);
  QObject::connect(signalMapper, SIGNAL(mapped(QString)),
                   this, SLOT(updateSetting(QString)));
  signalMapper->setMapping(object, key);
  this->connect(object, signal, signalMapper, SLOT(map()));

  if (d->SaveToSettingsWhenRegister)
    {
    this->updateSetting(key);
    }
}

// --------------------------------------------------------------------------
QVariant ctkSettingsPanel::defaultPropertyValue(const QString& key) const
{
  Q_D(const ctkSettingsPanel);
  if (!d->Properties.contains(key))
    {
    return QVariant();
    }
  return d->Properties.value(key).DefaultValue;
}

// --------------------------------------------------------------------------
QVariant ctkSettingsPanel::previousPropertyValue(const QString& key) const
{
  Q_D(const ctkSettingsPanel);
  if (!d->Properties.contains(key))
    {
    return QVariant();
    }
  return d->Properties.value(key).PreviousValue;
}

// --------------------------------------------------------------------------
QVariant ctkSettingsPanel::propertyValue(const QString& key) const
{
  Q_D(const ctkSettingsPanel);
  if (!d->Properties.contains(key))
    {
    return QVariant();
    }
  return d->Properties.value(key).value();
}

// --------------------------------------------------------------------------
QStringList ctkSettingsPanel::changedSettings()const
{
  Q_D(const ctkSettingsPanel);
  QStringList settingsKeys;
  foreach(const QString& key, d->Properties.keys())
    {
    const PropertyType& prop = d->Properties[key];
    if (prop.PreviousValue != prop.value())
      {
      settingsKeys << key;
      }
    }
  return settingsKeys;
}

// --------------------------------------------------------------------------
QString ctkSettingsPanel::settingLabel(const QString& settingKey)const
{
  Q_D(const ctkSettingsPanel);
  return d->Properties[settingKey].Label;
}

// --------------------------------------------------------------------------
ctkSettingsPanel::SettingOptions ctkSettingsPanel
::settingOptions(const QString& settingKey)const
{
  Q_D(const ctkSettingsPanel);
  return d->Properties[settingKey].Options;
}

// --------------------------------------------------------------------------
void ctkSettingsPanel::applySettings()
{
  Q_D(ctkSettingsPanel);
  foreach(const QString& key, d->Properties.keys())
    {
    PropertyType& prop = d->Properties[key];
    prop.PreviousValue = prop.value();
    }
}

// --------------------------------------------------------------------------
void ctkSettingsPanel::resetSettings()
{
  Q_D(ctkSettingsPanel);
  foreach(const QString& key, d->Properties.keys())
    {
    this->setSetting(key, d->Properties[key].PreviousValue);
    }
}

// --------------------------------------------------------------------------
void ctkSettingsPanel::restoreDefaultSettings()
{
  Q_D(ctkSettingsPanel);
  foreach(const QString& key, d->Properties.keys())
    {
    this->setSetting(key, d->Properties[key].DefaultValue);
    }
}
