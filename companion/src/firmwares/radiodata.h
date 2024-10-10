#pragma once

#include "generalsettings.h"
#include "modeldata.h"
#include "labels.h"

#include "datahelpers.h"  // required for getElementName

#include <QtCore>

// identiying names of static abstract item models
constexpr char AIM_RADIO_MODEL_SORT_ORDER[]        {"radio.modelsortorder"};

constexpr int LABEL_LENGTH = 16;

class RadioDataConversionState;
class AbstractStaticItemModel;

enum MODEL_SORT_ORDER{
  MSO_NO_SORT,
  MSO_NAME_ASC,
  MSO_NAME_DES,
  MSO_DATE_ASC,
  MSO_DATE_DES,
  MSO_SORT_COUNT
};

class RadioData {
  Q_DECLARE_TR_FUNCTIONS(RadioData)

  public:
    RadioData();

    GeneralSettings generalSettings;

    struct LabelData {
      QString name;
      bool selected;
    };

    typedef QVector<LabelData> ModelLabels;

    ModelLabels labels;

    int sortOrder;
    std::vector<ModelData> models;

    void convert(RadioDataConversionState & cstate);

    void addLabel(QString label);
    bool deleteLabel(QString label);
    bool deleteLabel(int index);
    bool renameLabel(QString from, QString to);
    bool renameLabel(int index, QString to);
    void swapLabel(int indFrom, int indTo);
    bool addLabelToModel(int index, QString label);
    bool removeLabelFromModel(int index, QString label);
    void addLabelsFromModels();
    int indexOfLabel(QString & label) const;

    static QStringList fromCSV(const QString &csv);
    static QString toCSV(QStringList lbls);
    static QString escapeCSV(QString str);
    static QString unEscapeCSV(QString str);

    void setCurrentModel(unsigned int index);
    void fixModelFilenames();
    QString getNextModelFilename();

    static QString modelSortOrderToString(int value);
    static AbstractStaticItemModel * modelSortOrderItemModel();

    // leave here until all calls repointed
    static QString getElementName(const QString & prefix, unsigned int index, const char * name = 0, bool padding = false)
      { return DataHelpers::getElementName(prefix, index, name, padding); }

  protected:
    void fixModelFilename(unsigned int index);

};
