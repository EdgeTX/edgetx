#pragma once

#include "generalsettings.h"
#include "modeldata.h"

#include "datahelpers.h"  // required for getElementName

#include <QtCore>

class RadioDataConversionState;

class CategoryData {
  public:
    CategoryData(const char * name) {
      memset(this->name, 0, sizeof(CategoryData::name));
      strncpy(this->name, name, sizeof(CategoryData::name)-1);
    }
    char name[15+1];
};

class RadioData {
  Q_DECLARE_TR_FUNCTIONS(RadioData)

  public:
    RadioData();

    GeneralSettings generalSettings;
    std::vector<QString> labels;
    std::vector<ModelData> models;

    void convert(RadioDataConversionState & cstate);

    void setCurrentModel(unsigned int index);
    void fixModelFilenames();
    QString getNextModelFilename();

    // leave here until all calls repointed
    static QString getElementName(const QString & prefix, unsigned int index, const char * name = 0, bool padding = false)
      { return DataHelpers::getElementName(prefix, index, name, padding); }

  protected:
    void fixModelFilename(unsigned int index);
};
