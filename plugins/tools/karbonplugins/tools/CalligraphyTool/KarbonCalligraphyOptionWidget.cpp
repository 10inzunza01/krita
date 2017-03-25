/* This file is part of the KDE project
   Copyright (C) 2008 Fela Winkelmolen <fela.kde@gmail.com>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "KarbonCalligraphyOptionWidget.h"

#include <KoIcon.h>

#include <klocalizedstring.h>
#include <kcombobox.h>
#include <kconfiggroup.h>
#include <ksharedconfig.h>
#include <kconfig.h>
#include <QDebug>
#include <kmessagebox.h>

#include <QInputDialog>
#include <QSpinBox>
#include <QCheckBox>
#include <QDoubleSpinBox>
#include <QLabel>
#include <QVBoxLayout>
#include <QToolButton>
#include <QTabWidget>

#include <kis_popup_button.h>

#include "kis_double_parse_spin_box.h"
#include "kis_int_parse_spin_box.h"
#include "ui_karboncalligraphytooloptions.h"
#include "kis_slider_spin_box.h"
#include <kis_pressure_size_option.h>
#include <kis_pressure_rotation_option.h>
#include <kis_pressure_ratio_option.h>
#include <kis_curve_option_widget.h>
/*
Profiles are saved in karboncalligraphyrc

In the group "General", profile is the name of profile used

Every profile is described in a group, the name of which is "ProfileN"
Starting to count from 0 onwards
(NOTE: the index in profiles is different from the N)

Default profiles are added by the function addDefaultProfiles(), once they
have been added, the entry defaultProfilesAdded in the "General" group is
set to true

TODO: add a reset defaults option?
*/

// name of the configuration file
const QString RCFILENAME = "karboncalligraphyrc";

class KarbonCalligraphyToolOptions: public QWidget, public Ui::WdgCalligraphyToolOptions
{
public:
    KarbonCalligraphyToolOptions(QWidget *parent = 0)
        : QWidget(parent) {
        setupUi(this);
    }
};

KarbonCalligraphyOptionWidget::KarbonCalligraphyOptionWidget()
    : m_changingProfile(false)
{
    m_options = new KarbonCalligraphyToolOptions();
    m_options->setupUi(this);

    m_options->sldDistanceInterval->setPrefix(i18n("Distance: "));
    //the distance is in SCREEN coordinates!
    m_options->sldDistanceInterval->setSuffix(i18n("px"));
    m_options->sldDistanceInterval->setRange(1, 1000, 2);
    m_options->sldDistanceInterval->setSingleStep(1);

    m_options->sldTimeInterval->setPrefix(i18n("Time: "));
    m_options->sldTimeInterval->setSuffix(i18n("ms"));
    m_options->sldTimeInterval->setRange(1, 1000, 2);
    m_options->sldTimeInterval->setSingleStep(1);

    m_options->sldCaps->setPrefix(i18n("Caps: "));
    m_options->sldCaps->setRange(0.0, 2.0, 2);
    m_options->sldCaps->setSingleStep(0.03);

    m_sizeOption = new KisCurveOptionWidget(new KisPressureSizeOption(), i18n("0%"), i18n("100%"));
    m_rotationOption = new KisCurveOptionWidget(new KisPressureRotationOption(), i18n("-180°"), i18n("180°"));
    //m_ratioOption = new KisCurveOptionWidget(new KisPressureRatioOption(), i18n("0%"), i18n("100%"));
    m_rotationOption->setChecked(true);
    m_sizeOption->setChecked(true);

    m_options->bnSize->setIcon(kisIcon("brush_size"));
    m_options->bnSize->setPopupWidget(m_sizeOption->configurationPage());

    m_options->bnRotation->setIcon(kisIcon("brush_rotation"));
    m_options->bnRotation->setPopupWidget(m_rotationOption->configurationPage());

    m_options->bnRatio->hide();
    //m_options->bnRatio->setIcon(kisIcon("brush_ratio"));
    //m_options->bnRatio->setPopupWidget(m_ratioOption->configurationPage());

    createConnections();
    addDefaultProfiles(); // if they are already added does nothing
    loadProfiles();
}

KarbonCalligraphyOptionWidget::~KarbonCalligraphyOptionWidget()
{
    qDeleteAll(m_profiles);
}

void KarbonCalligraphyOptionWidget::emitAll()
{
    emit usePathChanged(m_options->rdAdjustPath->isChecked());
    emit useAssistantChanged(m_options->rdAdjustAssistant->isChecked());
    emit useNoAdjustChanged(m_options->rdNoAdjust->isChecked());
    emit generateSettings();
    emit smoothTimeChanged(m_options->sldTimeInterval->value());
    emit smoothDistanceChanged(m_options->sldDistanceInterval->value());
}

void KarbonCalligraphyOptionWidget::loadProfile(const QString &name)
{
    if (m_changingProfile) {
        return;
    }
    // write the new profile in the config file
    KConfig config(RCFILENAME);
    KConfigGroup generalGroup(&config, "General");
    generalGroup.writeEntry("profile", name);
    config.sync();

    // and load it
    loadCurrentProfile();

    // don't show Current if it isn't selected
    if (name != i18n("Current")) {
        removeProfile(i18n("Current"));
    }
}

void KarbonCalligraphyOptionWidget::updateCurrentProfile()
{
    if (!m_changingProfile) {
        saveProfile("Current");
    }
}

void KarbonCalligraphyOptionWidget::saveProfileAs()
{
    QString name;

    // loop until a valid name is entered or the user cancelled
    while (1) {
        bool ok;
        name = QInputDialog::getText(this,
                                     i18n("Profile name"),
                                     i18n("Please insert the name by which "
                                          "you want to save this profile:"),
                                     QLineEdit::Normal, QString(), &ok);
        if (!ok) {
            return;
        }

        if (name.isEmpty() || name == i18n("Current")) {
            KMessageBox::sorry(this,
                               i18n("Sorry, the name you entered is invalid."),
                               i18nc("invalid profile name", "Invalid name."));
            // try again
            saveProfileAs();
            continue; // ask again
        }

        if (m_profiles.contains(name)) {
            int ret = KMessageBox::warningYesNo(this,
                                                i18n("A profile with that name already exists.\n"
                                                        "Do you want to overwrite it?"));

            if (ret == KMessageBox::Yes) {
                break;    // exit while loop (save profile)
            }
            // else ask again
        } else {
            // the name is valid
            break; // exit while loop (save profile)
        }
    }

    saveProfile(name);
}

void KarbonCalligraphyOptionWidget::removeProfile()
{
    removeProfile(m_options->cmbProfiles->currentText());
}

void KarbonCalligraphyOptionWidget::generateSettings()
{
    KisPropertiesConfigurationSP settings = new KisPropertiesConfiguration();
    settings->setProperty("capSize", m_options->sldCaps->value());
    m_sizeOption->writeOptionSetting(settings);
    m_rotationOption->writeOptionSetting(settings);
    emit settingsChanged(settings);
}

/******************************************************************************
 ************************* Convenience Functions ******************************
 ******************************************************************************/

void KarbonCalligraphyOptionWidget::createConnections()
{
    connect(m_options->cmbProfiles, SIGNAL(currentIndexChanged(QString)),
            SLOT(loadProfile(QString)));

    // propagate changes
    connect(m_options->rdAdjustPath, SIGNAL(toggled(bool)),
            SIGNAL(usePathChanged(bool)));
    connect(m_options->rdAdjustAssistant, SIGNAL(toggled(bool)),
            SIGNAL(useAssistantChanged(bool)));
    connect(m_options->rdNoAdjust, SIGNAL(toggled(bool)),
            SIGNAL(useNoAdjustChanged(bool)));

    connect(m_options->sldCaps, SIGNAL(valueChanged(double)),
            SLOT(generateSettings()));

    connect(m_options->sldTimeInterval, SIGNAL(valueChanged(double)),
            SIGNAL(smoothTimeChanged(double)));
    connect(m_options->sldDistanceInterval, SIGNAL(valueChanged(double)),
            SIGNAL(smoothDistanceChanged(double)));

    connect(m_sizeOption, SIGNAL(sigSettingChanged()), SLOT(generateSettings()));
    connect(m_rotationOption, SIGNAL(sigSettingChanged()), SLOT(generateSettings()));

    // update profile
    connect(m_options->rdAdjustPath, SIGNAL(toggled(bool)),
            SLOT(updateCurrentProfile()));
    connect(m_options->rdAdjustAssistant, SIGNAL(toggled(bool)),
            SLOT(updateCurrentProfile()));
    connect(m_options->rdNoAdjust, SIGNAL(toggled(bool)),
            SLOT(updateCurrentProfile()));
    connect(m_options->sldCaps, SIGNAL(valueChanged(double)),
            SLOT(updateCurrentProfile()));
    connect(m_options->sldTimeInterval, SIGNAL(valueChanged(double)),
            SLOT(updateCurrentProfile()));
    connect(m_options->sldDistanceInterval, SIGNAL(valueChanged(double)),
            SLOT(updateCurrentProfile()));
    connect(m_sizeOption, SIGNAL(sigSettingChanged()), SLOT(updateCurrentProfile()));
    connect(m_rotationOption, SIGNAL(sigSettingChanged()), SLOT(updateCurrentProfile()));

    connect(m_options->bnSaveProfile, SIGNAL(clicked()), SLOT(saveProfileAs()));
    connect(m_options->bnRemoveProfile, SIGNAL(clicked()), SLOT(removeProfile()));

    // visualization
    //connect(m_useAngle, SIGNAL(toggled(bool)), SLOT(toggleUseAngle(bool)));
}

void KarbonCalligraphyOptionWidget::addDefaultProfiles()
{
    // check if the profiles where already added
    KConfig config(RCFILENAME);
    KConfigGroup generalGroup(&config, "General");

    if (generalGroup.readEntry("defaultProfilesAdded", false)) {
        return;
    }

    KConfigGroup profile0(&config, "Profile0");
    profile0.writeEntry("name", i18n("Mouse"));
    profile0.writeEntry("usePath", false);
    profile0.writeEntry("useAssistants", false);
    profile0.writeEntry("caps", 0.0);
    profile0.writeEntry("timeInterval", 0);
    profile0.writeEntry("distanceInterval", 0);
    QString curveConfigMouse("<!DOCTYPE params>\n<params>\n<param type='bool' "
                                       " name='PressureRotation'>true</param>\n <param type='bool' name='PressureSize'>"
                                       "true</param>\n <param type='string' name='RotationSensor'><![CDATA[<!DOCTYPE params>"
                                       "\n<params fanCornersEnabled='0' lockedAngleMode='0' angleOffset='90' "
                                       " id='drawingangle' fanCornersStep='30'/>\n]]></param>\n"
                                       "<param type='bool' name='RotationUseCurve'>false</param>\n"
                                       "<param type='bool' name='RotationUseSameCurve'>true</param>\n"
                                       "<param type='double' name='RotationValue'>1</param>\n"
                                       "<param type='string' name='SizeSensor'>"
                                       "<![CDATA[<!DOCTYPE params>\n<params id='pressure'>\n"
                                       "<curve>0,0;1,1;</curve>\n</params>\n]]></param>"
                                       "<param type='bool' name='SizeUseCurve'>false</param>\n"
                                       "<param type='bool' name='SizeUseSameCurve'>true</param>\n"
                                       "<param type='double' name='SizeValue'>1</param>\n</params>");
    profile0.writeEntry("curveConfig", curveConfigMouse);

    KConfigGroup profile1(&config, "Profile1");
    profile1.writeEntry("name", i18n("Brush"));
    profile1.writeEntry("usePath", false);
    profile1.writeEntry("useAssistants", false);
    profile1.writeEntry("caps", 0.0);
    profile1.writeEntry("timeInterval", 50);
    profile1.writeEntry("distanceInterval", 15);
    QString curveConfigBrush("<!DOCTYPE params>\n<params>\n<param type='bool' "
                                       " name='PressureRotation'>true</param>\n <param type='bool' name='PressureSize'>"
                                       "true</param>\n <param type='string' name='RotationSensor'><![CDATA[<!DOCTYPE params>"
                                       "\n<params fanCornersEnabled='0' lockedAngleMode='0' angleOffset='90' "
                                       " id='drawingangle' fanCornersStep='30'/>\n]]></param>\n"
                                       "<param type='bool' name='RotationUseCurve'>true</param>\n"
                                       "<param type='bool' name='RotationUseSameCurve'>true</param>\n"
                                       "<param type='double' name='RotationValue'>1</param>\n"
                                       "<param type='string' name='SizeSensor'>"
                                       "<![CDATA[<!DOCTYPE params>\n<params id='pressure'>\n"
                                       "<curve>0,0;0.375,0.25;0.625,0.75;1,1;</curve>\n</params>\n]]></param>"
                                       "<param type='bool' name='SizeUseCurve'>true</param>\n"
                                       "<param type='bool' name='SizeUseSameCurve'>true</param>\n"
                                       "<param type='double' name='SizeValue'>1</param>\n</params>");
    profile1.writeEntry("curveConfig", curveConfigBrush);

    KConfigGroup profile2(&config, "Profile2");
    profile2.writeEntry("name", i18n("GPen"));
    profile2.writeEntry("usePath", false);
    profile2.writeEntry("useAssistants", false);
    profile2.writeEntry("caps", 0.0);
    profile2.writeEntry("timeInterval", 50);
    profile2.writeEntry("distanceInterval", 15);
    QString curveConfigGpen("<!DOCTYPE params>\n<params>\n<param type='bool' "
                                      " name='PressureRotation'>true</param>\n <param type='bool' name='PressureSize'>"
                                      "true</param>\n <param type='string' name='RotationSensor'><![CDATA[<!DOCTYPE params>"
                                      "\n<params fanCornersEnabled='0' lockedAngleMode='0' angleOffset='90' "
                                      " id='drawingangle' fanCornersStep='30'/>\n]]></param>\n"
                                      "<param type='bool' name='RotationUseCurve'>true</param>\n"
                                      "<param type='bool' name='RotationUseSameCurve'>true</param>\n"
                                      "<param type='double' name='RotationValue'>1</param>\n"
                                      "<param type='string' name='SizeSensor'>"
                                      "<![CDATA[<!DOCTYPE params>\n<params id='pressure'>\n"
                                      "<curve>0,0;0.625,0.375;1,1;</curve>\n</params>\n]]></param>"
                                      "<param type='bool' name='SizeUseCurve'>true</param>\n"
                                      "<param type='bool' name='SizeUseSameCurve'>true</param>\n"
                                      "<param type='double' name='SizeValue'>1</param>\n</params>");
    profile2.writeEntry("curveConfig", curveConfigGpen);

    generalGroup.writeEntry("profile", i18n("Brush"));
    generalGroup.writeEntry("defaultProfilesAdded", true);

    config.sync();
}

void KarbonCalligraphyOptionWidget::loadProfiles()
{
    KConfig config(RCFILENAME);

    // load profiles as long as they are present
    int i = 0;
    while (1) { // forever
        KConfigGroup profileGroup(&config, "Profile" + QString::number(i));
        // invalid profile, assume we reached the last one
        if (!profileGroup.hasKey("name")) {
            break;
        }

        Profile *profile = new Profile;
        profile->index = i;
        profile->name =             profileGroup.readEntry("name", QString());
        profile->usePath =          profileGroup.readEntry("usePath", false);
        profile->useAssistants =          profileGroup.readEntry("useAssistants", false);
        profile->caps =             profileGroup.readEntry("caps", 0.0);
        profile->timeInterval =     profileGroup.readEntry("timeInterval", 0.0);
        profile->distanceInterval = profileGroup.readEntry("distanceInterval", 0.0);
        profile->curveConfig = new KisPropertiesConfiguration();
        profile->curveConfig->fromXML(profileGroup.readEntry("curveConfig", QString()));

        m_profiles.insert(profile->name, profile);
        ++i;
    }

    m_changingProfile = true;
    ProfileMap::const_iterator it = m_profiles.constBegin();
    ProfileMap::const_iterator lastIt = m_profiles.constEnd();
    for (; it != lastIt; ++it) {
        m_options->cmbProfiles->addItem(it.key());
    }
    m_changingProfile = false;

    loadCurrentProfile();
}

void KarbonCalligraphyOptionWidget::loadCurrentProfile()
{
    KConfig config(RCFILENAME);
    KConfigGroup generalGroup(&config, "General");
    QString currentProfile = generalGroup.readEntry("profile", QString());
    // find the index needed by the comboBox
    int index = profilePosition(currentProfile);

    if (currentProfile.isEmpty() || index < 0) {
        return;
    }

    m_options->cmbProfiles->setCurrentIndex(index);

    Profile *profile = m_profiles[currentProfile];

    m_changingProfile = true;
    m_options->rdAdjustPath->setChecked(profile->usePath);
    m_options->rdAdjustAssistant->setChecked(profile->useAssistants);
    if (profile->useAssistants == false && profile->usePath==false) {
        m_options->rdNoAdjust->setChecked(true);
    }
    m_options->sldCaps->setValue(profile->caps);
    m_options->sldTimeInterval->setValue(profile->timeInterval);
    m_options->sldDistanceInterval->setValue(profile->distanceInterval);
    m_sizeOption->readOptionSetting(profile->curveConfig);
    m_rotationOption->readOptionSetting(profile->curveConfig);
    m_changingProfile = false;
}

void KarbonCalligraphyOptionWidget::saveProfile(const QString &name)
{
    Profile *profile = new Profile;
    profile->name = name;
    profile->usePath = m_options->rdAdjustPath->isChecked();
    profile->caps = m_options->sldCaps->value();
    profile->useAssistants = m_options->rdAdjustAssistant->isChecked();
    profile->timeInterval = m_options->sldTimeInterval->value();
    profile->distanceInterval = m_options->sldDistanceInterval->value();
    profile->curveConfig = new KisPropertiesConfiguration();
    m_sizeOption->writeOptionSetting(profile->curveConfig);
    m_rotationOption->writeOptionSetting(profile->curveConfig);

    if (m_profiles.contains(name)) {
        // there is already a profile with the same name, overwrite
        profile->index = m_profiles[name]->index;
        m_profiles.insert(name, profile);
    } else {
        // it is a new profile
        profile->index = m_profiles.count();
        m_profiles.insert(name, profile);
        // add the profile to the combobox
        QString dbg;
        for (int i = 0; i < m_options->cmbProfiles->count(); ++i) {
            dbg += m_options->cmbProfiles->itemText(i) + ' ';
        }
        int pos = profilePosition(name);
        m_changingProfile = true;
        m_options->cmbProfiles->insertItem(pos, name);
        m_changingProfile = false;
        for (int i = 0; i < m_options->cmbProfiles->count(); ++i) {
            dbg += m_options->cmbProfiles->itemText(i) + ' ';
        }
    }

    KConfig config(RCFILENAME);
    QString str = "Profile" + QString::number(profile->index);
    KConfigGroup profileGroup(&config, str);

    profileGroup.writeEntry("name", name);
    profileGroup.writeEntry("usePath", profile->usePath);
    profileGroup.writeEntry("useAssistants", profile->useAssistants);
    profileGroup.writeEntry("caps", profile->caps);
    profileGroup.writeEntry("timeInterval", profile->timeInterval);
    profileGroup.writeEntry("distanceInterval", profile->distanceInterval);
    profileGroup.writeEntry("curveConfig", profile->curveConfig->toXML());

    KConfigGroup generalGroup(&config, "General");
    generalGroup.writeEntry("profile", name);

    config.sync();

    m_options->cmbProfiles->setCurrentIndex(profilePosition(name));
}

void KarbonCalligraphyOptionWidget::removeProfile(const QString &name)
{
    int index = profilePosition(name);
    if (index < 0) {
        return;    // no such profile
    }

    // remove the file from the config file
    KConfig config(RCFILENAME);
    int deletedIndex = m_profiles[name]->index;
    QString deletedGroup = "Profile" + QString::number(deletedIndex);
    config.deleteGroup(deletedGroup);
    config.sync();

    // and from profiles
    m_profiles.remove(name);

    m_options->cmbProfiles->removeItem(index);

    // now in the config file there is value ProfileN missing,
    // where N = configIndex, so put the last one there
    if (m_profiles.isEmpty()) {
        return;
    }

    int lastN = -1;
    Profile *profile = 0; // profile to be moved, will be the last one
    Q_FOREACH (Profile *p, m_profiles) {
        if (p->index > lastN) {
            lastN = p->index;
            profile = p;
        }
    }

    Q_ASSERT(profile != 0);

    // do nothing if the deleted group was the last one
    if (deletedIndex > lastN) {
        return;
    }

    QString lastGroup = "Profile" + QString::number(lastN);
    config.deleteGroup(lastGroup);

    KConfigGroup profileGroup(&config, deletedGroup);
    profileGroup.writeEntry("name", name);
    profileGroup.writeEntry("usePath", profile->usePath);
    profileGroup.writeEntry("useAssistants", profile->useAssistants);
    profileGroup.writeEntry("caps", profile->caps);
    profileGroup.writeEntry("timeInterval", profile->timeInterval);
    profileGroup.writeEntry("distanceInterval", profile->distanceInterval);
    config.sync();

    profile->index = deletedIndex;
}

int KarbonCalligraphyOptionWidget::profilePosition(const QString &profileName)
{
    int res = 0;
    ProfileMap::const_iterator it = m_profiles.constBegin();
    ProfileMap::const_iterator lastIt = m_profiles.constEnd();
    for (; it != lastIt; ++it) {
        if (it.key() == profileName) {
            return res;
        }
        ++res;
    }
    return -1;
}
