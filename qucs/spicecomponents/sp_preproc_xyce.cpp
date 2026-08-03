/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "sp_preproc_xyce.h"
#include "main.h"
#include "misc.h"
#include "extsimkernels/spicecompat.h"

#include <QFontInfo>
#include <QFontMetrics>

XycePreProc::XycePreProc()
{
  
  isEquation = false;
  Type = isComponent; // Analogue and digital component.
  Description = QObject::tr("Xyce Preprocessor");
  Simulator = spicecompat::simXyce;

  QFont f = QucsSettings.font;
  f.setWeight(QFont::Light);
  f.setPointSizeF(12.0);
  QFontMetrics  metrics(f, 0);  // use the the screen-compatible metric
  QSize r = metrics.size(0, QObject::tr("Xyce Preprocessor"));
  int xb = r.width()  >> 1;
  int yb = r.height() >> 1;

  Lines.append(new qucs::Line(-xb, -yb, -xb,  yb,QPen(Qt::darkBlue,2)));
  Lines.append(new qucs::Line(-xb,  yb,  xb+3,yb,QPen(Qt::darkBlue,2)));
  Texts.append(new Text(-xb+4,  -yb-3, QObject::tr("Xyce Preprocessor"),
      QColor(0,0,0), QFontInfo(f).pixelSize()));

  x1 = -xb-3;  y1 = -yb-5;
  x2 =  xb+8;  y2 =  yb+3;

  tx = x1+4;
  ty = y2+4;
  Model = "XycePreProc";
  Name  = "XycePreProc";
  SpiceModel = ".PREPROCESS";

  Props.append(new Property("ReplaceGround", "no", false,
                            QObject::tr("Replace nodes 'GND', 'GND!', 'GROUND' or any capital/lowercase variant thereof as synonyms for node 0")+" [yes, no]"));
  Props.append(new Property("RemoveUnused", "", false,
                            QObject::tr("Comma seperated list of components to remove if they are connected to the same node (C,D,I,L,M,Q,R,V are valid options).")));
  Props.append(new Property("AddResistorsOneTerminal", "0 Ohm", false,
                            QObject::tr("Value of resistor to put between nodes with a singular device node and ground.")));
  Props.append(new Property("AddResistorsNoDCPath", "0 Ohm", false,
                            QObject::tr("Value of resistor to put between nodes with no DC path and ground.")));
}

XycePreProc::~XycePreProc()
{
}

Component* XycePreProc::newOne()
{
  return new XycePreProc();
}

Element* XycePreProc::info(QString& Name, char* &BitmapFile, bool getNewOne)
{
  Name = QObject::tr("Xyce Preprocessor");
  BitmapFile = (char *) "sp_preproc_xyce";

  if(getNewOne)  return new XycePreProc();
  return 0;
}

QString XycePreProc::spice_netlist(spicecompat::SpiceDialect dialect /* = spicecompat::SPICEDefault */)
{
    if (dialect != spicecompat::SPICEXyce) {
      return {};
    }

    QString s;
    const bool replaceGround = getProperty("ReplaceGround")->Value == "yes";
    const QString removeUnused = getProperty("RemoveUnused")->Value;
    const QString resOneTerm = spicecompat::normalize_value(getProperty("AddResistorsOneTerminal")->Value);
    const QString resDC = spicecompat::normalize_value(getProperty("AddResistorsNoDCPath")->Value);

    // REPLACEGROUND
    if (replaceGround) {
      s += QStringLiteral("%1 REPLACEGROUND TRUE\n").arg(SpiceModel);
    }

    // REMOVEUNUSED
    if (!removeUnused.isEmpty()) {
      static const QSet<QString> validElements{
        "C", // Capacitor
        "D", // Diode
        "I", // Independendent Current Source
        "L", // Inductor
        "M", // MOSFET
        "Q", // BJT
        "R", // Resistor
        "V"  // Independent voltage source
      };

      QStringList removeUnusedList;
      QSet<QString> seen;

      // Sanitize that we only keep valid elements
      for (const QString rawElement : removeUnused.split(",", Qt::SkipEmptyParts)) {
        QString element = rawElement.trimmed();

        if (!validElements.contains(element)) {
          qWarning() << QStringLiteral("Xyce Preprocess: '%1' is not a valid element for REMOVEUNUSED").arg(element);
          continue;
        }

        if (seen.contains(element)) {
          continue; // already seen, skip duplicatge
        }

        seen.insert(element);
        removeUnusedList.append(element);
      }
      // Output if there's any elements remaining
      if (!removeUnusedList.isEmpty()) {
        s += QStringLiteral("%1 REMOVEUNUSED %2\n").arg(SpiceModel, removeUnusedList.join(","));
      }
    }

    // ADDRESISTORS - ONETERMINAL
    if (!resOneTerm.isEmpty()) {
      double res_val, scale;
      QString unit;
      misc::str2num(resOneTerm, res_val, unit, scale);
      if (res_val > 0) {
        s += QStringLiteral("%1 ADDRESISTORS ONETERMINAL %2\n").arg(SpiceModel, resOneTerm);
      }
    }

    // ADDRESISTORS - NODCPATH
    if (!resDC.isEmpty()) {
      double res_val, scale;
      QString unit;
      misc::str2num(resDC, res_val, unit, scale);
      if (res_val > 0) {
        s += QStringLiteral("%1 ADDRESISTORS NODCPATH %2\n").arg(SpiceModel, resDC);
      }
    }
    return s;
}
