/*
 * Copyright (C) OpenTX
 *
 * Based on code named
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

#ifndef _EEPROMIMPORTEXPORT_H_
#define _EEPROMIMPORTEXPORT_H_

#include "customdebug.h"
#include <QtCore>
#include <QBitArray>
#include <utility>

class DataField {
  Q_DECLARE_TR_FUNCTIONS(DataField)

  public:
    explicit DataField(DataField * parent, const char * name=""):
      parent(parent),
      name(name)
    {
    }

    virtual ~DataField()
    = default;

    virtual const QString & getName()
    {
      return name;
    }

    virtual unsigned int size() = 0; // size in bits
    virtual void ExportBits(QBitArray & output) = 0;
    virtual void ImportBits(const QBitArray & input) = 0;

    static QBitArray bytesToBits(QByteArray bytes)
    {
      QBitArray bits(bytes.count()*8);
      // Convert from QByteArray to QBitArray
      for (int i=0; i<bytes.count(); ++i)
        for (int b=0; b<8; ++b)
          bits.setBit(i*8+b, bytes.at(i)&(1<<b));
      return bits;
    }

    static QByteArray bitsToBytes(QBitArray bits, int offset=0)
    {
      QByteArray bytes;
      bytes.resize((offset+bits.count()+7)/8);
      bytes.fill(0);
      // Convert from QBitArray to QByteArray
      for (int b=0; b<bits.count(); ++b)
        bytes[(b+offset)/8] = ( bytes.at((b+offset)/8) | ((bits[b]?1:0)<<((b+offset)%8)));
      return bytes;
    }

    int Export(QByteArray & output)
    {
      QBitArray result;
      ExportBits(result);
      output = bitsToBytes(result);
      return 0;
    }

    int Import(const QByteArray & input)
    {
      QBitArray bits = bytesToBits(input);
      if (unsigned(bits.size()) < size()) {
        qDebug() << QString("Error importing %1: size too small %2 bits / %3 bits").arg(getName()).arg(bits.size()).arg(size());
        return -1;
      }
      ImportBits(bits);
      return 0;
    }

    virtual int dump(int level=0, int offset=0)
    {
      QBitArray bits;
      ExportBits(bits);
      QByteArray bytes = bitsToBytes(bits);
      int result = (offset+bits.count()) % 8;
      for (int i=0; i<level; i++) printf("  ");
      if (bits.count() % 8 == 0)
        printf("%s (%dbytes) ", getName().toLatin1().constData(), bytes.count());
      else
        printf("%s (%dbits) ", getName().toLatin1().constData(), bits.count());
      for (int i=0; i<bytes.count(); i++) {
        unsigned char c = bytes[i];
        if ((i==0 && offset) || (i==bytes.count()-1 && result!=0))
          printf("(%02x) ", c);
        else
          printf("%02x ", c);
      }
      printf("\n"); fflush(stdout);
      return result;
    }

  protected:
    virtual void setError(const QString & error)
    {
      if (parent) {
        parent->setError(error);
      }
      else {
        qWarning() << qPrintable(error);
      }
    }

    DataField * parent;
    QString name;
};

//class ProxyField: public DataField {
//  public:
//    explicit ProxyField(DataField * parent):
//      DataField(parent, "Proxy")
//    {
//    }
//
//    virtual DataField * getField() = 0;
//
//};

template<class container, int N>
class BaseUnsignedField: public DataField {
  public:
    BaseUnsignedField(DataField * parent, container & field):
      DataField(parent, "Unsigned"),
      field(field),
      min(0),
      max(std::numeric_limits<container>::max())
    {
    }

    BaseUnsignedField(DataField * parent, container & field, const char * name):
      DataField(parent, name),
      field(field),
      min(0),
      max(std::numeric_limits<container>::max())
    {
    }

    BaseUnsignedField(DataField * parent, container & field, container min, container max, const char * name="Unsigned"):
      DataField(parent, name),
      field(field),
      min(min),
      max(max)
    {
    }

    BaseUnsignedField() = delete;

    void ExportBits(QBitArray & output) override
    {
      container value = field;
      if (value > max) value = max;
      if (value < min) value = min;

      output.resize(N);
      for (int i=0; i<N; i++) {
        if (value & ((container)1<<i)) {
          output.setBit(i);
        }
      }
    }

    void ImportBits(const QBitArray & input) override
    {
      field = 0;
      for (int i=0; i<N; i++) {
        if (input[i])
          field |= ((container)1<<i);
      }
      qCDebug(eepromImport) << QString("\timported %1<%2>: 0x%3(%4)").arg(name).arg(N).arg(field, 0, 16).arg(field);
    }

    unsigned int size() override
    {
      return N;
    }

  protected:
    container & field;
    container min;
    container max;
};

template <int N>
class UnsignedField : public BaseUnsignedField<unsigned int, N>
{
  public:
    UnsignedField(DataField * parent, unsigned int & field):
      BaseUnsignedField<unsigned int, N>(parent, field)
    {
    }

    UnsignedField(DataField * parent, unsigned int & field, const char *name):
      BaseUnsignedField<unsigned int, N>(parent, field, name)
    {
    }

    UnsignedField(DataField * parent, unsigned int & field, unsigned int min, unsigned int max, const char *name="Unsigned"):
      BaseUnsignedField<unsigned int, N>(parent, field, min, max, name)
    {
    }
};

template<int N>
class BoolField: public DataField {
  public:
    explicit BoolField(DataField * parent, bool & field):
      DataField(parent, "Bool"),
      field(field)
    {
    }

    BoolField() = delete;

    void ExportBits(QBitArray & output) override
    {
      output.resize(N);
      if (field) {
        output.setBit(0);
      }
    }

    void ImportBits(const QBitArray & input) override
    {
      field = input[0];
      qCDebug(eepromImport) << QString("\timported %1<%2>: 0x%3(%4)").arg(name).arg(N).arg(field, 0, 16).arg(field);
    }

    unsigned int size() override
    {
      return N;
    }

  protected:
    bool & field;
};

template<int N>
class SignedField: public DataField {
  public:
    SignedField(DataField * parent, int & field):
      DataField(parent, "Signed"),
      field(field)
    {
    }

    SignedField(DataField * parent, int & field, const char *name):
      DataField(parent, name),
      field(field)
    {
    }

    SignedField(DataField * parent, int & field, int min, int max, const char *name="Signed"):
      DataField(parent, name),
      field(field),
      min(min),
      max(max)
    {
    }

    void ExportBits(QBitArray & output) override
    {
      int value = field;
      if (value > max) value = max;
      if (value < min) value = min;

      output.resize(N);
      for (int i=0; i<N; i++) {
        if (((unsigned int)value) & (1<<i))
          output.setBit(i);
      }
    }

    void ImportBits(const QBitArray & input) override
    {
      unsigned int value = 0;
      for (int i=0; i<N; i++) {
        if (input[i])
          value |= (1<<i);
      }

      if (input[N-1]) {
        for (unsigned int i=N; i<8*sizeof(int); i++) {
          value |= (1<<i);
        }
      }

      field = (int)value;
      qCDebug(eepromImport) << QString("\timported %1<%2>: 0x%3(%4)").arg(name).arg(N).arg(field, 0, 16).arg(field);
    }

    unsigned int size() override
    {
      return N;
    }

  protected:
    int & field;
    int min = INT_MIN;
    int max = INT_MAX;
};

template<int N>
class SpareBitsField: public UnsignedField<N> {
  public:
    explicit SpareBitsField(DataField * parent):
      UnsignedField<N>(parent, spare, 0, 0, "Spare"),
      spare(0)
    {
    }

  protected:
    unsigned int spare;
};

template<int N>
class CharField: public DataField {
  public:
    CharField(DataField * parent, char * field, bool truncate=true, const char * name="Char"):
      DataField(parent, name),
      field(field),
      truncate(truncate)
    {
    }

    void ExportBits(QBitArray & output) override
    {
      output.resize(N*8);
      int b = 0;
      int len = truncate ? strlen(field) : N;
      for (int i=0; i<N; i++) {
        int idx = (i>=len ? 0 : field[i]);
        for (int j=0; j<8; j++, b++) {
          if (idx & (1<<j))
            output.setBit(b);
        }
      }
    }

    void ImportBits(const QBitArray & input) override
    {
      unsigned int b = 0;
      for (int i=0; i<N; i++) {
        int8_t idx = 0;
        for (int j=0; j<8; j++) {
          if (input[b++])
            idx |= (1<<j);
        }
        field[i] = idx;
      }
      qCDebug(eepromImport) << QString("\timported %1<%2>: '%3'").arg(name).arg(N).arg(field);
    }

    unsigned int size() override
    {
      return 8 * N;
    }

  protected:
    char * field;
    bool truncate;
};

static const char specialCharsTab[] = "_-.,";
static inline int8_t char2zchar(char c)
{
  if (c==' ') return 0;
  if (c>='A' && c<='Z') return 1+c-'A';
  if (c>='a' && c<='z') return -1-c+'a';
  if (c>='0' && c<='9') return 27+c-'0';
  for (int8_t i=0;;i++) {
    char cc = specialCharsTab[i];
    if (cc==0) return 0;
    if (cc==c) return 37+i;
  }
}

#define ZCHAR_MAX 40
static inline char zchar2char(int8_t idx)
{
  if (idx == 0) return ' ';
  if (idx < 0) {
    if (idx > -27) return 'a' - idx - 1;
    idx = -idx;
  }
  if (idx < 27) return 'A' + idx - 1;
  if (idx < 37) return '0' + idx - 27;
  if (idx <= ZCHAR_MAX) return specialCharsTab[idx-37];
  return ' ';
}

template<int N>
class ZCharField: public DataField {
  public:
    ZCharField(DataField * parent, char * field, const char * name = "ZChar"):
      DataField(parent, name),
      field(field)
    {
    }

    void ExportBits(QBitArray & output) override
    {
      output.resize(N*8);
      int b = 0;
      int len = strlen(field);
      for (int i=0; i<N; i++) {
        int idx = i>=len ? 0 : char2zchar(field[i]);
        for (int j=0; j<8; j++, b++) {
          if (idx & (1<<j))
            output.setBit(b);
        }
      }
    }

    void ImportBits(const QBitArray & input) override
    {
      unsigned int b = 0;
      for (int i=0; i<N; i++) {
        int8_t idx = 0;
        for (int j=0; j<8; j++) {
          if (input[b++])
            idx |= (1<<j);
        }
        field[i] = zchar2char(idx);
      }

      field[N] = '\0';
      for (int i=N-1; i>=0; i--) {
        if (field[i] == ' ')
          field[i] = '\0';
        else
          break;
      }
      qCDebug(eepromImport) << QString("\timported %1<%2>: '%3'").arg(name).arg(N).arg(field);
    }

    unsigned int size() override
    {
      return 8*N;
    }

  protected:
    char * field;
};

class StructField: public DataField {
  public:
    explicit StructField(DataField * parent, const char * name = "Struct"):
      DataField(parent, name)
    {
    }

    ~StructField() override {
      foreach(DataField * field, fields) {
        delete field;
      }
    }

    inline void Append(DataField * field) {
      //qCDebug(eepromImport) << QString("StructField(%1) appending field: %2").arg(name).arg(field->getName());
      fields.append(field);
    }

    void ExportBits(QBitArray & output) override
    {
      int offset = 0;
      output.resize(size());
      foreach(DataField *field, fields) {
        QBitArray bits;
        field->ExportBits(bits);
        for (int i=0; i<bits.size(); i++)
          output[offset++] = bits[i];
      }
    }

    void ImportBits(const QBitArray & input) override
    {
      qCDebug(eepromImport) << QString("\timporting %1[%2]:").arg(name).arg(fields.size());
      int offset = 0;
      foreach(DataField *field, fields) {
        unsigned int size = field->size();
        QBitArray bits(size);
        for (unsigned int i=0; i<size; i++) {
          bits[i] = input[offset++];
        }
        field->ImportBits(bits);
      }
    }

    unsigned int size() override
    {
      unsigned int result = 0;
      foreach(DataField *field, fields) {
        result += field->size();
      }
      return result;
    }

    int dump(int level=0, int offset=0) override
    {
      for (int i=0; i<level; i++) printf("  ");
      printf("%s (%d bytes)\n", getName().toLatin1().constData(), size()/8);
      foreach(DataField *field, fields) {
        offset = field->dump(level+1, offset);
      }
      return offset;
    }

  protected:
    QList<DataField *> fields;
};

class TransformedField: public DataField {
  public:
    TransformedField(DataField * parent, DataField & field):
      DataField(parent),
      field(field)
    {
    }

    ~TransformedField() override
    = default;

    void ExportBits(QBitArray & output) override
    {
      beforeExport();
      field.ExportBits(output);
    }

    void ImportBits(const QBitArray & input) override
    {
      qCDebug(eepromImport) << QString("\timporting TransformedField %1:").arg(field.getName());
      field.ImportBits(input);
      afterImport();
    }


    const QString & getName() override
    {
      return field.getName();
    }

    unsigned int size() override
    {
      return field.size();
    }

    virtual void beforeExport() = 0;

    virtual void afterImport() = 0;

    int dump(int level=0, int offset=0) override
    {
      beforeExport();
      return field.dump(level, offset);
    }

  protected:
    DataField & field;
};

template<class selectorT>
class UnionField: public DataField {
  public:

    class UnionMember {
      public:
        virtual ~UnionMember() = default;
        virtual bool select(const selectorT& attr) const = 0;
        virtual DataField* getField() = 0;
    };

    class TransformedMember: public UnionMember, public TransformedField {
      public:
        TransformedMember(DataField* parent, DataField& field):
          TransformedField(parent, field)
        {
        }

        virtual DataField* getField()
        {
          return this;
        }
    };

    UnionField(DataField * parent, selectorT& selectField, const char * name="Union"):
      DataField(parent, name), selectField(selectField), maxSize(0)
    {
    }

    ~UnionField() override {
      foreach(UnionMember *member, members) {
        delete member;
      }
    }

    inline void Append(UnionMember * member) {
      members.append(member);
      if (member->getField()->size() > maxSize)
        maxSize = member->getField()->size();
    }

    void ExportBits(QBitArray & output) override
    {
      foreach(UnionMember *member, members) {
        if (member->select(selectField)) {
          member->getField()->ExportBits(output);
          break;
        }
      }
      output.resize(maxSize);
    }

    void ImportBits(const QBitArray & input) override
    {
      foreach(UnionMember *member, members) {
        if (member->select(selectField)) {
          member->getField()->ImportBits(input);
          break;
        }
      }
    }

    unsigned int size() override
    {
      return maxSize;
    }

  protected:
    selectorT&           selectField;
    QList<UnionMember *> members;
    unsigned int         maxSize;
};

class ConversionTable {

  public:
    bool exportValue(const int before, int &after)
    {
      after = 0;

      for (auto & tuple : exportTable) {
        if (before == tuple.a) {
          after = tuple.b;
          return true;
        }
      }

      return false;
    }

    bool importValue(const int before, int &after)
    {
      after = 0;

      for (auto & tuple : importTable) {
        if (before == tuple.b) {
          after = tuple.a;
          return true;
        }
      }

      return false;
    }

  protected:

    class ConversionTuple {
      public:
        ConversionTuple(const int a, const int b):
          a(a),
          b(b)
        {
        }

        int a;
        int b;
    };

    void addConversion(const int a, const int b)
    {
      ConversionTuple conversion(a, b);
      importTable.push_back(conversion);
      exportTable.push_back(conversion);
    }

//    void addImportConversion(const int a, const int b)
//    {
//      importTable.push_back(ConversionTuple(a, b));
//    }
//
//    void addExportConversion(const int a, const int b)
//    {
//      exportTable.push_back(ConversionTuple(a, b));
//    }

    std::list<ConversionTuple> importTable;
    std::list<ConversionTuple> exportTable;
};

template<class T>
class ConversionField: public TransformedField {
  public:
    ConversionField(DataField * parent, int & field, ConversionTable *table, const char *name, QString error = ""):
      TransformedField(parent, internalField),
      internalField(this, _field, name),
      field(field),
      table(table),
      error(std::move(error))
    {
    }

    ConversionField(DataField * parent, unsigned int & field, ConversionTable *table, const char *name, QString error = ""):
      TransformedField(parent, internalField),
      internalField(this, (unsigned int &)_field, name),
      field((int &)field),
      table(table),
      error(std::move(error))
    {
    }

    ConversionField(DataField * parent, int & field, int (*exportFunc)(int), int (*importFunc)(int)):
      TransformedField(parent, internalField),
      internalField(this, _field),
      field(field),
      exportFunc(exportFunc),
      importFunc(importFunc)
    {
    }

    ConversionField(DataField * parent, int & field, int shift, int scale=0, int min=INT_MIN, int max=INT_MAX, const char *name = "Signed shifted"):
      TransformedField(parent, internalField),
      internalField(this, _field, name),
      field(field),
      shift(shift),
      scale(scale),
      min(min),
      max(max)
    {
    }

    ConversionField(DataField * parent, unsigned int & field, int shift, int scale=0):
      TransformedField(parent, internalField),
      internalField(this, (unsigned int &)_field),
      field((int &)field),
      shift(shift),
      scale(scale)
    {
    }

    void beforeExport() override
    {
       _field = scale ? field / scale : field;

      if (table) {
        if (!table->exportValue(_field, _field)) {
          setError(error.isEmpty() ? DataField::tr("Conversion error on field %1").arg(name) : error);
        }
        return;
      }

      if (shift) {
        if (_field < min)
          _field = min + shift;
        else if (_field > max)
          _field = max + shift;
        else
          _field += shift;
      }

      if (exportFunc) {
        _field = exportFunc(_field);
      }
    }

    void afterImport() override
    {
      field = _field;

      if (table) {
        if (table->importValue(field, field))
          return;
      }

      if (shift) {
        field -= shift;
      }

      if (importFunc) {
        field = importFunc(field);
      }

      if (scale) {
        field *= scale;
      }

      qCDebug(eepromImport) << QString("\timported ConversionField<%1>:").arg(internalField.getName()) << QString(" before: %1, after: %2").arg(_field).arg(field);
    }

  protected:
    T internalField;
    int & field;
    int _field = 0;
    ConversionTable * table = nullptr;
    int shift = 0;
    int scale = 0;
    int min = INT_MIN;
    int max = INT_MAX;
    int (*exportFunc)(int) = nullptr;
    int (*importFunc)(int) = nullptr;
    const QString error = "";
};

#endif // _EEPROMIMPORTEXPORT_H_
