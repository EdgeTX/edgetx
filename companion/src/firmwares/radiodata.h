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
    QStringList labels;
    std::vector<ModelData> models;

    void convert(RadioDataConversionState & cstate);

    void addLabel(QString label);
    void deleteLabel(QString label);
    bool renameLabel(QString from, QString to);
    bool renameLabel(int index, QString to);
    bool addLabelToModel(int modelIndex, QString label);
    bool removeLabelFromModel(int modelIndex, QString label);

    void setCurrentModel(unsigned int index);
    void fixModelFilenames();
    QString getNextModelFilename();

    // leave here until all calls repointed
    static QString getElementName(const QString & prefix, unsigned int index, const char * name = 0, bool padding = false)
      { return DataHelpers::getElementName(prefix, index, name, padding); }

  protected:
    void fixModelFilename(unsigned int index);
};
