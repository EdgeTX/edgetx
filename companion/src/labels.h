#pragma once

#include <QObject>
#include <QList>
#include <QAbstractItemModel>
#include <QItemSelectionModel>
#include <QValidator>
#include <QLineEdit>
#include <QStyledItemDelegate>

#include "radiodata.h"

class RadioData;

typedef struct {
  QString label;
  int radioLabelIndex;
} LabelItem;

class LabelsModel : public QAbstractItemModel
{
  Q_OBJECT

public:
  LabelsModel(QItemSelectionModel *selectionModel, RadioData *radioData, QObject *parent = nullptr);
  ~LabelsModel();
  Qt::ItemFlags flags(const QModelIndex &index) const override;
  bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
  QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
  QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
  QModelIndex parent(const QModelIndex &index) const override;
  int rowCount(const QModelIndex &parent = QModelIndex()) const override;
  int columnCount(const QModelIndex &parent = QModelIndex()) const override;
  QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
  bool insertRows(int row, int count, const QModelIndex &parent) override;
  bool removeRows(int row, int count, const QModelIndex &parent) override;

public slots:
  void buildLabelsList();

private slots:
  void modelsSelectionChanged();

signals:
  void modelChanged(int index);
  void labelsFault(QString msg);

private:
  QItemSelectionModel *modelsSelection;
  RadioData *radioData;
  int selectedModel;
  QList<QModelIndex> modelIndices;
  QList<LabelItem> labels;
};

class LabelValidator : public QValidator
{
  Q_OBJECT
public:
  QValidator::State validate(QString &label, int &pos) const;
 void fixup(QString &input) const;
};

class LabelEditTextDelegate : public QStyledItemDelegate
{
  Q_OBJECT

public:
  LabelEditTextDelegate(QObject *parent = nullptr) : QStyledItemDelegate(parent) {}

  QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option,
                        const QModelIndex &index) const
  {
    QLineEdit *editor = new QLineEdit(parent);
    editor->setValidator(new LabelValidator);
    return editor;
  }
};
