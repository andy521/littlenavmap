/*****************************************************************************
* Copyright 2015-2016 Alexander Barthel albar965@mailbox.org
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*****************************************************************************/

#include "options/optionsdialog.h"

#include "common/constants.h"
#include "gui/mainwindow.h"
#include "ui_options.h"
#include "common/weatherreporter.h"
#include "gui/widgetstate.h"

#include <QMessageBox>
#include <QRegularExpression>
#include <QRegularExpressionValidator>

const int MAX_RANGE_RING_SIZE = 4000;
const int MAX_RANGE_RINGS = 10;

class RangeRingValidator :
  public QValidator
{
public:
  RangeRingValidator();

private:
  bool ringStrToVector(const QString& str) const;
  virtual QValidator::State validate(QString& input, int& pos) const override;

  QRegularExpressionValidator regexpVal;
};

RangeRingValidator::RangeRingValidator()
{
  regexpVal.setRegularExpression(QRegularExpression("^([1-9]\\d{0,3} )*[1-9]\\d{1,3}$"));
}

QValidator::State RangeRingValidator::validate(QString& input, int& pos) const
{
  State state = regexpVal.validate(input, pos);
  if(state == Invalid)
    return Invalid;
  else if(!ringStrToVector(input))
    return Invalid;

  return state;
}

bool RangeRingValidator::ringStrToVector(const QString& input) const
{
  int numRing = 0;
  for(const QString& str : input.split(" "))
  {
    QString val = str.trimmed();
    if(!val.isEmpty())
    {
      bool ok;
      int num = val.toInt(&ok);
      if(!ok || num > MAX_RANGE_RING_SIZE)
        return false;

      numRing++;
    }
  }
  return numRing <= MAX_RANGE_RINGS;
}

// ------------------------------------------------------------------------

OptionsDialog::OptionsDialog(MainWindow *parentWindow)
  : QDialog(parentWindow), ui(new Ui::Options), mainWindow(parentWindow)
{
  ui->setupUi(this);

  rangeRingValidator = new RangeRingValidator;

  widgets.append(ui->tabWidgetOptions);
  widgets.append(ui->checkBoxOptionsGuiCenterKml);
  widgets.append(ui->checkBoxOptionsGuiCenterRoute);
  widgets.append(ui->checkBoxOptionsMapEmptyAirports);
  widgets.append(ui->checkBoxOptionsRouteEastWestRule);
  widgets.append(ui->checkBoxOptionsRoutePreferNdb);
  widgets.append(ui->checkBoxOptionsRoutePreferVor);
  widgets.append(ui->checkBoxOptionsStartupLoadKml);
  widgets.append(ui->checkBoxOptionsStartupLoadMapSettings);
  widgets.append(ui->checkBoxOptionsStartupLoadRoute);
  widgets.append(ui->checkBoxOptionsWeatherInfoAsn);
  widgets.append(ui->checkBoxOptionsWeatherInfoNoaa);
  widgets.append(ui->checkBoxOptionsWeatherInfoVatsim);
  widgets.append(ui->checkBoxOptionsWeatherTooltipAsn);
  widgets.append(ui->checkBoxOptionsWeatherTooltipNoaa);
  widgets.append(ui->checkBoxOptionsWeatherTooltipVatsim);
  widgets.append(ui->lineEditOptionsMapRangeRings);
  widgets.append(ui->lineEditOptionsWeatherAsnPath);
  widgets.append(ui->listWidgetOptionsDatabaseAddon);
  widgets.append(ui->listWidgetOptionsDatabaseExclude);
  widgets.append(ui->radioButtonOptionsMapScrollFull);
  widgets.append(ui->radioButtonOptionsMapScrollNone);
  widgets.append(ui->radioButtonOptionsMapScrollNormal);
  widgets.append(ui->radioButtonOptionsMapSimUpdateFast);
  widgets.append(ui->radioButtonOptionsMapSimUpdateLow);
  widgets.append(ui->radioButtonOptionsMapSimUpdateMedium);
  widgets.append(ui->radioButtonOptionsStartupShowHome);
  widgets.append(ui->radioButtonOptionsStartupShowLast);
  widgets.append(ui->spinBoxOptionsCacheDiskSize);
  widgets.append(ui->spinBoxOptionsCacheMemorySize);
  widgets.append(ui->spinBoxOptionsGuiInfoText);
  widgets.append(ui->spinBoxOptionsGuiRouteText);
  widgets.append(ui->spinBoxOptionsGuiSearchText);
  widgets.append(ui->spinBoxOptionsGuiSimInfoText);
  widgets.append(ui->spinBoxOptionsMapClickRect);
  widgets.append(ui->spinBoxOptionsMapSymbolSize);
  widgets.append(ui->spinBoxOptionsMapTextSize);
  widgets.append(ui->spinBoxOptionsMapTooltipRect);
  widgets.append(ui->doubleSpinBoxOptionsMapZoomShowMap);
  widgets.append(ui->spinBoxOptionsRouteGroundBuffer);

  ui->lineEditOptionsMapRangeRings->setValidator(rangeRingValidator);

  connect(ui->buttonBoxOptions, &QDialogButtonBox::clicked, this, &OptionsDialog::buttonBoxClicked);

  connect(ui->pushButtonOptionsStartupResetDefault, &QPushButton::clicked,
          this, &OptionsDialog::resetDefaultClicked);
  connect(ui->pushButtonOptionsWeatherAsnPathSelect, &QPushButton::clicked,
          this, &OptionsDialog::selectAsnPathClicked);

  connect(ui->pushButtonOptionsCacheClearDisk, &QPushButton::clicked,
          this, &OptionsDialog::clearDiskCachedClicked);
  connect(ui->pushButtonOptionsCacheClearMemory, &QPushButton::clicked,
          this, &OptionsDialog::clearMemCachedClicked);
}

OptionsDialog::~OptionsDialog()
{
  delete rangeRingValidator;
  delete ui;
}

int OptionsDialog::exec()
{
  fromOptionData();

  bool hasAsn = mainWindow->getWeatherReporter()->hasAsnWeather();
  ui->checkBoxOptionsWeatherInfoAsn->setEnabled(hasAsn);
  ui->checkBoxOptionsWeatherTooltipAsn->setEnabled(hasAsn);

  return QDialog::exec();
}

void OptionsDialog::resetDefaultClicked()
{
  qDebug() << "OptionsDialog::resetDefaultClicked";

  QMessageBox::StandardButton result = QMessageBox::question(this, QApplication::applicationName(),
                                                             tr("Reset all options to default?"));

  if(result == QMessageBox::Yes)
  {
    OptionData::instance() = OptionData();
    fromOptionData();
    saveState();
    emit optionsChanged();
  }
}

void OptionsDialog::selectAsnPathClicked()
{
  qDebug() << "OptionsDialog::selectAsnPathClicked";
}

void OptionsDialog::clearMemCachedClicked()
{
  qDebug() << "OptionsDialog::clearMemCachedClicked";
}

void OptionsDialog::clearDiskCachedClicked()
{
  qDebug() << "OptionsDialog::clearDiskCachedClicked";
}

void OptionsDialog::buttonBoxClicked(QAbstractButton *button)
{
  qDebug() << "Clicked" << button->text();

  if(button == ui->buttonBoxOptions->button(QDialogButtonBox::Apply))
  {
    toOptionData();
    saveState();
    emit optionsChanged();
  }
  else if(button == ui->buttonBoxOptions->button(QDialogButtonBox::Ok))
  {
    toOptionData();
    saveState();
    emit optionsChanged();
    accept();
  }
  else if(button == ui->buttonBoxOptions->button(QDialogButtonBox::Cancel))
    reject();
}

void OptionsDialog::saveState()
{
  fromOptionData();

  atools::gui::WidgetState saver(lnm::OPTIONS_DIALOG_WIDGET, false, true);
  saver.save(widgets);
}

void OptionsDialog::restoreState()
{
  atools::gui::WidgetState saver(lnm::OPTIONS_DIALOG_WIDGET, false /*visibility*/, true /*block signals*/);
  saver.restore(widgets);

  toOptionData();
}

QVector<int> OptionsDialog::ringStrToVector(const QString& string) const
{
  QVector<int> rings;
  for(const QString& str :  string.split(" "))
  {
    QString val = str.trimmed();

    if(!val.isEmpty())
    {
      bool ok;
      int num = val.toInt(&ok);
      if(ok && num <= MAX_RANGE_RING_SIZE)
        rings.append(num);
    }
  }
  return rings;
}

void OptionsDialog::toOptionData()
{
  OptionData& data = OptionData::instanceInternal();

  toFlags(ui->checkBoxOptionsStartupLoadKml, opts::STARTUP_LOAD_KML);
  toFlags(ui->checkBoxOptionsStartupLoadMapSettings, opts::STARTUP_LOAD_MAP_SETTINGS);
  toFlags(ui->checkBoxOptionsStartupLoadRoute, opts::STARTUP_LOAD_ROUTE);
  toFlags(ui->radioButtonOptionsStartupShowHome, opts::STARTUP_SHOW_HOME);
  toFlags(ui->radioButtonOptionsStartupShowLast, opts::STARTUP_SHOW_LAST);
  toFlags(ui->checkBoxOptionsGuiCenterKml, opts::GUI_CENTER_KML);
  toFlags(ui->checkBoxOptionsGuiCenterRoute, opts::GUI_CENTER_ROUTE);
  toFlags(ui->checkBoxOptionsMapEmptyAirports, opts::MAP_EMPTY_AIRPORTS);
  toFlags(ui->checkBoxOptionsRouteEastWestRule, opts::ROUTE_EAST_WEST_RULE);
  toFlags(ui->checkBoxOptionsRoutePreferNdb, opts::ROUTE_PREFER_NDB);
  toFlags(ui->checkBoxOptionsRoutePreferVor, opts::ROUTE_PREFER_VOR);
  toFlags(ui->checkBoxOptionsWeatherInfoAsn, opts::WEATHER_INFO_ASN);
  toFlags(ui->checkBoxOptionsWeatherInfoNoaa, opts::WEATHER_INFO_NOAA);
  toFlags(ui->checkBoxOptionsWeatherInfoVatsim, opts::WEATHER_INFO_VATSIM);
  toFlags(ui->checkBoxOptionsWeatherTooltipAsn, opts::WEATHER_TOOLTIP_ASN);
  toFlags(ui->checkBoxOptionsWeatherTooltipNoaa, opts::WEATHER_TOOLTIP_NOAA);
  toFlags(ui->checkBoxOptionsWeatherTooltipVatsim, opts::WEATHER_TOOLTIP_VATSIM);

  data.mapRangeRings = ringStrToVector(ui->lineEditOptionsMapRangeRings->text());

  data.weatherActiveSkyPath = ui->lineEditOptionsWeatherAsnPath->text();

  data.databaseAddonExclude.clear();
  for(int i = 0; i < ui->listWidgetOptionsDatabaseAddon->count(); i++)
    data.databaseAddonExclude.append(ui->listWidgetOptionsDatabaseAddon->item(i)->text());

  data.databaseExclude.clear();
  for(int i = 0; i < ui->listWidgetOptionsDatabaseExclude->count(); i++)
    data.databaseExclude.append(ui->listWidgetOptionsDatabaseExclude->item(i)->text());

  if(ui->radioButtonOptionsMapScrollFull->isChecked())
    data.mapScrollDetail = opts::FULL;
  else if(ui->radioButtonOptionsMapScrollNone->isChecked())
    data.mapScrollDetail = opts::NONE;
  else if(ui->radioButtonOptionsMapScrollNormal->isChecked())
    data.mapScrollDetail = opts::NORMAL;

  if(ui->radioButtonOptionsMapSimUpdateFast->isChecked())
    data.mapSimUpdateRate = opts::FAST;
  else if(ui->radioButtonOptionsMapSimUpdateLow->isChecked())
    data.mapSimUpdateRate = opts::LOW;
  else if(ui->radioButtonOptionsMapSimUpdateMedium->isChecked())
    data.mapSimUpdateRate = opts::MEDIUM;

  data.cacheSizeDisk = ui->spinBoxOptionsCacheDiskSize->value();
  data.cacheSizeMemory = ui->spinBoxOptionsCacheMemorySize->value();
  data.guiInfoTextSize = ui->spinBoxOptionsGuiInfoText->value();
  data.guiRouteTableTextSize = ui->spinBoxOptionsGuiRouteText->value();
  data.guiSearchTableTextSize = ui->spinBoxOptionsGuiSearchText->value();
  data.guiInfoSimSize = ui->spinBoxOptionsGuiSimInfoText->value();
  data.mapClickSensitivity = ui->spinBoxOptionsMapClickRect->value();
  data.mapTooltipSensitivity = ui->spinBoxOptionsMapTooltipRect->value();
  data.mapSymbolSize = ui->spinBoxOptionsMapSymbolSize->value();
  data.mapTextSize = ui->spinBoxOptionsMapTextSize->value();
  data.mapZoomShow = static_cast<float>(ui->doubleSpinBoxOptionsMapZoomShowMap->value());
  data.routeGroundBuffer = ui->spinBoxOptionsRouteGroundBuffer->value();
  data.valid = true;
}

void OptionsDialog::fromOptionData()
{
  fromFlags(ui->checkBoxOptionsStartupLoadKml, opts::STARTUP_LOAD_KML);
  fromFlags(ui->checkBoxOptionsStartupLoadMapSettings, opts::STARTUP_LOAD_MAP_SETTINGS);
  fromFlags(ui->checkBoxOptionsStartupLoadRoute, opts::STARTUP_LOAD_ROUTE);
  fromFlags(ui->radioButtonOptionsStartupShowHome, opts::STARTUP_SHOW_HOME);
  fromFlags(ui->radioButtonOptionsStartupShowLast, opts::STARTUP_SHOW_LAST);
  fromFlags(ui->checkBoxOptionsGuiCenterKml, opts::GUI_CENTER_KML);
  fromFlags(ui->checkBoxOptionsGuiCenterRoute, opts::GUI_CENTER_ROUTE);
  fromFlags(ui->checkBoxOptionsMapEmptyAirports, opts::MAP_EMPTY_AIRPORTS);
  fromFlags(ui->checkBoxOptionsRouteEastWestRule, opts::ROUTE_EAST_WEST_RULE);
  fromFlags(ui->checkBoxOptionsRoutePreferNdb, opts::ROUTE_PREFER_NDB);
  fromFlags(ui->checkBoxOptionsRoutePreferVor, opts::ROUTE_PREFER_VOR);
  fromFlags(ui->checkBoxOptionsWeatherInfoAsn, opts::WEATHER_INFO_ASN);
  fromFlags(ui->checkBoxOptionsWeatherInfoNoaa, opts::WEATHER_INFO_NOAA);
  fromFlags(ui->checkBoxOptionsWeatherInfoVatsim, opts::WEATHER_INFO_VATSIM);
  fromFlags(ui->checkBoxOptionsWeatherTooltipAsn, opts::WEATHER_TOOLTIP_ASN);
  fromFlags(ui->checkBoxOptionsWeatherTooltipNoaa, opts::WEATHER_TOOLTIP_NOAA);
  fromFlags(ui->checkBoxOptionsWeatherTooltipVatsim, opts::WEATHER_TOOLTIP_VATSIM);

  OptionData& data = OptionData::instance();

  QString txt;
  for(int val : data.mapRangeRings)
  {
    if(val > 0)
    {
      if(!txt.isEmpty())
        txt += " ";
      txt += QString::number(val);
    }
  }
  ui->lineEditOptionsMapRangeRings->setText(txt);
  ui->lineEditOptionsWeatherAsnPath->setText(data.weatherActiveSkyPath);

  ui->listWidgetOptionsDatabaseAddon->clear();
  for(const QString& str : data.databaseAddonExclude)
    ui->listWidgetOptionsDatabaseAddon->addItem(str);

  ui->listWidgetOptionsDatabaseExclude->clear();
  for(const QString& str : data.databaseExclude)
    ui->listWidgetOptionsDatabaseExclude->addItem(str);

  switch(data.mapScrollDetail)
  {
    case opts::FULL:
      ui->radioButtonOptionsMapScrollFull->setChecked(true);
      break;
    case opts::NORMAL:
      ui->radioButtonOptionsMapScrollNormal->setChecked(true);
      break;
    case opts::NONE:
      ui->radioButtonOptionsMapScrollNone->setChecked(true);
      break;
  }

  switch(data.mapSimUpdateRate)
  {
    case opts::FAST:
      ui->radioButtonOptionsMapSimUpdateFast->setChecked(true);
      break;
    case opts::MEDIUM:
      ui->radioButtonOptionsMapSimUpdateMedium->setChecked(true);
      break;
    case opts::LOW:
      ui->radioButtonOptionsMapSimUpdateLow->setChecked(true);
      break;
  }

  ui->spinBoxOptionsCacheDiskSize->setValue(data.cacheSizeDisk);
  ui->spinBoxOptionsCacheMemorySize->setValue(data.cacheSizeMemory);
  ui->spinBoxOptionsGuiInfoText->setValue(data.guiInfoTextSize);
  ui->spinBoxOptionsGuiRouteText->setValue(data.guiRouteTableTextSize);
  ui->spinBoxOptionsGuiSearchText->setValue(data.guiSearchTableTextSize);
  ui->spinBoxOptionsGuiSimInfoText->setValue(data.guiInfoSimSize);
  ui->spinBoxOptionsMapClickRect->setValue(data.mapClickSensitivity);
  ui->spinBoxOptionsMapTooltipRect->setValue(data.mapTooltipSensitivity);
  ui->spinBoxOptionsMapSymbolSize->setValue(data.mapSymbolSize);
  ui->spinBoxOptionsMapTextSize->setValue(data.mapTextSize);
  ui->doubleSpinBoxOptionsMapZoomShowMap->setValue(data.mapZoomShow);
  ui->spinBoxOptionsRouteGroundBuffer->setValue(data.routeGroundBuffer);
}

void OptionsDialog::toFlags(QCheckBox *checkBox, opts::Flags flag)
{
  if(checkBox->isChecked())
    OptionData::instanceInternal().flags |= flag;
  else
    OptionData::instanceInternal().flags &= ~flag;
}

void OptionsDialog::toFlags(QRadioButton *checkBox, opts::Flags flag)
{
  if(checkBox->isChecked())
    OptionData::instanceInternal().flags |= flag;
  else
    OptionData::instanceInternal().flags &= ~flag;
}

void OptionsDialog::fromFlags(QCheckBox *checkBox, opts::Flags flag)
{
  checkBox->setChecked(OptionData::instanceInternal().flags & flag);
}

void OptionsDialog::fromFlags(QRadioButton *checkBox, opts::Flags flag)
{
  checkBox->setChecked(OptionData::instanceInternal().flags & flag);
}
