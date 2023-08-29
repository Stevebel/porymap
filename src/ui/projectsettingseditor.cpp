#include "projectsettingseditor.h"
#include "ui_projectsettingseditor.h"
#include "config.h"
#include "noscrollcombobox.h"

#include <QAbstractButton>
#include <QFormLayout>

/*
    Editor for the settings in a user's porymap.project.cfg and porymap.user.cfg files.
*/

ProjectSettingsEditor::ProjectSettingsEditor(QWidget *parent, Project *project) :
    QMainWindow(parent),
    ui(new Ui::ProjectSettingsEditor),
    project(project)
{
    ui->setupUi(this);
    setAttribute(Qt::WA_DeleteOnClose);
    this->initUi();
    this->connectSignals();
    this->refresh();
    this->restoreWindowState();
}

ProjectSettingsEditor::~ProjectSettingsEditor()
{
    delete ui;
}

// TODO: Move tool tips to editable areas

void ProjectSettingsEditor::connectSignals() {
    connect(ui->buttonBox, &QDialogButtonBox::clicked, this, &ProjectSettingsEditor::dialogButtonClicked);
    connect(ui->button_ChoosePrefabs, &QAbstractButton::clicked, this, &ProjectSettingsEditor::choosePrefabsFileClicked);

    // Connect combo boxes
    QList<NoScrollComboBox *> combos = ui->centralwidget->findChildren<NoScrollComboBox *>();
    foreach(auto i, combos)
        connect(i, &QComboBox::currentTextChanged, this, &ProjectSettingsEditor::markEdited);
    connect(ui->comboBox_BaseGameVersion, &QComboBox::currentTextChanged, this, &ProjectSettingsEditor::promptRestoreDefaults);

    // Connect check boxes
    QList<QCheckBox *> checkboxes = ui->centralwidget->findChildren<QCheckBox *>();
    foreach(auto i, checkboxes)
        connect(i, &QCheckBox::stateChanged, this, &ProjectSettingsEditor::markEdited);

    // Connect spin boxes
    QList<QSpinBox *> spinBoxes = ui->centralwidget->findChildren<QSpinBox *>();
    foreach(auto i, spinBoxes)
        connect(i, QOverload<int>::of(&QSpinBox::valueChanged), [this](int) { this->markEdited(); });

    // Connect line edits
    QList<QLineEdit *> lineEdits = ui->centralwidget->findChildren<QLineEdit *>();
    foreach(auto i, lineEdits)
        connect(i, &QLineEdit::textEdited, this, &ProjectSettingsEditor::markEdited);
}

void ProjectSettingsEditor::markEdited() {
    // Don't treat signals emitted while the UI is refreshing as edits
    if (!this->refreshing)
        this->hasUnsavedChanges = true;
}

void ProjectSettingsEditor::initUi() {
    // Populate combo boxes
    if (project) ui->comboBox_DefaultPrimaryTileset->addItems(project->primaryTilesetLabels);
    if (project) ui->comboBox_DefaultSecondaryTileset->addItems(project->secondaryTilesetLabels);
    ui->comboBox_BaseGameVersion->addItems(ProjectConfig::versionStrings);
    ui->comboBox_AttributesSize->addItems({"1", "2", "4"});

    // Validate that the border metatiles text is a comma-separated list of metatile values
    const QString regex_Hex = "(0[xX])?[A-Fa-f0-9]+";
    static const QRegularExpression expression(QString("^(%1,)*%1$").arg(regex_Hex)); // Comma-separated list of hex values
    QRegularExpressionValidator *validator = new QRegularExpressionValidator(expression);
    ui->lineEdit_BorderMetatiles->setValidator(validator);

    ui->spinBox_Elevation->setMaximum(15);
    ui->spinBox_FillMetatile->setMaximum(Project::getNumMetatilesTotal() - 1);

    // TODO: These need to be subclassed (or changed to line edits) to handle larger values
    ui->spinBox_BehaviorMask->setMaximum(INT_MAX);
    ui->spinBox_EncounterTypeMask->setMaximum(INT_MAX);
    ui->spinBox_LayerTypeMask->setMaximum(INT_MAX);
    ui->spinBox_TerrainTypeMask->setMaximum(INT_MAX);
}

void ProjectSettingsEditor::restoreWindowState() {
    logInfo("Restoring project settings editor geometry from previous session.");
    const QMap<QString, QByteArray> geometry = porymapConfig.getProjectSettingsEditorGeometry();
    this->restoreGeometry(geometry.value("project_settings_editor_geometry"));
    this->restoreState(geometry.value("project_settings_editor_state"));
}

// Set UI states using config data
void ProjectSettingsEditor::refresh() {
    this->refreshing = true; // Block signals

    // Set combo box texts
    ui->comboBox_DefaultPrimaryTileset->setTextItem(projectConfig.getDefaultPrimaryTileset());
    ui->comboBox_DefaultSecondaryTileset->setTextItem(projectConfig.getDefaultSecondaryTileset());
    ui->comboBox_BaseGameVersion->setTextItem(projectConfig.getBaseGameVersionString());
    ui->comboBox_AttributesSize->setTextItem(QString::number(projectConfig.getMetatileAttributesSize()));

    // Set check box states
    ui->checkBox_UsePoryscript->setChecked(projectConfig.getUsePoryScript());
    ui->checkBox_ShowWildEncounterTables->setChecked(userConfig.getEncounterJsonActive());
    ui->checkBox_CreateTextFile->setChecked(projectConfig.getCreateMapTextFileEnabled());
    ui->checkBox_PrefabImportPrompted->setChecked(projectConfig.getPrefabImportPrompted());
    ui->checkBox_EnableTripleLayerMetatiles->setChecked(projectConfig.getTripleLayerMetatilesEnabled());
    ui->checkBox_EnableRequiresItemfinder->setChecked(projectConfig.getHiddenItemRequiresItemfinderEnabled());
    ui->checkBox_EnableQuantity->setChecked(projectConfig.getHiddenItemQuantityEnabled());
    ui->checkBox_EnableCloneObjects->setChecked(projectConfig.getEventCloneObjectEnabled());
    ui->checkBox_EnableWeatherTriggers->setChecked(projectConfig.getEventWeatherTriggerEnabled());
    ui->checkBox_EnableSecretBases->setChecked(projectConfig.getEventSecretBaseEnabled());
    ui->checkBox_EnableRespawn->setChecked(projectConfig.getHealLocationRespawnDataEnabled());
    ui->checkBox_EnableAllowFlags->setChecked(projectConfig.getMapAllowFlagsEnabled());
    ui->checkBox_EnableFloorNumber->setChecked(projectConfig.getFloorNumberEnabled());
    ui->checkBox_EnableCustomBorderSize->setChecked(projectConfig.getUseCustomBorderSize());
    ui->checkBox_OutputCallback->setChecked(projectConfig.getTilesetsHaveCallback());
    ui->checkBox_OutputIsCompressed->setChecked(projectConfig.getTilesetsHaveIsCompressed());

    // Set spin box values
    ui->spinBox_Elevation->setValue(projectConfig.getNewMapElevation());
    ui->spinBox_FillMetatile->setValue(projectConfig.getNewMapMetatileId());
    ui->spinBox_BehaviorMask->setValue(projectConfig.getMetatileBehaviorMask());
    ui->spinBox_EncounterTypeMask->setValue(projectConfig.getMetatileEncounterTypeMask());
    ui->spinBox_LayerTypeMask->setValue(projectConfig.getMetatileLayerTypeMask());
    ui->spinBox_TerrainTypeMask->setValue(projectConfig.getMetatileTerrainTypeMask());

    // Set line edit texts
    ui->lineEdit_BorderMetatiles->setText(projectConfig.getNewMapBorderMetatileIdsString());
    ui->lineEdit_PrefabsPath->setText(projectConfig.getPrefabFilepath(false));

    this->refreshing = false; // Allow signals
}

void ProjectSettingsEditor::save() {
    if (!this->hasUnsavedChanges)
        return;

    // Prevent a call to save() for each of the config settings
    projectConfig.setSaveDisabled(true);
    userConfig.setSaveDisabled(true);

    projectConfig.setDefaultPrimaryTileset(ui->comboBox_DefaultPrimaryTileset->currentText());
    projectConfig.setDefaultSecondaryTileset(ui->comboBox_DefaultSecondaryTileset->currentText());
    projectConfig.setBaseGameVersion(projectConfig.stringToBaseGameVersion(ui->comboBox_BaseGameVersion->currentText()));
    projectConfig.setMetatileAttributesSize(ui->comboBox_AttributesSize->currentText().toInt());
    projectConfig.setUsePoryScript(ui->checkBox_UsePoryscript->isChecked());
    userConfig.setEncounterJsonActive(ui->checkBox_ShowWildEncounterTables->isChecked());
    projectConfig.setCreateMapTextFileEnabled(ui->checkBox_CreateTextFile->isChecked());
    projectConfig.setPrefabImportPrompted(ui->checkBox_PrefabImportPrompted->isChecked());
    projectConfig.setTripleLayerMetatilesEnabled(ui->checkBox_EnableTripleLayerMetatiles->isChecked());
    projectConfig.setHiddenItemRequiresItemfinderEnabled(ui->checkBox_EnableRequiresItemfinder->isChecked());
    projectConfig.setHiddenItemQuantityEnabled(ui->checkBox_EnableQuantity->isChecked());
    projectConfig.setEventCloneObjectEnabled(ui->checkBox_EnableCloneObjects->isChecked());
    projectConfig.setEventWeatherTriggerEnabled(ui->checkBox_EnableWeatherTriggers->isChecked());
    projectConfig.setEventSecretBaseEnabled(ui->checkBox_EnableSecretBases->isChecked());
    projectConfig.setHealLocationRespawnDataEnabled(ui->checkBox_EnableRespawn->isChecked());
    projectConfig.setMapAllowFlagsEnabled(ui->checkBox_EnableAllowFlags->isChecked());
    projectConfig.setFloorNumberEnabled(ui->checkBox_EnableFloorNumber->isChecked());
    projectConfig.setUseCustomBorderSize(ui->checkBox_EnableCustomBorderSize->isChecked());
    projectConfig.setTilesetsHaveCallback(ui->checkBox_OutputCallback->isChecked());
    projectConfig.setTilesetsHaveIsCompressed(ui->checkBox_OutputIsCompressed->isChecked());
    projectConfig.setNewMapElevation(ui->spinBox_Elevation->value());
    projectConfig.setNewMapMetatileId(ui->spinBox_FillMetatile->value());
    projectConfig.setMetatileBehaviorMask(ui->spinBox_BehaviorMask->value());
    projectConfig.setMetatileTerrainTypeMask(ui->spinBox_TerrainTypeMask->value());
    projectConfig.setMetatileEncounterTypeMask(ui->spinBox_EncounterTypeMask->value());
    projectConfig.setMetatileLayerTypeMask(ui->spinBox_LayerTypeMask->value());
    projectConfig.setPrefabFilepath(ui->lineEdit_PrefabsPath->text());

    // Parse border metatile list
    QList<QString> metatileIdStrings = ui->lineEdit_BorderMetatiles->text().split(",");
    QList<uint16_t> metatileIds;
    for (auto s : metatileIdStrings) {
        uint16_t metatileId = s.toUInt(nullptr, 0);
        metatileIds.append(qMin(metatileId, static_cast<uint16_t>(Project::getNumMetatilesTotal() - 1)));
    }
    projectConfig.setNewMapBorderMetatileIds(metatileIds);

    projectConfig.setSaveDisabled(false);
    projectConfig.save();
    userConfig.setSaveDisabled(false);
    userConfig.save();
    this->hasUnsavedChanges = false;

    // Technically, a reload is not required for several of the config settings.
    // For simplicity we prompt the user to reload when any setting is changed regardless.
    this->projectNeedsReload = true;
}

void ProjectSettingsEditor::choosePrefabsFileClicked(bool) {
    QString startPath = this->project->importExportPath;
    QFileInfo fileInfo(ui->lineEdit_PrefabsPath->text());
    if (fileInfo.exists() && fileInfo.isFile() && fileInfo.suffix() == "json") {
        // Current setting is a valid JSON file. Start the file dialog there
        startPath = fileInfo.dir().absolutePath();
    }
    QString filepath = QFileDialog::getOpenFileName(this, "Choose Prefabs File", startPath, "JSON Files (*.json)");
    if (filepath.isEmpty())
        return;
    this->project->setImportExportPath(filepath);
    ui->lineEdit_PrefabsPath->setText(filepath);
    ui->checkBox_PrefabImportPrompted->setChecked(true);
    this->hasUnsavedChanges = true;
}

int ProjectSettingsEditor::prompt(const QString &text, QMessageBox::StandardButton defaultButton) {
    QMessageBox messageBox(this);
    messageBox.setText(text);
    messageBox.setIcon(QMessageBox::Question);
    messageBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No | defaultButton);
    messageBox.setDefaultButton(defaultButton);
    return messageBox.exec();
}

bool ProjectSettingsEditor::promptSaveChanges() {
    if (!this->hasUnsavedChanges)
        return true;

    int result = this->prompt("Settings have been modified, save changes?", QMessageBox::Cancel);
    if (result == QMessageBox::Cancel)
        return false;

    if (result == QMessageBox::Yes)
        this->save();
    else if (result == QMessageBox::No)
        this->hasUnsavedChanges = false; // Discarding changes

    return true;
}

bool ProjectSettingsEditor::promptRestoreDefaults() {
    if (this->refreshing)
        return false;

    const QString versionText = ui->comboBox_BaseGameVersion->currentText();
    if (this->prompt(QString("Restore default config settings for %1?").arg(versionText)) == QMessageBox::No)
        return false;

    // Restore defaults by resetting config in memory, refreshing the UI, then restoring the config.
    // Don't want to save changes until user accepts them.
    ProjectConfig tempProject = projectConfig;
    UserConfig tempUser = userConfig;
    projectConfig.reset(projectConfig.stringToBaseGameVersion(versionText));
    userConfig.reset();
    this->refresh();
    projectConfig = tempProject;
    userConfig = tempUser;

    this->hasUnsavedChanges = true;
    return true;
}

void ProjectSettingsEditor::dialogButtonClicked(QAbstractButton *button) {
    auto buttonRole = ui->buttonBox->buttonRole(button);
    if (buttonRole == QDialogButtonBox::AcceptRole) {
        // "OK" button
        this->save();
        close();
    } else if (buttonRole == QDialogButtonBox::RejectRole) {
        // "Cancel" button
        if (!this->promptSaveChanges())
            return;
        close();
    } else if (buttonRole == QDialogButtonBox::ResetRole) {
        // "Restore Defaults" button
        this->promptRestoreDefaults();
    }
}

void ProjectSettingsEditor::closeEvent(QCloseEvent* event) {
    if (!this->promptSaveChanges()) {
        event->ignore();
        return;
    }

    if (this->projectNeedsReload) {
        if (this->prompt("Settings changed, reload project to apply changes?") == QMessageBox::Yes)
            emit this->reloadProject();
    }

    porymapConfig.setProjectSettingsEditorGeometry(
        this->saveGeometry(),
        this->saveState()
    );
}
