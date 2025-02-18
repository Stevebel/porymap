#include "encountertabledelegates.h"
#include "encountertablemodel.h"

#include <QSpinBox>
#include "project.h"
#include "noscrollcombobox.h"



SpeciesComboDelegate::SpeciesComboDelegate(Project *project, QObject *parent) : QStyledItemDelegate(parent) {
    this->project = project;
}

void SpeciesComboDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const {
    QString species = index.data(Qt::DisplayRole).toString();

    QPixmap pm;
    if (!QPixmapCache::find(species, &pm)) {
        QImage img(this->project->speciesToIconPath.value(species));
        img.setColor(0, qRgba(0, 0, 0, 0));
        pm = QPixmap::fromImage(img);
        QPixmapCache::insert(species, pm);
    }
    QPixmap monIcon = pm.copy(0, 0, 32, 32);

    painter->drawText(QRect(option.rect.topLeft() + QPoint(36, 0), option.rect.bottomRight()), Qt::AlignLeft | Qt::AlignVCenter, species);
    painter->drawPixmap(QRect(option.rect.topLeft(), QSize(32, 32)), monIcon, monIcon.rect());
}

QWidget *SpeciesComboDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &, const QModelIndex &) const {
    NoScrollComboBox *editor = new NoScrollComboBox(parent);
    editor->setFrame(false);
    editor->addItems(this->project->speciesToIconPath.keys());
    return editor;
}

void SpeciesComboDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const {
    QString species = index.data(Qt::EditRole).toString();
    NoScrollComboBox *combo = static_cast<NoScrollComboBox *>(editor);
    combo->setCurrentIndex(combo->findText(species));
}

void SpeciesComboDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const {
    NoScrollComboBox *combo = static_cast<NoScrollComboBox *>(editor);
    QString species = combo->currentText();
    model->setData(index, species, Qt::EditRole);
}

void SpeciesComboDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &) const {
    editor->setGeometry(option.rect);
}



SpinBoxDelegate::SpinBoxDelegate(Project *project, QObject *parent) : QStyledItemDelegate(parent) {
    this->project = project;
}

QWidget *SpinBoxDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &, const QModelIndex &index) const {
    int col = index.column();
    if (col == EncounterTableModel::ColumnType::EncounterChanceDay || col == EncounterTableModel::ColumnType::EncounterChanceNight) {
        QDoubleSpinBox *editor2 = new QDoubleSpinBox(parent);
        editor2->setFrame(false);
        editor2->setMinimum(0);
        editor2->setMaximum(100);
        editor2->setSingleStep(0.001);
        return editor2;
    }

    QSpinBox *editor = new QSpinBox(parent);
    editor->setFrame(false);

    if (col == EncounterTableModel::ColumnType::MinLevel || col == EncounterTableModel::ColumnType::MaxLevel) {
        editor->setMinimum(this->project->miscConstants.value("min_level_define").toInt());
        editor->setMaximum(this->project->miscConstants.value("max_level_define").toInt());
    }
    else if (col == EncounterTableModel::ColumnType::EncounterRate) {
        editor->setMinimum(0);
        editor->setMaximum(180);
    }

    return editor;
}

void SpinBoxDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const {
    int col = index.column();
    if (col == EncounterTableModel::ColumnType::EncounterChanceDay || col == EncounterTableModel::ColumnType::EncounterChanceNight) {
        QDoubleSpinBox *doubleSpinBox = static_cast<QDoubleSpinBox *>(editor);
        doubleSpinBox->setValue(index.model()->data(index, Qt::EditRole).toDouble());
        return;
    }

    int value = index.model()->data(index, Qt::EditRole).toInt();

    QSpinBox *spinBox = static_cast<QSpinBox *>(editor);
    spinBox->setValue(value);
}

void SpinBoxDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const {
    int col = index.column();
    if (col == EncounterTableModel::ColumnType::EncounterChanceDay || col == EncounterTableModel::ColumnType::EncounterChanceNight) {
        QDoubleSpinBox *doubleSpinBox = static_cast<QDoubleSpinBox *>(editor);
        doubleSpinBox->interpretText();
        double value = doubleSpinBox->value();
        model->setData(index, value, Qt::EditRole);
        return;
    }

    QSpinBox *spinBox = static_cast<QSpinBox*>(editor);
    spinBox->interpretText();
    int value = spinBox->value();
    model->setData(index, value, Qt::EditRole);
}

void SpinBoxDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &) const {
    editor->setGeometry(option.rect);
}
