/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef SP_PREPROC_XYCE_H
#define SP_PREPROC_XYCE_H

#include "components/component.h"

class XycePreProc : public Component  {
public:
  XycePreProc();
  ~XycePreProc();
  Component* newOne();
  static Element* info(QString&, char* &, bool getNewOne=false);

protected:
  QString spice_netlist(spicecompat::SpiceDialect dialect = spicecompat::SPICEDefault);
  // Qt::GlobalColor color() const override { return Qt::darkGreen; }
};

#endif
