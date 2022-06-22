#ifndef LABELS_H
#define LABELS_H

#include <QObject>

/** @brief Keeps track of all model labels
 *
 */

class Labels : public QObject
{
  Q_OBJECT
public:
  Labels();
  int getLabelCount() {return 4;}
signals:
  void labelsChanged(int lblcount);

};

#endif // LABELS_H
