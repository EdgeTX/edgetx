/*
 * Copyright (C) EdgeTX
 *
 * Based on code named
 *   opentx - https://github.com/opentx/opentx
 *   th9x - http://code.google.com/p/th9x
 *   er9x - http://code.google.com/p/er9x
 *   gruvin9x - http://code.google.com/p/gruvin9x
 *
 * License GPLv2: http://www.gnu.org/licenses/gpl-2.0.html
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#pragma once

#include "rawsource.h"
#include "rawswitch.h"

#include <QStandardItemModel>

class GeneralSettings;
class ModelData;
class AdjustmentReference;

class AbstractItemModel: public QStandardItemModel
{
    Q_OBJECT
  public:
    enum ItemModelId {
      IMID_Unknown,
      IMID_RawSource,
      IMID_RawSwitch,
      IMID_Curve,
      IMID_GVarRef,
      IMID_ThrSource,
      IMID_CustomFuncAction,
      IMID_CustomFuncResetParam,
      IMID_TeleSource,
      IMID_CurveRefType,
      IMID_CurveRefFunc,
      IMID_FlexSwitches,
      IMID_ReservedCount,
      IMID_Custom
    };
    Q_ENUM(ItemModelId)

    enum ItemModelDataRoles {
      IMDR_Id = Qt::UserRole,
      IMDR_Type,
      IMDR_Flags,
      IMDR_Available
    };
    Q_ENUM(ItemModelDataRoles)

    enum ItemModelDataGroups {
      IMDG_None     = 0x01,
      IMDG_Negative = 0x02,
      IMDG_Positive = 0x04
    };
    Q_ENUM(ItemModelDataGroups)

    enum ItemModelUpdateEvent {
       IMUE_None               = 0,
       IMUE_SystemRefresh      = 1 << 0,
       IMUE_Channels           = 1 << 1,
       IMUE_Curves             = 1 << 2,
       IMUE_FlightModes        = 1 << 3,
       IMUE_GVars              = 1 << 4,
       IMUE_Inputs             = 1 << 5,
       IMUE_LogicalSwitches    = 1 << 6,
       IMUE_Scripts            = 1 << 7,
       IMUE_TeleSensors        = 1 << 8,
       IMUE_Timers             = 1 << 9,
       IMUE_Modules            = 1 << 10,
       IMUE_FunctionSwitches   = 1 << 11,
       IMUE_All                = IMUE_SystemRefresh | IMUE_Channels | IMUE_Curves | IMUE_FlightModes | IMUE_GVars | IMUE_Inputs |
                                 IMUE_LogicalSwitches | IMUE_Scripts | IMUE_TeleSensors | IMUE_Timers | IMUE_Modules | IMUE_FunctionSwitches
    };
    Q_ENUM(ItemModelUpdateEvent)

    explicit AbstractItemModel(const GeneralSettings * const generalSettings, const ModelData * const modelData,
                               Firmware * firmware, const Boards * const board, const Board::Type boardType) :
      QStandardItemModel(nullptr),
      generalSettings(generalSettings),
      modelData(modelData),
      firmware(firmware),
      board(board),
      boardType(boardType),
      m_id(IMID_Unknown),
      m_name(""),
      m_updateMask(IMUE_None)
      {}

    virtual ~AbstractItemModel() {}

    void setId(int id) { m_id = id; }
    int getId () const { return m_id; }
    bool isReservedModelId() const { return m_id > IMID_Unknown && m_id < IMID_ReservedCount; }
    void setName(QString name) { m_name = name; }
    QString getName() const { return isReservedModelId() ? idToString(m_id) : m_name; }
    void setUpdateMask(const int mask) { m_updateMask = mask; }
    int getUpdateMask() const { return m_updateMask; }

    inline bool doUpdate(const int event) const { return m_updateMask & event; }

    AbstractItemModel * getItemModel(const int id) const;

    static void dumpItemModelContents(AbstractItemModel * itemModel);

  public slots:
    virtual void update(const int event = IMUE_SystemRefresh) = 0;

  protected:
    const GeneralSettings * generalSettings;
    const ModelData * modelData;
    Firmware * firmware;
    const Boards * board;
    const Board::Type boardType;

  private:
    int m_id = IMID_Unknown;
    QString m_name = "";
    int m_updateMask = IMUE_None;

    static QString idToString(const int value);
};

class AbstractStaticItemModel: public AbstractItemModel
{
    Q_OBJECT
  public:
    explicit AbstractStaticItemModel(const GeneralSettings * const generalSettings = nullptr, const ModelData * const modelData = nullptr,
                                     Firmware * firmware = nullptr, const Boards * const board = nullptr,
                                     const Board::Type boardType = Board::BOARD_UNKNOWN) :
      AbstractItemModel(generalSettings, modelData, firmware, board, boardType) {}
    virtual ~AbstractStaticItemModel() {};

    inline void appendToItemList(QString text, int id, bool isAvailable = true, int type = 0, int flags = 0)
                                  { itemList.append(new ListItem(text, id, isAvailable, type, flags)); }

    void loadItemList();

  public slots:
    virtual void update(const int event) override final {}

  protected:
    struct ListItem
    {
      QString text;
      int id;
      bool isAvailable;
      int type;
      int flags;

      ListItem() {}
      ListItem(QString p_text, int p_id, bool p_isAvailable, int p_type, int p_flags) :
               text(p_text), id(p_id), isAvailable(p_isAvailable), type(p_type), flags(p_flags) {}
    };

    QVector<ListItem *> itemList;
};

class AbstractDynamicItemModel: public AbstractItemModel
{
    Q_OBJECT
  public:
    explicit AbstractDynamicItemModel(const GeneralSettings * const generalSettings, const ModelData * const modelData,
                                      Firmware * firmware, const Boards * const board, const Board::Type boardType) :
      AbstractItemModel(generalSettings, modelData, firmware, board, boardType) {}
    virtual ~AbstractDynamicItemModel() {};

  public slots:
    virtual void update(const int event = IMUE_SystemRefresh) {}

  signals:
    void aboutToBeUpdated();
    void updateComplete();
};

class RawSourceItemModel: public AbstractDynamicItemModel
{
    Q_OBJECT
  public:
    explicit RawSourceItemModel(const GeneralSettings * const generalSettings, const ModelData * const modelData,
                                Firmware * firmware, const Boards * const board, const Board::Type boardType);
    virtual ~RawSourceItemModel() {};

  public slots:
    virtual void update(const int event = IMUE_SystemRefresh) override;

  protected:
    virtual void setDynamicItemData(QStandardItem * item, const RawSource & src) const;
    void addItems(const RawSourceType & type, const int group, int count, const int start = 0);
};

class RawSwitchItemModel: public AbstractDynamicItemModel
{
    Q_OBJECT
  public:
    explicit RawSwitchItemModel(const GeneralSettings * const generalSettings, const ModelData * const modelData,
                                Firmware * firmware, const Boards * const board, const Board::Type boardType);
    virtual ~RawSwitchItemModel() {};

  public slots:
    virtual void update(const int event = IMUE_SystemRefresh) override;

  protected:
    virtual void setDynamicItemData(QStandardItem * item, const RawSwitch & rsw) const;
    void addItems(const RawSwitchType & type, int count);
};

class CurveItemModel: public AbstractDynamicItemModel
{
    Q_OBJECT
  public:
    explicit CurveItemModel(const GeneralSettings * const generalSettings, const ModelData * const modelData,
                            Firmware * firmware, const Boards * const board, const Board::Type boardType);
    virtual ~CurveItemModel() {};

  public slots:
    virtual void update(const int event = IMUE_SystemRefresh) override;

  protected:
    virtual void setDynamicItemData(QStandardItem * item, const int value) const;
};

class GVarReferenceItemModel: public AbstractDynamicItemModel
{
    Q_OBJECT
  public:
    explicit GVarReferenceItemModel(const GeneralSettings * const generalSettings, const ModelData * const modelData,
                                    Firmware * firmware, const Boards * const board, const Board::Type boardType);
    virtual ~GVarReferenceItemModel() {};

  public slots:
    virtual void update(const int event = IMUE_SystemRefresh) override;

  protected:
    virtual void setDynamicItemData(QStandardItem * item, const AdjustmentReference & ar) const;
    void addItems(int count);
};

class ThrottleSourceItemModel: public AbstractDynamicItemModel
{
    Q_OBJECT
  public:
    explicit ThrottleSourceItemModel(const GeneralSettings * const generalSettings, const ModelData * const modelData,
                                     Firmware * firmware, const Boards * const board, const Board::Type boardType);
    virtual ~ThrottleSourceItemModel() {};

  public slots:
    virtual void update(const int event = IMUE_SystemRefresh) override;

  protected:
    virtual void setDynamicItemData(QStandardItem * item, const int value) const;
};

class CustomFuncActionItemModel: public AbstractDynamicItemModel
{
    Q_OBJECT
  public:
    explicit CustomFuncActionItemModel(const GeneralSettings * const generalSettings, const ModelData * const modelData,
                                       Firmware * firmware, const Boards * const board, const Board::Type boardType);
    virtual ~CustomFuncActionItemModel() {};

  public slots:
    virtual void update(const int event = IMUE_SystemRefresh) override;

  protected:
    virtual void setDynamicItemData(QStandardItem * item, const int value) const;
};

class CustomFuncResetParamItemModel: public AbstractDynamicItemModel
{
    Q_OBJECT
  public:
    explicit CustomFuncResetParamItemModel(const GeneralSettings * const generalSettings, const ModelData * const modelData,
                                           Firmware * firmware, const Boards * const board, const Board::Type boardType);
    virtual ~CustomFuncResetParamItemModel() {};

  public slots:
    virtual void update(const int event = IMUE_SystemRefresh) override;

  protected:
    virtual void setDynamicItemData(QStandardItem * item, const int value) const;
};

class TelemetrySourceItemModel: public AbstractDynamicItemModel
{
    Q_OBJECT
  public:
    explicit TelemetrySourceItemModel(const GeneralSettings * const generalSettings, const ModelData * const modelData,
                                      Firmware * firmware, const Boards * const board, const Board::Type boardType);
    virtual ~TelemetrySourceItemModel() {};

  public slots:
    virtual void update(const int event = IMUE_SystemRefresh) override;

  protected:
    virtual void setDynamicItemData(QStandardItem * item, const int value) const;
};

class CurveRefTypeItemModel : public AbstractStaticItemModel
{
    Q_OBJECT
  public:
    explicit CurveRefTypeItemModel(const GeneralSettings * const generalSettings, const ModelData * const modelData,
                                   Firmware * firmware, const Boards * const board, const Board::Type boardType);
    virtual ~CurveRefTypeItemModel() {};
};

class CurveRefFuncItemModel : public AbstractStaticItemModel
{
    Q_OBJECT
  public:
    explicit CurveRefFuncItemModel(const GeneralSettings * const generalSettings, const ModelData * const modelData,
                                   Firmware * firmware, const Boards * const board, const Board::Type boardType);
    virtual ~CurveRefFuncItemModel() {};
};

class PrecisionItemModel : public AbstractStaticItemModel
{
    Q_OBJECT
  public:
    explicit PrecisionItemModel(const int minDecimals, const int maxDecimals, const QString suffix = "", const bool placeholders = false);
    virtual ~PrecisionItemModel() {};
};

class FlexSwitchesItemModel: public AbstractDynamicItemModel
{
    Q_OBJECT
  public:
    explicit FlexSwitchesItemModel(const GeneralSettings * const generalSettings, const ModelData * const modelData,
                                      Firmware * firmware, const Boards * const board, const Board::Type boardType);
    virtual ~FlexSwitchesItemModel() {};

  public slots:
    virtual void update(const int event = IMUE_SystemRefresh) override;

  protected:
    virtual void setDynamicItemData(QStandardItem * item, const int value) const;
};


//
//  CompoundItemModelFactory
//

class CompoundItemModelFactory
{
  public:
    CompoundItemModelFactory(const GeneralSettings * const generalSettings, const ModelData * const modelData);
    virtual ~CompoundItemModelFactory();

    void addItemModel(const int id);
    int registerItemModel(AbstractItemModel * itemModel);
    void unregisterItemModels();
    void unregisterItemModel(const int id);
    bool isItemModelRegistered(const int id) const;
    AbstractItemModel * getItemModel(const int id) const;
    AbstractItemModel * getItemModel(const QString name) const;
    void update(const int event = AbstractItemModel::IMUE_SystemRefresh);
    void dumpAllItemModelContents() const;

  protected:
    const GeneralSettings * generalSettings;
    const ModelData * modelData;
    Firmware * firmware;
    Boards * board;
    Board::Type boardType;
    QVector<AbstractItemModel *> registeredItemModels;

  private:
    void setSourceId(AbstractItemModel * itemModel);
};
