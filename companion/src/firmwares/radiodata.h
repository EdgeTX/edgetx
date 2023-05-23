#pragma once

#include "generalsettings.h"
#include "modeldata.h"
#include "labels.h"

#include "datahelpers.h"  // required for getElementName

#include <QtCore>

class RadioDataConversionState;

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

    // leave here until all calls repointed
    static QString getElementName(const QString & prefix, unsigned int index, const char * name = 0, bool padding = false)
      { return DataHelpers::getElementName(prefix, index, name, padding); }

  protected:
    void fixModelFilename(unsigned int index);

};
